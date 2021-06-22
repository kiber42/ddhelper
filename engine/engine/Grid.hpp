#pragma once

#include "engine/PositionedVector.hpp"

#include <functional>
#include <vector>

template <class DataType>
class Grid
{
public:
  Grid(unsigned sizeX, unsigned sizeY);
  Grid(unsigned sizeX, unsigned sizeY, const DataType& init);
  const unsigned sizeX;
  const unsigned sizeY;

  bool isValid(Position position) const;

  using DataRef = typename std::vector<DataType>::reference;
  using ConstDataRef = typename std::vector<DataType>::const_reference;
  DataRef operator[](Position position);
  ConstDataRef operator[](Position position) const;

  using MatcherFn = std::function<bool(Position)>;
  //! Change value of any matching position to newValue
  void setMatching(MatcherFn matcher, DataType newValue);
  //! Iterate over Grid, stop early and return true when abort condition is fulfilled
  bool iterateOver(MatcherFn abortCondition);
  //! Iterate over 8 adjacent cells, stop early and return true when abort condition is fulfilled
  bool iterateAround(Position center, MatcherFn abortCondition);

private:
  std::vector<DataType> data;
};

template <class DataType>
Grid<DataType>::Grid(unsigned sizeX, unsigned sizeY)
  : sizeX(sizeX)
  , sizeY(sizeY)
  , data(sizeX * sizeY)
{
}

template <class DataType>
Grid<DataType>::Grid(unsigned sizeX, unsigned sizeY, const DataType& init)
  : sizeX(sizeX)
  , sizeY(sizeY)
  , data(sizeX * sizeY, init)
{
}

template <class DataType>
bool Grid<DataType>::isValid(Position position) const
{
  return position.getX() < sizeX && position.getY() < sizeY;
}

template <class DataType>
auto Grid<DataType>::operator[](Position position) -> DataRef
{
  return data[position.getY() * sizeX + position.getX()];
}

template <class DataType>
auto Grid<DataType>::operator[](Position position) const -> ConstDataRef
{
  return data[position.getY() * sizeX + position.getX()];
}

template <class DataType>
void Grid<DataType>::setMatching(MatcherFn matcher, DataType newValue)
{
  for (unsigned x = 0; x < sizeX; ++x)
  {
    for (unsigned y = 0; y < sizeY; ++y)
    {
      if (matcher(Position(x, y)))
        data[y * sizeX + x] = newValue;
    }
  }
}

template <class DataType>
bool Grid<DataType>::iterateOver(MatcherFn matcher)
{
  for (unsigned x = 0; x < sizeX; ++x)
  {
    for (unsigned y = 0; y < sizeY; ++y)
    {
      if (matcher(Position(x, y)))
        return true;
    }
  }
  return false;
}

template <class DataType>
bool Grid<DataType>::iterateAround(Position center, MatcherFn matcher)
{
  const unsigned cx = center.getX();
  const unsigned cy = center.getX();
  for (unsigned dx = cx > 0 ? -1u : 0u; dx != +2; ++dx)
  {
    const unsigned x = cx + dx;
    if (x >= sizeX)
      return false;
    for (unsigned dy = cy > 0 ? -1u : 0u; dy != +2; ++dy)
    {
      const unsigned y = cy + dy;
      if ((dx != 0 || dy != 0) && y < sizeY && matcher(Position(x, y)))
        return true;
    }
  }
  return false;
}
