#pragma once

#include <limits>
#include <type_traits>

template <typename Short, typename Long>
constexpr Short clamped(Long unclamped, Short min, Short max)
{
  return unclamped < min ? min : (unclamped > max ? max : unclamped);
}

template <typename Short, typename Long>
constexpr Short clampedTo(Long unclamped)
{
  if constexpr (std::is_unsigned<Short>::value)
  {
    if constexpr (std::is_unsigned<Long>::value)
    {
      return unclamped > std::numeric_limits<Short>::max() ? std::numeric_limits<Short>::max()
                                                           : static_cast<Short>(unclamped);
    }
    else
    {
      return unclamped < 0 ? static_cast<Short>(0)
                           : (unclamped > std::numeric_limits<Short>::max() ? std::numeric_limits<Short>::max()
                                                                            : static_cast<Short>(unclamped));
    }
  }
  else
  {
    return unclamped > std::numeric_limits<Short>::max()
               ? std::numeric_limits<Short>::max()
               : (unclamped < std::numeric_limits<Short>::min() ? std::numeric_limits<Short>::min()
                                                                : static_cast<Short>(unclamped));
  }
}
