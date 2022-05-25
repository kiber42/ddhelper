#pragma once

#include <memory>

#if !defined(_WIN32)
struct _XImage;
typedef struct _XImage XImage;
#else
struct XImage
{
  // TODO: Implement image capture on Windows
};
#endif

namespace importer
{
  class GameWindow;

  class ImageCapture
  {
  public:
    ImageCapture(GameWindow& gameWindow);
    ~ImageCapture();

    GameWindow& getGameWindow() const;

    const XImage* acquire();
    [[nodiscard]] const XImage* current() const;

  private:
    class Impl;
    std::unique_ptr<Impl> impl;
  };
} // namespace importer
