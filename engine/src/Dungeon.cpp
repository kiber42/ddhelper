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
  for (unsigned dy = -1u; dy != +2; ++dy)
    for (unsigned dx = -1u; dx != +2; ++dx)
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
  Grid<unsigned> path(sizeX, sizeY, -1u);
  // Zero fields that should not be used in pathfinding
  path.setMatching([&](Position pos) { return !isRevealed(pos); }, 0u);
  for (const auto& monster : monsters)
    path[monster.getPosition()] = 0u;
  return pathfinder(hero.getPosition(), position, std::move(path));
}

bool Dungeon::isConnected(Position position) const
{
  Grid<unsigned> path(sizeX, sizeY, -1u);
  // Zero fields that should not be used in pathfinding
  path.setMatching([&](Position pos) { return !isGround(pos); }, 0u);
  return pathfinder(hero.getPosition(), position, std::move(path));
}

bool Dungeon::pathfinder(Position start, Position end, Grid<unsigned> path)
{
  assert(path.isValid(start));
  assert(path.isValid(end));
  Grid<bool> updated(path.sizeX, path.sizeY, false);
  path[start] = 1;
  updated[start] = true;
  bool anyUpdate;
  do
  {
    anyUpdate = false;
    const bool found = path.iterateOver([&](Position current) {
      if (!updated[current])
        return false;
      updated[current] = false;
      unsigned dist = path[current] + 1;
      if (dist == 1)
        return false;
      return path.iterateAround(current, [&](Position near) {
        assert(path.isValid(end));
        if (end == near)
          return true;
        if (dist < path[near])
        {
          path[near] = dist;
          updated[near] = true;
          anyUpdate = true;
        }
        return false;
      });
    });
    if (found)
      return true;
  } while (anyUpdate);
  return false;
}
