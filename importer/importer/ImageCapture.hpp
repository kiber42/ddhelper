#pragma once

#include "importer/GameWindow.hpp"

#include <X11/Xlib.h>

#include <memory>

class ImageCapture
{
public:
  ImageCapture(GameWindow& gameWindow);
  ~ImageCapture();

  const XImage* acquire();
  [[nodiscard]] const XImage* current() const;

private:
  class Impl;
  std::unique_ptr<Impl> impl;
};
