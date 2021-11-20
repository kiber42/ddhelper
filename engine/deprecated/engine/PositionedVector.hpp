#pragma once

#include <memory>
#include <utility>
#include <vector>

struct Position
{
  unsigned x{0};
  unsigned y{0};
  auto operator<=>(const Position& other) const = default;
};

template <class Object>
using Positioned = std::pair<Object, Position>;

template <class Object>
Positioned<Object> make_positioned(Object&& object, Position position)
{
  return {std::forward<Object>(object), std::move(position)};
}

template <class Object>
class PositionedVector : public std::vector<Positioned<Object>>
{
public:
  void add(Object object, Position position);
  bool anyAt(Position) const;
  std::vector<Object> getAt(Position position) const;
  std::vector<Object> getAll() const;
};

template <class Object>
void PositionedVector<Object>::add(Object object, Position position)
{
  PositionedVector<Object>::emplace_back(std::move(object), std::move(position));
}

template <class Object>
bool PositionedVector<Object>::anyAt(Position position) const
{
  return std::any_of(begin(*this), end(*this),
                     [position](auto& positionedObject) { return positionedObject.second == position; });
}

template <class Object>
std::vector<Object> PositionedVector<Object>::getAt(Position position) const
{
  std::vector<Object> objectsAtPosition;
  for (auto& object : *this)
    if (object.first == position)
      objectsAtPosition.emplace_back(object.first);
  return objectsAtPosition;
}

template <class Object>
std::vector<Object> PositionedVector<Object>::getAll() const
{
  std::vector<Object> allObjects;
  for (auto& object : *this)
    allObjects.emplace_back(object.first);
  return allObjects;
}
