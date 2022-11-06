#include "importer/ImageCapture.hpp"

#include "importer/GameWindow.hpp"

#include <opencv2/opencv.hpp>

#if !defined(_WIN32)
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XShm.h>
#include <sys/ipc.h>
#include <sys/shm.h>

namespace
{
  struct XImageCaptureHelper
  {
    XImageCaptureHelper(const importer::GameWindow& gameWindow)
    {
      if (!gameWindow.valid())
        return;

      auto display = gameWindow.getDisplay();
      XGetWindowAttributes(display, gameWindow.getHandle(), &attributes);
      ximage =
          XShmCreateImage(display, DefaultVisualOfScreen(attributes.screen), DefaultDepthOfScreen(attributes.screen),
                          ZPixmap, nullptr, &shminfo, attributes.width, attributes.height);
      if (!ximage)
        return;

      shminfo.shmid = shmget(IPC_PRIVATE, ximage->bytes_per_line * ximage->height, IPC_CREAT | 0777);
      if (shminfo.shmid < 0)
        return;
      shminfo.shmaddr = ximage->data = static_cast<char*>(shmat(shminfo.shmid, 0, 0));
      shminfo.readOnly = 0;

      if (!XShmAttach(display, &shminfo))
        return;
      attachedDisplay = display;

      valid = XShmGetImage(display, gameWindow.getHandle(), ximage, 0, 0, 0x00ffffff) != 0;
    }

    ~XImageCaptureHelper()
    {
      if (ximage)
        XDestroyImage(ximage);
      if (attachedDisplay)
        XShmDetach(attachedDisplay, &shminfo);
      if (shminfo.shmaddr)
        shmdt(shminfo.shmaddr);
    }

    XImage* acquire()
    {
      if (!valid)
        return nullptr;
      return ximage;
    }

  private:
    bool valid{};
    Display* attachedDisplay{};
    XImage* ximage{};
    XShmSegmentInfo shminfo{};
    XWindowAttributes attributes{};
  };
} // namespace

cv::Mat importer::ImageCapture::asMatrix()
{
  XImageCaptureHelper helper(gameWindow);
  auto ximage = helper.acquire();
  lastCaptureSuccessful = ximage != nullptr;
  // Need to clone the image data, since the shared memory handle will be released.  This copy could be avoided by
  // keeping the XImageCaptureHelper alive for longer, but this doesn't seem worth the effort.  The copy also reduces
  // the risk of use after free if the game window is closed or resized.
  if (lastCaptureSuccessful)
    return cv::Mat(ximage->height, ximage->width, CV_8UC4, ximage->data).clone();
  else
    return cv::Mat();
}

#else

namespace
{
  template <class Value>
  struct WithCleanup
  {
    WithCleanup(Value&& value, std::function<void(Value)>&& cleanup)
      : value(std::move(value))
      , cleanup(std::move(cleanup))
    {
    }

    ~WithCleanup()
    {
      if (value)
        cleanup(value);
    }

    const Value& operator*() const { return value; }

    WithCleanup(const WithCleanup&) = delete;
    WithCleanup& operator=(const WithCleanup&) = delete;

  private:
    Value value;
    std::function<void(Value)> cleanup;
  };

  template <class Value, typename Any>
  WithCleanup(Value, Any) -> WithCleanup<Value>;
} // namespace

cv::Mat importer::ImageCapture::asMatrix()
{
  lastCaptureSuccessful = false;

  auto handle = gameWindow.getHandle();

  RECT windowRect;
  GetClientRect(handle, &windowRect);
  const auto width = windowRect.right;
  const auto height = windowRect.bottom;

  auto info = BITMAPINFO{.bmiHeader = BITMAPINFOHEADER{.biSize = sizeof(BITMAPINFOHEADER),
                                                       .biWidth = width,
                                                       .biHeight = -height,
                                                       .biPlanes = 1,
                                                       .biBitCount = 32,
                                                       .biCompression = BI_RGB, // uncompressed RGB
                                                       .biSizeImage = 0,        // not used for uncompressed RGB
                                                       .biXPelsPerMeter = 1,
                                                       .biYPelsPerMeter = 1,
                                                       .biClrUsed = 0,
                                                       .biClrImportant = 0}};

  auto deviceContext = WithCleanup(GetDC(handle), [handle](auto dc) { ReleaseDC(handle, dc); });
  auto memoryDeviceContext = WithCleanup(CreateCompatibleDC(*deviceContext), DeleteDC);
  auto bitmap = WithCleanup(CreateCompatibleBitmap(*deviceContext, width, height), DeleteObject);

  // Blit to memory
  SelectObject(*memoryDeviceContext, *bitmap);
  BitBlt(*memoryDeviceContext, 0, 0, width, height, *deviceContext, 0, 0, SRCCOPY);

  // Create and fill matrix
  cv::Mat mat = cv::Mat(height, width, CV_8UC4);
  GetDIBits(*memoryDeviceContext, *bitmap, 0, height, mat.data, &info, DIB_RGB_COLORS);

  lastCaptureSuccessful = true;

  return mat;
}

#endif
