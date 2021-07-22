#include <opencv2/opencv.hpp>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XShm.h>

#include <sys/ipc.h>
#include <sys/shm.h>

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

int captureTest()
{
  auto display = XOpenDisplay(NULL);
  auto window = findWindowByName(display, "Desktop Dungeons");
  if (!window)
    return -1;

  XWindowAttributes window_attributes;
  XGetWindowAttributes(display, *window, &window_attributes);

  Screen* screen = window_attributes.screen;
  const uint width = window_attributes.width;
  const uint height = window_attributes.height;

  XShmSegmentInfo shminfo;
  XImage* ximg = XShmCreateImage(display, DefaultVisualOfScreen(screen), DefaultDepthOfScreen(screen), ZPixmap, NULL,
                                 &shminfo, width, height);

  shminfo.shmid = shmget(IPC_PRIVATE, ximg->bytes_per_line * ximg->height, IPC_CREAT | 0777);
  shminfo.shmaddr = ximg->data = (char*)shmat(shminfo.shmid, 0, 0);
  shminfo.readOnly = 0;
  if (shminfo.shmid < 0)
  {
    puts("Fatal shminfo error!");
    return EXIT_FAILURE;
  }

  if (!XShmAttach(display, &shminfo))
  {
    puts("Could not attach to window.");
    return EXIT_FAILURE;
  }

  while (XShmGetImage(display, *window, ximg, 0, 0, 0x00ffffff))
  {
    auto img = cv::Mat(height, width, CV_8UC4, ximg->data);
    cv::imshow("img", img);
    cv::waitKey(100);
  }

  XShmDetach(display, &shminfo);
  XDestroyImage(ximg);
  shmdt(shminfo.shmaddr);
  XCloseDisplay(display);

  return EXIT_SUCCESS;
}
