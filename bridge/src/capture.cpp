#include "bridge/capture.hpp"

#include <sys/ipc.h>

namespace
{
  std::optional<Window> findWindowByName(Display* display, Window top, const std::string& name)
  {
    char* window_name;
    if (XFetchName(display, top, &window_name) && name == window_name)
      return top;

    Window dummy;
    Window* children;
    unsigned int numChildren;
    if (!XQueryTree(display, top, &dummy, &dummy, &children, &numChildren))
      return {};

    auto cleaner = [](Window* children) { XFree(children); };
    std::unique_ptr<Window, decltype(cleaner)> cleanup(children);
    for (unsigned i = 0; i < numChildren; ++i)
    {
      if (auto window = findWindowByName(display, children[i], name))
        return window;
    }
    return {};
  }

  std::optional<Window> findWindowByName(Display* display, const std::string& name)
  {
    for (int i = 0; i < ScreenCount(display); ++i)
    {
      if (auto window = findWindowByName(display, RootWindow(display, i), name))
        return window;
    }
    return {};
  }
} // namespace

ImageCapture::~ImageCapture()
{
  clear();
}

bool ImageCapture::init()
{
  if (isInitialized())
  {
    XWindowAttributes updated;
    if (!XGetWindowAttributes(display.get(), *window, &updated) || attributes.screen != updated.screen ||
        attributes.width != updated.width || attributes.height != updated.height)
    {
      clear();
      return init();
    }
    return true;
  }

  display.reset(XOpenDisplay(NULL));
  window = findWindowByName(display.get(), "Desktop Dungeons");
  if (!window)
    return false;

  XGetWindowAttributes(display.get(), *window, &attributes);
  shminfo.reset(new XShmSegmentInfo);
  ximage.reset(XShmCreateImage(display.get(), DefaultVisualOfScreen(attributes.screen),
                               DefaultDepthOfScreen(attributes.screen), ZPixmap, nullptr, shminfo.get(),
                               attributes.width, attributes.height));
  if (!ximage)
    return false;

  shminfo->shmid = shmget(IPC_PRIVATE, ximage->bytes_per_line * ximage->height, IPC_CREAT | 0777);
  if (shminfo->shmid < 0)
  {
    shminfo.reset();
    return false;
  }
  shminfo->shmaddr = ximage->data = static_cast<char*>(shmat(shminfo->shmid, 0, 0));
  shminfo->readOnly = 0;

  if (!XShmAttach(display.get(), shminfo.get()))
  {
    shminfo.reset();
    return false;
  }

  return true;
}

void ImageCapture::clear()
{
  if (display && shminfo)
    XShmDetach(display.get(), shminfo.get());
  ximage.reset();
  shminfo.reset();
  display.reset();
  window = 0;
}

bool ImageCapture::isInitialized() const
{
  // shminfo is initialized last and can therefore to check whether initialization is complete
  return shminfo != nullptr;
}

const XImage* ImageCapture::acquire()
{
  if (init() && XShmGetImage(display.get(), *window, ximage.get(), 0, 0, 0x00ffffff))
    return ximage.get();
  return nullptr;
}

const XImage* ImageCapture::current() const
{
  return ximage.get();
}

bool ImageCapture::isAttached() const
{
  thread_local XWindowAttributes updated;
  return isInitialized() && XGetWindowAttributes(display.get(), *window, &updated);
}
