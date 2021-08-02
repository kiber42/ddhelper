#include "bridge/ImageCapture.hpp"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XShm.h>
#include <sys/ipc.h>
#include <sys/shm.h>

class ImageCapture::Impl
{
public:
  Impl(GameWindow&);
  bool init();
  void clear();
  bool acquire();
  bool isInitialized() const;
  const XImage* getImage() const;

private:
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

  GameWindow& gameWindow;
  Display* attachedDisplay;
  std::unique_ptr<XImage, ximage_cleanup> ximage;
  std::unique_ptr<XShmSegmentInfo, shm_cleanup> shminfo;
  XWindowAttributes attributes;
};

ImageCapture::Impl::Impl(GameWindow& gameWindow) : gameWindow(gameWindow) {}


bool ImageCapture::Impl::init()
{
  if (!gameWindow.valid())
    return false;

  if (isInitialized())
  {
    XWindowAttributes updated;
    if (!XGetWindowAttributes(gameWindow.getDisplay(), gameWindow.getWindow(), &updated) ||
        attributes.screen != updated.screen || attributes.width != updated.width || attributes.height != updated.height)
    {
      clear();
      return init();
    }
    return true;
  }

  XGetWindowAttributes(gameWindow.getDisplay(), gameWindow.getWindow(), &attributes);
  shminfo.reset(new XShmSegmentInfo);
  ximage.reset(XShmCreateImage(gameWindow.getDisplay(), DefaultVisualOfScreen(attributes.screen),
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

  attachedDisplay = gameWindow.getDisplay();
  if (!XShmAttach(attachedDisplay, shminfo.get()))
  {
    shminfo.reset();
    return false;
  }

  return true;
}

void ImageCapture::Impl::clear()
{
  if (attachedDisplay && shminfo)
    XShmDetach(attachedDisplay, shminfo.get());
  attachedDisplay = nullptr;
  shminfo.reset();
  ximage.reset();
}

bool ImageCapture::Impl::acquire()
{
  return init() && XShmGetImage(gameWindow.getDisplay(), gameWindow.getWindow(), ximage.get(), 0, 0, 0x00ffffff);
}

const XImage* ImageCapture::Impl::getImage() const
{
  return ximage.get();
}

bool ImageCapture::Impl::isInitialized() const
{
  // shminfo is initialized last and can therefore to check whether initialization is complete
  return shminfo != nullptr;
}

ImageCapture::ImageCapture(GameWindow& gameWindow)
  : impl(new Impl(gameWindow))
{
}

ImageCapture::~ImageCapture()
{
  // needs to be defined here, where Impl is a complete type
}

const XImage* ImageCapture::acquire()
{
  if (impl->acquire())
    return impl->getImage();
  return nullptr;
}

const XImage* ImageCapture::current() const
{
  return impl->getImage();
}
