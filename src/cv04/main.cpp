//
// Created by richard on 24.03.21.
//
#include <string>
#include <map>
#include <stdexcept>
#include <iostream>

#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

constexpr int defaultOffset = 0;

enum class Align
{
  Horizontal,
  Vertical,
  None
};

class CLIParser
{
public:
  CLIParser(int argc, const char **argv, const std::vector<std::string> &positionalArguments)
  {
    appName_ = argv[0];
    std::string key, val;
    int parsedArguments = 0;
    for (int i = 1; i < argc; i++)
    {
      const std::string cur = argv[i];
      int pos = cur.find('=');
      if (pos != std::string::npos)
      {
        key = cur.substr(1, pos - 1);
        val = cur.substr(pos + 1);
      }
      else
      {
        if (cur[0] == '-')
        {
          if (i + 1 >= argc)
            throw std::runtime_error("Invalid CLI arguments");
          key = cur.substr(1);
          val = argv[i + 1];
          if (val[0] == '-')
          {
            val = "";
          }
          else
          {
            i++;
          }
        }
        else
        {
          key = positionalArguments.at(parsedArguments);
          val = cur;
        }

        pos = val.find('=');
        if (pos != std::string::npos)
        {
          throw std::runtime_error("Invalid CLI arguments");
        }
      }

      cliProperties_[key] = val;
      parsedArguments++;
    }
    std::cout << "Parsed props for app: " << appName_ << "\n";
    for (const auto &prop : cliProperties_)
    {
      std::cout << "  " << prop.first << " - " << prop.second << "\n";
    }
  }

  bool hasProperty(const std::string &property)
  {
    return cliProperties_.find(property) != cliProperties_.end();
  }

  template <class T>
  T getProperty(const std::string &property)
  {
    static_assert(std::is_arithmetic<T>());

    if (!hasProperty(property))
      throw std::runtime_error("Property doesnt exist");

    if (std::is_arithmetic<T>())
    {
      if (std::is_same<float, T>())
      {
        return std::stof(cliProperties_[property]);
      }
      if (std::is_same<double, T>())
      {
        return std::stod(cliProperties_[property]);
      }
      if (std::is_integral<T>())
      {
        return std::stoi(cliProperties_[property]);
      }
    }
    throw std::runtime_error("Invalid");
  }

  std::string appName_;
  std::map<std::string, std::string> cliProperties_;
};

template <>
std::string CLIParser::getProperty(const std::string &property)
{
  return cliProperties_[property];
}

void printUsage()
{
  std::cout << "Exports screenshots in matrix" << '\n';
  std::cout << "Example: " << '\n';
  std::cout << "  ./mosaic -rows=5 -cols=4 -width=1280 -height=1024 -in=input.avi -out=output-1.jpg " << '\n';
  std::cout << "  ./mosaic 5 4 1280 1024 input.avi output-1.jpg " << '\n';

  std::cout << "Params: " << '\n';
  std::cout << "  -rows     (mandatory) Number of rows in output matrix" << '\n';
  std::cout << "  -cols     (mandatory) Number of cols in output matrix" << '\n';
  std::cout << "  -width    (mandatory) Width in pixels of output image" << '\n';
  std::cout << "  -height   (mandatory) Height in pixels of output image" << '\n';
  std::cout << "  -in       (mandatory) Input video filename" << '\n';
  std::cout << "  -out      (mandatory) Output filename" << '\n';
  std::cout << "  -offset               Sets offset for first and last frame" << '\n';
  std::cout << "                        This handles problem with black frames" << '\n';
  std::cout << "                        Defaultly set to " << defaultOffset << '\n';
  std::cout << "  -help                 Prints this help" << '\n';
}

void extractFrame(double relPos, cv::Mat &frame, cv::VideoCapture &cap, int offset)
{
  // Handle bounds
  relPos = std::max(relPos, 0.);
  relPos = std::min(relPos, 1.);

  //Clear frame
  frame = cv::Mat();

  const double frameCount = cap.get(cv::CAP_PROP_FRAME_COUNT);
  const double maxIndex = frameCount - 1;

  double framePos = (maxIndex - 1) * relPos;

  // Handle user offset
  framePos = std::max(framePos, 0. + offset);
  framePos = std::min(framePos, (maxIndex - 1.) - offset);

  while (frame.empty())
  {
    cap.set(cv::CAP_PROP_POS_FRAMES, framePos);
    cap >> frame;

    if (relPos == 0)
    {
      // If there is empty frame at the begining, find first valid frame
      framePos += 1;
    }
    if (relPos == 1)
    {
      // If there is empty frame at the end, find last valid frame
      framePos -= 1;
    }
  }
}

inline bool willFit(const cv::Size &srcSize, const cv::Size &dstSize)
{
  return srcSize.width <= dstSize.width && srcSize.height <= dstSize.height;
}

int main(int argc, const char **argv)
{
  //-rows=5 -cols=4 -width=1280 -height=1024 -in="/run/media/richard/My Passport/Videos/Filmy/Pulp Fiction.mkv" -out=output-1.jpg
  //5 4 1280 1024 "/run/media/richard/My Passport/Videos/Filmy/Pulp Fiction.mkv" output-1.jpg

  std::vector<std::string> positionalArguments = {
      "rows",
      "cols",
      "width",
      "height",
      "in",
      "out"};

  CLIParser parser(argc, argv, positionalArguments);

  bool printHelp = parser.hasProperty("help");
  printHelp |= !parser.hasProperty("rows");
  printHelp |= !parser.hasProperty("cols");
  printHelp |= !parser.hasProperty("width");
  printHelp |= !parser.hasProperty("height");
  printHelp |= !parser.hasProperty("in");
  printHelp |= !parser.hasProperty("out");

  if (printHelp)
  {
    printUsage();
    return 0;
  }
  //  return 0;

  size_t rows = parser.getProperty<size_t>("rows");
  size_t cols = parser.getProperty<size_t>("cols");
  size_t width = parser.getProperty<size_t>("width");
  size_t height = parser.getProperty<size_t>("height");

  const int frameOffset = parser.hasProperty("offset") ? parser.getProperty<int>("offset") : defaultOffset;

  std::string fileInputName = parser.getProperty<std::string>("in");
  std::string fileOutputName = parser.getProperty<std::string>("out");

  cv::VideoCapture cap(fileInputName);

  if (!cap.isOpened())
  {
    throw std::runtime_error("Error opening video stream or file");
  }

  const double numberOfFrames = cap.get(cv::CAP_PROP_FRAME_COUNT);
  std::cout << "number of frames in video: " << numberOfFrames << '\n';

  cv::Mat frame;

  std::vector<cv::Mat> frames;
  cv::Size frameSize(width / cols, height / rows);

  Align alignOutput;
  size_t alignOffset;
  cv::Size scaledFrameSize;
  {
    cv::Size originalFrameSize(cap.get(cv::CAP_PROP_FRAME_WIDTH), cap.get(cv::CAP_PROP_FRAME_HEIGHT));
    double widthScale = static_cast<double>(frameSize.width) / static_cast<double>(originalFrameSize.width);
    double heightScale = static_cast<double>(frameSize.height) / static_cast<double>(originalFrameSize.height);

    cv::Size scaledByWidth = cv::Size(originalFrameSize.width * widthScale, originalFrameSize.height * widthScale);
    cv::Size scaledByHeight = cv::Size(originalFrameSize.width * heightScale, originalFrameSize.height * heightScale);
    if (willFit(scaledByWidth, frameSize))
    {
      scaledFrameSize = scaledByWidth;
      alignOutput = Align::Vertical;
      alignOffset = (frameSize.height - scaledFrameSize.height) / 2;
    }
    else if (willFit(scaledByHeight, frameSize))
    {
      scaledFrameSize = scaledByHeight;
      alignOutput = Align::Horizontal;
      alignOffset = (frameSize.width - scaledFrameSize.width) / 2;
    }
    else
    {
      scaledFrameSize = frameSize;
      alignOutput = Align::None;
      alignOffset = 0;
    }
  }

  if (rows * cols > 1)
  {
    for (int i = 0; i < rows * cols; i++)
    {
      double offset = i / ((rows * cols) - 1.);

      extractFrame(offset, frame, cap, frameOffset);
      if (frame.empty())
      {
        frame = cv::Mat::zeros(scaledFrameSize, CV_8UC3);
      }
      cv::resize(frame, frame, scaledFrameSize);
      frames.emplace_back(frame);
    }
  }
  else
  {
    extractFrame(0, frame, cap, frameOffset);
    cv::resize(frame, frame, scaledFrameSize);
    frames.emplace_back(frame);
  }

  cv::Mat outputMat = cv::Mat::zeros(height, width, CV_8UC3);
  for (int i = 0; i < frames.size(); i++)
  {
    size_t x = i % cols;
    size_t y = i / cols;

    //    std::cout << "Index: " << i << " x: " << x << " y: " << y << "\n";

    size_t xOffset = x * frameSize.width;
    size_t yOffset = y * frameSize.height;

    switch (alignOutput)
    {
    case Align::Horizontal:
    {
      xOffset += alignOffset;
      break;
    }
    case Align::Vertical:
    {
      yOffset += alignOffset;
      break;
    }
    case Align::None:
    {
      break;
    }
    }

    for (int row = 0; row < frames[0].rows; row++)
    {
      for (int col = 0; col < frames[0].cols; col++)
      {
        outputMat.at<cv::Vec3b>(yOffset + row, xOffset + col) = frames.at(i).at<cv::Vec3b>(row, col);
      }
    }
  }

  cv::imwrite(fileOutputName, outputMat);
  //  cv::imshow(fileOutputName, outputMat);
  //  cv::waitKey();

  cap.release();
  cv::destroyAllWindows();
}
