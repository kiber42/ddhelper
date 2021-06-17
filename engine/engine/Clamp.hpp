#pragma once

#include <limits>
#include <type_traits>

template <typename T>
constexpr T clamped(T unclamped, T min, T max)
{
  return unclamped < min ? min : (unclamped > max ? max : unclamped);
}

template <typename T, typename T2>
constexpr T clampedTo(T2 unclamped)
{
  if constexpr (std::is_unsigned<T>::value)
  {
    if constexpr (std::is_unsigned<T2>::value)
    {
      return unclamped > std::numeric_limits<T>::max() ? std::numeric_limits<T>::max() : static_cast<T>(unclamped);
    }
    else
    {
      return unclamped < 0 ? static_cast<T>(0)
                           : (unclamped > std::numeric_limits<T>::max() ? std::numeric_limits<T>::max()
                                                                        : static_cast<T>(unclamped));
    }
  }
  else
  {
    return unclamped > std::numeric_limits<T>::max()
               ? std::numeric_limits<T>::max()
               : (unclamped < std::numeric_limits<T>::min() ? std::numeric_limits<T>::min()
                                                            : static_cast<T>(unclamped));
  }
}
