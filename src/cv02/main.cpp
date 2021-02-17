//
// Created by richard on 05.02.21.
//
#include <opencv2/core/mat.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

enum class Channel : unsigned int {
  Blue,
  Green,
  Red
};

Channel getMaskChannel(const cv::Vec3b &mask) {
  if (mask[0])
    return Channel::Blue;
  if (mask[1])
    return Channel::Green;
  if (mask[2])
    return Channel::Red;
  
  throw std::runtime_error("Invalid input");
}


const cv::Vec3b blueMask(1, 0, 0);
const cv::Vec3b greenMask(0, 1, 0);
const cv::Vec3b redMask(0, 0, 1);

const cv::Mat_<cv::Vec3b> bayerFilter = (
    cv::Mat_<cv::Vec3b>(2, 2) << redMask, greenMask, greenMask, blueMask);

template<typename T_MAT_TYPE>
inline T_MAT_TYPE maskPix(const T_MAT_TYPE &pix, const T_MAT_TYPE &mask) {
  T_MAT_TYPE finalPix;
  for (int i = 0; i < pix.channels; i++) {
    finalPix[i] = pix[i] * mask[i];
  }
  return finalPix;
}


inline bool idxInBounds(const int idx, const int upper, const int lower = 0) {
  return (idx >= lower && idx < upper);
}

template<typename T_MAT_TYPE>
T_MAT_TYPE getPix(const cv::Mat_<T_MAT_TYPE> &src, const int row, const int col, const cv::Mat_<T_MAT_TYPE> &mask) {
//  if (row == 2 && col == 3) {
//    printf("Breakpoint hit\n");
//  }
  const int rows = src.rows;
  const int cols = src.cols;
  cv::Vec3f currPix = cv::Vec3f(0, 0, 0);
  
  constexpr std::array<std::pair<int, int>, 9> convolutionPositions = {
      std::make_pair(-1, -1), std::make_pair(-1, +0), std::make_pair(-1, +1),
      std::make_pair(+0, -1), std::make_pair(+0, +0), std::make_pair(+0, +1),
      std::make_pair(+1, -1), std::make_pair(+1, +0), std::make_pair(+1, +1),};
  
  std::vector<T_MAT_TYPE> convPixels;
  std::vector<T_MAT_TYPE> convMasks;
  
  for (const auto &pos : convolutionPositions) {
    if (idxInBounds(row + pos.first, rows) && idxInBounds(col + pos.second, cols)) {
      const auto convPixel = src((row + pos.first), (col + pos.second));
      const auto convMask = mask((row + pos.first) & 1, (col + pos.second) & 1);
      convPixels.emplace_back(convPixel);
      convMasks.emplace_back(convMask);
    }
  }
  
  std::array<int, 3> counts = {0, 0, 0};
  
  for (int i = 0; i < convPixels.size(); i++) {
    const auto iterMask = convMasks.at(i);
    const auto iterPix = convPixels.at(i);
    const int chnl = static_cast<int>(getMaskChannel(iterMask));
    counts[chnl]++;
    currPix[chnl] += iterPix[chnl];
  }
  
  for (int i = 0; i < 3; i++) {
    currPix[i] = currPix[i] / counts[i];
  }
  
  return currPix;
}

template<typename T_MAT_TYPE>
void applyFilter(const cv::Mat_<T_MAT_TYPE> &src, cv::Mat_<T_MAT_TYPE> &dst, const cv::Mat_<T_MAT_TYPE> &filter) {
  for (int row = 0; row < src.rows; row++) {
    for (int col = 0; col < src.cols; col++) {
      dst(row, col) = getPix(src, row, col, filter);
    }
  }
}

int main() {
  cv::Mat inputBayerImg = cv::imread("data/bayer.bmp", cv::IMREAD_GRAYSCALE);
  cv::Mat_<cv::Vec3b> bayerProcessImg;
  
  cv::cvtColor(inputBayerImg, bayerProcessImg, cv::COLOR_GRAY2BGR);
  cv::Mat_<cv::Vec3b> bayerOutputImg(bayerProcessImg.rows, bayerProcessImg.cols, cv::Vec3b(0, 0, 0));
  
  applyFilter(bayerProcessImg, bayerOutputImg, bayerFilter);
  
  cv::imshow("bayerInput", inputBayerImg);
  cv::imshow("bayer", bayerOutputImg);
  cv::waitKey();
}
