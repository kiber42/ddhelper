#pragma once

#include <algorithm>
#include <array>
#include <stdexcept>
#include <utility>

template <typename Key, typename Value, std::size_t Size>
struct Map
{
  std::array<std::pair<Key, Value>, Size> data;

  [[nodiscard]] constexpr Value at(const Key& key) const
  {
    const auto iter = std::find_if(begin(data), end(data), [&key](const auto& v) { return v.first == key; });
    if (iter != end(data))
      return iter->second;
    else
      throw std::range_error("Not found");
  }
};
