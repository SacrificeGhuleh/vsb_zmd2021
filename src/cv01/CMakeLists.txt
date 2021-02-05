cmake_minimum_required(VERSION 3.14)
project(RGB_YUV_Converter)

# add libs you need
set(OpenCV_LIBS opencv_core opencv_imgproc opencv_highgui opencv_imgcodecs opencv_calib3d opencv_objdetect)

add_executable(${PROJECT_NAME} ${SRC} main.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE ${OpenCV_LIBS})