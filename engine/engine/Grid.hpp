#pragma once

#include "engine/PositionedVector.hpp"

#include <vector>

template <class DataType>
class Grid
{
public:
  Grid(unsigned sizeX, unsigned sizeY, const DataType& init = DataType());
  const unsigned sizeX;
  const unsigned sizeY;

  using DataRef = typename std::vector<DataType>::reference;

  bool isValid(Position position) const;
  DataRef operator[](Position position);
  DataType operator[](Position position) const;

private:
  std::vector<DataType> data;
};

template <class DataType>
Grid<DataType>::Grid(unsigned sizeX, unsigned sizeY, const DataType& init)
  : sizeX(sizeX)
  , sizeY(sizeY)
  , data(sizeX * sizeY)
{
  std::fill(begin(data), end(data), init);
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
DataType Grid<DataType>::operator[](Position position) const
{
  return data[position.getY() * sizeX + position.getX()];
}
