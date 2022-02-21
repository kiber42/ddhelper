// Include OpenCV before any X11
#include <opencv2/opencv.hpp>

#include "X11/Xlib.h"

#include "importer/GameWindow.hpp"
#include "importer/ImageCapture.hpp"

bool captureTest()
{
  int numFrames = 0;
  importer::GameWindow gameWindow;
  importer::ImageCapture capture(gameWindow);
  while (auto ximage = capture.acquire())
  {
    auto img = cv::Mat(ximage->height, ximage->width, CV_8UC4, ximage->data);
    cv::imshow("img", img);
    cv::waitKey(100);
    ++numFrames;
  }

  return numFrames != 0;
}

int main()
{
  return captureTest() ? EXIT_SUCCESS : EXIT_FAILURE;
}
