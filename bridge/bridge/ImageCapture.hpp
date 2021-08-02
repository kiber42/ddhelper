#pragma once

#include "bridge/GameWindow.hpp"

#include <memory>

class ImageCapture
{
public:
  ImageCapture();
  ~ImageCapture();

  const XImage* acquire();
  const XImage* current() const;

private:
  class Impl;
  std::unique_ptr<Impl> impl;
};
