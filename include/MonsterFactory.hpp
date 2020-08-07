#pragma once

#include "MonsterStats.hpp"

#include <memory>

MonsterStats makeGenericMonsterStats(int level, int hp, int damage, int deathProtection = 0);
