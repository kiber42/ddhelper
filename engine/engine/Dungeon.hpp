#pragma once

class Hero;
class Monster;

#include "engine/Grid.hpp"
#include "engine/PositionedVector.hpp"

#include <memory>
#include <optional>
#include <vector>

class Dungeon
{
public:
  Dungeon(unsigned sizeX, unsigned sizeY);
  void setHero(std::shared_ptr<Hero> hero, Position position);

  void add(std::shared_ptr<Monster> monster, Position position);
  std::vector<std::shared_ptr<Monster>> getMonsters();

  std::optional<Position> randomFreePosition() const;

  //! tile can be stepped on in general and there's currently no obstacle on it
  bool isFree(Position position) const;

  //! tile can be stepped on
  bool isGround(Position position) const;
  //! tile can not be stepped on but can be crushed (knock-back, endiswall, ...)
  bool isWall(Position position) const;
  //! tile can not be stepped on or manipulated in any way
  bool isPermanentWall(Position position) const;

  bool isRevealed(Position position) const;
  void reveal(Position position);
  void revealOne(Position position);

  /** Brings dungeon into a consistent state.
   *
   *  At the moment, this only removes defeated monsters from dungeon.
   */
  void update();

  //! Pathfinding methods

  //! Check whether the hero can access a position (path revealed and not blocked)
  bool isAccessible(Position position) const;

  //! Check whether the hero can access a position in principle (path may be unrevealed or blocked)
  bool isConnected(Position position) const;

private:
  unsigned sizeX, sizeY;
  Positioned<std::shared_ptr<Hero>> hero;
  PositionedVector<std::shared_ptr<Monster>> monsters;
  Grid<bool> revealed;
  Grid<bool> wall;
  Grid<bool> permanentWall;

  bool pathfinder(Position position, bool mustBeRevealed, bool mustNotBeBlocked) const;
};
