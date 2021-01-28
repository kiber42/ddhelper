#pragma once

#include "Monster.hpp"

#include <optional>

void addMonsterToPool(Monster newMonster, Monsters& pool);

std::optional<Monster> runMonsterPool(Monsters& monsters);
