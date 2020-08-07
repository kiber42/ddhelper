#include "Dungeon.hpp"

#include "Hero.hpp"
#include "Monster.hpp"

#include <algorithm>
#include <numeric>
#include <random>

#include <cassert>

Dungeon::Dungeon(int sizeX, int sizeY)
  : sizeX(sizeX)
  , sizeY(sizeY)
  , revealed(sizeX, sizeY, false)
  , wall(sizeX, sizeY, false)
  , permanentWall(sizeX, sizeY, false)
{
}

void Dungeon::setHero(std::shared_ptr<Hero> newHero, Position position)
{
  hero = make_positioned(newHero, position);
  reveal(position);
}

void Dungeon::add(std::shared_ptr<Monster> monster, Position position)
{
  monsters.add(monster, position);
}

std::vector<std::shared_ptr<Monster>> Dungeon::getMonsters()
{
  return monsters.getAll();
}

namespace
{
std::random_device rd;
std::mt19937 g(rd());
} // namespace

std::optional<Position> Dungeon::randomFreePosition() const
{
  std::vector<int> indices(sizeX * sizeY);
  std::iota(begin(indices), end(indices), 0);
  std::shuffle(begin(indices), end(indices), g);
  for (int index : indices)
  {
    Position position(index % sizeX, index / sizeX);
    if (isFree(position))
    {
      return position;
    }
  }
  return std::nullopt;
}

bool Dungeon::isFree(Position position) const
{
  return !monsters.anyAt(position) && (!hero || hero.getPosition() != position);
}

bool Dungeon::isGround(Position position) const
{
  return !(wall[position] || permanentWall[position]);
}

bool Dungeon::isWall(Position position) const
{
  return wall[position];
}

bool Dungeon::isPermanentWall(Position position) const
{
  return permanentWall[position];
}

bool Dungeon::isRevealed(Position position) const
{
  return revealed[position];
}

void Dungeon::reveal(Position position)
{
  for (int dy = -1; dy <= +1; ++dy)
    for (int dx = -1; dx <= +1; ++dx)
    {
      Position posToReveal(position.getX() + dx, position.getY() + dy);
      if (revealed.isValid(posToReveal))
        revealed[posToReveal] = true;
    }
}

void Dungeon::revealOne(Position position)
{
  revealed[position] = true;
}

void Dungeon::update()
{
  monsters.erase(std::remove_if(begin(monsters), end(monsters), [](auto &monster) { return monster->isDefeated(); }));
}

bool Dungeon::isAccessible(Position position) const
{
  return pathfinder(position, true, true);
}

bool Dungeon::isConnected(Position position) const
{
  return pathfinder(position, false, false);
}

bool Dungeon::pathfinder(Position position, bool mustBeRevealed, bool mustNotBeBlocked) const
{
  Grid<int> path(sizeX, sizeY, sizeX * sizeY);
  Grid<bool> updated(sizeX, sizeY, false);
  assert(path.isValid(hero.getPosition()));
  assert(path.isValid(position));
  for (int x = 0; x < sizeX; ++x)
  {
    for (int y = 0; y < sizeY; ++y)
    {
      Position pos(x, y);
      if (!(mustNotBeBlocked ? isFree(pos) : isGround(pos)) || (mustBeRevealed && !isRevealed(pos)))
      {
        path[pos] = -1;
      }
    }
  }
  path[hero.getPosition()] = 1;
  updated[hero.getPosition()] = true;
  bool anyUpdate;
  do
  {
    anyUpdate = false;
    for (int x = 0; x < sizeX; ++x)
    {
      for (int y = 0; y < sizeY; ++y)
      {
        const Position p0(x, y);
        if (updated[p0])
        {
          int dist = path[p0] + 1;
          for (int dx = -1; dx <= +1; ++dx)
          {
            for (int dy = -1; dy <= +1; ++dy)
            {
              const Position p1(x + dx, y + dy);
              if (path.isValid(p1) && dist < path[p1])
              {
                if (p1 == position)
                {
                  return true;
                }
                path[p1] = dist;
                updated[p1] = true;
                anyUpdate = true;
              }
            }
          }
          updated[p0] = false;
        }
      }
    }
  } while (anyUpdate);
  return false;
}
