#pragma once

#include <memory>
#include <utility>
#include <vector>

class Position
{
public:
  Position() = default;

  Position(unsigned x, unsigned y)
    : x(x)
    , y(y)
  {
  }

  unsigned getX() const { return x; }
  unsigned getY() const { return y; }

  auto operator<=>(const Position&) const = default;

private:
  unsigned x{0};
  unsigned y{0};
};

template <class Object>
class Positioned : public Object
{
public:
  Positioned() = default;
  Positioned(Object object, Position position)
    : Object(std::move(object))
    , _the_position(std::move(position))
  {
  }
  virtual ~Positioned() = default;

  Position getPosition() const { return _the_position; }
  void setPosition(Position position) { _the_position = position; }
  Object& get() { return *this; }

private:
  Position _the_position;
};

template <class Object>
Positioned<Object> make_positioned(Object object, Position position)
{
  return Positioned<Object>(std::move(object), std::move(position));
}

template <class Item>
class PositionedVector : public std::vector<Positioned<Item>>
{
public:
  void add(Item item, Position position);
  bool anyAt(Position) const;
  std::vector<Item> getAt(Position position);
  std::vector<Item> getAll();
};

template <class Item>
void PositionedVector<Item>::add(Item item, Position position)
{
  std::vector<Positioned<Item>>::emplace_back(make_positioned(std::move(item), std::move(position)));
}

template <class Item>
bool PositionedVector<Item>::anyAt(Position position) const
{
  for (auto& item : *this)
    if (item.getPosition() == position)
      return true;
  return false;
}

template <class Item>
std::vector<Item> PositionedVector<Item>::getAt(Position position)
{
  std::vector<Item> itemsAtPosition;
  for (auto& item : *this)
    if (item.getPosition() == position)
      itemsAtPosition.emplace_back(item.get());
  return itemsAtPosition;
}

template <class Item>
std::vector<Item> PositionedVector<Item>::getAll()
{
  std::vector<Item> allItems;
  for (auto& item : *this)
    allItems.emplace_back(item.get());
  return allItems;
}
