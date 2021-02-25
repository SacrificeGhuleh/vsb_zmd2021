//
// Created by richard on 05.02.21.
//
#include <opencv2/core/mat.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include <cmath>

template<typename T>
inline T sqr(const T &val) {
  return val * val;
}

bool cmpFloats(float a, float b, float epsilon = 0.001f) {
  return std::fabs(a - b) < epsilon;
}

template<size_t LEN>
void hdr(const std::array<cv::Mat_<cv::Vec3b>, LEN> &inputImages,
         cv::Mat_<uint8_t> &grayscaleHDR,
         cv::Mat_<cv::Vec3b> &colorHDR,
         const float sigma,
         const float mi) {
  const int imgRows = inputImages.at(0).rows;
  const int imgCols = inputImages.at(0).cols;
  
  std::array<cv::Mat_<float>, LEN> weighs;
  std::array<cv::Mat_<uint8_t>, LEN> grayImages;
  
  for (int i = 0; i < LEN; i++) {
    auto weighMat = cv::Mat_<float>(imgRows, imgCols, 0.0);
    cv::Mat_<uint8_t> grayImage;
    cv::cvtColor(inputImages.at(i), grayImage, cv::COLOR_BGR2GRAY);
    
    for (int row = 0; row < imgRows; row++) {
      for (int col = 0; col < imgCols; col++) {
        auto inputImagePix = grayImage(row, col);
        const float top = sqr(static_cast<float>(inputImagePix) - (255.f * mi));
        const float bot = 2.f * sqr(255.f * sigma);
//        const float weight = exp(sqr(static_cast<float>(inputImagePix) - (255.f * mi)) / (2.f * sqr(255.f * sigma)));
        const float weight = exp(top / bot);
        weighMat(row, col) = weight;
      }
    }
    weighs.at(i) = weighMat;
    grayImages.at(i) = grayImage;
  }
  
  // Normalize weight
  
  for (int row = 0; row < imgRows; row++) {
    for (int col = 0; col < imgCols; col++) {
      float weighSum = 0;
      
      for (int i = 0; i < LEN; i++) {
        weighSum += weighs.at(i)(row, col);
      }
      
      for (int i = 0; i < LEN; i++) {
        weighs.at(i)(row, col) /= weighSum;
      }

//      #ifndef NDEBUG
//      weighSum = 0;
//      for (int i = 0; i < LEN; i++) {
//        weighSum += weighs.at(i)(row, col);
//      }
//      assert(cmpFloats(weighSum, 1.f));
//      #endif
    }
  }
  
  
  grayscaleHDR = cv::Mat_<uint8_t>(imgRows, imgCols);
  colorHDR = cv::Mat_<cv::Vec3b>(imgRows, imgCols);
  
  for (int row = 0; row < imgRows; row++) {
    for (int col = 0; col < imgCols; col++) {
      float grayscalePix = 0;
      cv::Vec3f colorPix(0, 0, 0);
      
      for (int i = 0; i < LEN; i++) {
        grayscalePix += static_cast<float>(grayImages.at(i)(row, col)) * weighs.at(i)(row, col);
        colorPix += inputImages.at(i)(row, col) * weighs.at(i)(row, col);
      }
      
      grayscaleHDR(row, col) = grayscalePix;
      colorHDR(row, col) = colorPix;
    }
  }
}

constexpr size_t inputSequenceLen = 5U;

static cv::Mat_<uint8_t> grayscaleHDR;
static cv::Mat_<cv::Vec3b> colorHDR;
static cv::Mat_<cv::Vec3b> outMat;


static char *winName = "HDR setting";

static const std::array<cv::Mat_<cv::Vec3b>, inputSequenceLen>
    inputImages = {
    cv::imread("data/s1_0.png"),
    cv::imread("data/s1_1.png"),
    cv::imread("data/s1_2.png"),
    cv::imread("data/s1_3.png"),
    cv::imread("data/s1_4.png")
};

const int imgRows = inputImages.at(0).rows;
const int imgCols = inputImages.at(0).cols;

constexpr int sliderMin = 0;
constexpr int sliderMax = 255;

static int mi_ = 0.2 * sliderMax;
static int sigma_ = 0.5f * sliderMax;

static void onTrackbar(int, void *) {
  const float miFl = static_cast<float>(mi_) / static_cast<float>(sliderMax);
  const float sigmaFl = static_cast<float>(sigma_) / static_cast<float>(sliderMax);
  
  printf("Sigma: %f Mi: %f\n", sigmaFl, miFl);
  
  hdr(inputImages, grayscaleHDR, colorHDR, sigmaFl, miFl);
  
  
  for (int row = 0; row < imgRows; row++) {
    for (int col = 0; col < imgCols; col++) {
      const auto grayPix = grayscaleHDR(row, col);
      const auto colorPix = colorHDR(row, col);
      outMat(row, col) = colorPix;
      outMat(row, imgCols + col) = cv::Vec3b(grayPix, grayPix, grayPix);
    }
  }
  
  cv::imshow(winName, outMat);
}

int main() {
  grayscaleHDR = cv::Mat_<uint8_t>(imgRows, imgCols);
  colorHDR = cv::Mat_<cv::Vec3b>(imgRows, imgCols);
  
  outMat = cv::Mat_<cv::Vec3b>(imgRows, imgCols * 2);
  
  cv::namedWindow(winName, cv::WINDOW_KEEPRATIO);
  
  cv::createTrackbar("Sigma", winName, &sigma_, sliderMax, onTrackbar);
  cv::createTrackbar("Mi", winName, &mi_, sliderMax, onTrackbar);
  cv::imshow(winName, outMat);
  cv::resizeWindow(winName, imgCols * 2, imgRows);
  
  onTrackbar(0, nullptr);
  cv::waitKey();
}
