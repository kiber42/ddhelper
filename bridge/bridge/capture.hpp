#pragma once

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XShm.h>
#include <sys/shm.h>

#include <memory>
#include <optional>

class ImageCapture
{
public:
  ImageCapture() = default;
  ImageCapture(const ImageCapture&) = delete;
  ImageCapture& operator=(const ImageCapture&) const = delete;
  ~ImageCapture();

  const XImage* acquire();
  const XImage* current() const;
  bool isAttached() const;

private:
  bool init();
  void clear();
  bool isInitialized() const;

  struct display_cleanup
  {
    void operator()(Display* display) { XCloseDisplay(display); }
  };
  struct ximage_cleanup
  {
    void operator()(XImage* ximage) { XDestroyImage(ximage); }
  };
  struct shm_cleanup
  {
    void operator()(XShmSegmentInfo* shminfo)
    {
      if (shminfo->shmaddr)
        shmdt(shminfo->shmaddr);
    }
  };

  std::unique_ptr<Display, display_cleanup> display;
  std::unique_ptr<XImage, ximage_cleanup> ximage;
  std::unique_ptr<XShmSegmentInfo, shm_cleanup> shminfo;
  std::optional<Window> window;
  XWindowAttributes attributes;
};
