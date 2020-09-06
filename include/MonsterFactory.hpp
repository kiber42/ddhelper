#pragma once

#include "Monster.hpp"
#include "MonsterStats.hpp"

Monster makeStandardMonster(int level);

Monster makeGenericMonster(int level, int hp, int damage);
MonsterStats makeGenericMonsterStats(int level, int hp, int damage, int deathProtection = 0);
