#pragma once

#include <memory>

struct _XImage;
typedef struct _XImage XImage;

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
