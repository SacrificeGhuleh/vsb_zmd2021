//
// Created by richard on 05.02.21.
//
#include <opencv2/core/mat.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>


const cv::Mat toYuvKernel = (
    cv::Mat_<float>(3, 3) << 0.299, 0.587, 0.114, -0.14713, -0.28886, 0.436, 0.615, -0.51499, -0.10001);

const cv::Mat toRgbKernel = (
    cv::Mat_<float>(3, 3) << 1, 0, 1.13983, 1, -0.39465, -0.5806, 1, 2.03211, 0);

template<class T_DATA>
void convert(cv::Mat_<T_DATA> &yuvMat, const cv::Mat &kernel) {
  for (int row = 0; row < yuvMat.rows; row++) {
    for (int col = 0; col < yuvMat.cols; col++) {
      auto locPix = yuvMat(row, col);
      cv::Mat_<typename T_DATA::value_type> locPixMat = static_cast<cv::Mat>(locPix).t();
      cv::Mat_<typename T_DATA::value_type> resultData = locPixMat * kernel;
      locPix = static_cast<T_DATA>(resultData);
      yuvMat(row, col) = locPix;
    }
  }
}

int main() {
  cv::Mat inputImg = cv::imread("data/lenna.png");
  //Convert to RGB
  cv::cvtColor(inputImg, inputImg, cv::COLOR_BGR2RGB);
  cv::Mat_<cv::Vec3f> yuvImg;
  cv::Mat_<cv::Vec3f> floatInputImg;
  
  cv::Mat_<cv::Vec3f> outputImg(inputImg.rows * 2, inputImg.cols * 2);
  
  inputImg.convertTo(floatInputImg, CV_32FC3, 1.f / 255.f);
  yuvImg = floatInputImg.clone();
  
  convert(yuvImg, toYuvKernel);
  
  
  //Convert back to BGR
  cv::cvtColor(floatInputImg, floatInputImg, cv::COLOR_RGB2BGR);
  for (int row = 0; row < floatInputImg.rows; row++) {
    for (int col = 0; col < floatInputImg.cols; col++) {
      const auto &inputPix = floatInputImg(row, col);
      const auto &processedPix = yuvImg(row, col);
      
      outputImg(row, col) = inputPix;
      outputImg(floatInputImg.rows + row, col) = cv::Vec3f(processedPix[0], processedPix[0], processedPix[0]);
      outputImg(row, floatInputImg.cols + col) = cv::Vec3f(processedPix[1], processedPix[1], processedPix[1]);
      outputImg(floatInputImg.rows + row, floatInputImg.cols + col) = cv::Vec3f(processedPix[2], processedPix[2], processedPix[2]);
    }
  }
  
  cv::imshow("Output", outputImg);
  cv::imshow("Output YUV", yuvImg);
  
  cv::Mat_<cv::Vec3f> floatBackConvertedInputImg = yuvImg.clone();
  convert(floatBackConvertedInputImg, toRgbKernel);
  
  cv::cvtColor(floatBackConvertedInputImg, floatBackConvertedInputImg, cv::COLOR_RGB2BGR);
  
  cv::imshow("BackConvertedImg", floatBackConvertedInputImg);
  
  
  cv::waitKey();
  
  
}
