#include "engine/Dungeon.hpp"

#include "engine/Hero.hpp"
#include "engine/Monster.hpp"

#include <algorithm>
#include <numeric>
#include <random>

#include <cassert>

Dungeon::Dungeon(unsigned sizeX, unsigned sizeY)
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
  std::mt19937 generator{std::random_device{}()};
} // namespace

std::optional<Position> Dungeon::randomFreePosition() const
{
  std::vector<unsigned> indices(sizeX * sizeY);
  std::iota(begin(indices), end(indices), 0u);
  std::shuffle(begin(indices), end(indices), generator);
  for (unsigned index : indices)
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
  for (unsigned dy = -1u; dy <= +1; ++dy)
    for (unsigned dx = -1u; dx <= +1; ++dx)
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
  monsters.erase(std::remove_if(begin(monsters), end(monsters), [](auto& monster) { return monster->isDefeated(); }));
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
  Grid<unsigned> path(sizeX, sizeY, -1u);
  Grid<bool> updated(sizeX, sizeY, false);
  assert(path.isValid(hero.getPosition()));
  assert(path.isValid(position));
  for (unsigned x = 0; x < sizeX; ++x)
  {
    for (unsigned y = 0; y < sizeY; ++y)
    {
      Position pos(x, y);
      if (!(mustNotBeBlocked ? isFree(pos) : isGround(pos)) || (mustBeRevealed && !isRevealed(pos)))
      {
        path[pos] = -1u;
      }
    }
  }
  path[hero.getPosition()] = 1;
  updated[hero.getPosition()] = true;
  bool anyUpdate;
  do
  {
    anyUpdate = false;
    for (unsigned x = 0; x < sizeX; ++x)
    {
      for (unsigned y = 0; y < sizeY; ++y)
      {
        const Position p0(x, y);
        if (updated[p0])
        {
          unsigned dist = path[p0] + 1;
          for (unsigned dx = -1u; dx <= +1; ++dx)
          {
            for (unsigned dy = -1u; dy <= +1; ++dy)
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
