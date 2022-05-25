#pragma once

// TODO: Include OpenCV in Windows build
#if !defined(_WIN32)
#include "engine/StrongTypes.hpp"

#include <opencv2/opencv.hpp>

namespace importer
{
  using Column = NamedType<int, struct ColumnParameter>;
  using Row = NamedType<int, struct RowParameter>;

  struct PixelBGR
  {
    PixelBGR(const cv::Mat& image, Column col, Row row)
      : _v(image.at<cv::Vec3b>(row.get(), col.get()))
    {
    }
    constexpr uint8_t r() const { return _v.val[2]; }
    constexpr uint8_t g() const { return _v.val[1]; }
    constexpr uint8_t b() const { return _v.val[0]; }
    constexpr uint32_t rgb() const
    {
      return (static_cast<uint32_t>(_v.val[2]) << 16) + (static_cast<uint32_t>(_v.val[1]) << 8) +
             static_cast<uint32_t>(_v.val[0]);
    }
    constexpr uint32_t rg() const { return (static_cast<uint32_t>(_v.val[2]) << 8) + static_cast<uint32_t>(_v.val[1]); }

  private:
    cv::Vec3b _v;
  };

  struct PixelARGB
  {
    PixelARGB(const cv::Mat& image, Column col, Row row)
      : _v(image.at<uint32_t>(row.get(), col.get()) & 0xFFFFFF)
    {
    }
    constexpr uint8_t r() const { return (_v & 0xFF0000) >> 16; }
    constexpr uint8_t g() const { return (_v & 0xFF00) >> 8; }
    constexpr uint8_t b() const { return _v & 0xFF; }
    constexpr uint32_t rgb() const { return _v; }
    constexpr uint32_t rg() const { return _v >> 8; }

  private:
    uint32_t _v;
  };
} // namespace importer

#endif