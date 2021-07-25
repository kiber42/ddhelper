// Include OpenCV before any X11
#include <opencv2/opencv.hpp>

#include "bridge/capture.hpp"

bool captureTest()
{
  int numFrames = 0;
  ImageCapture capture;
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
