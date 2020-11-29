#include "Solution.hpp"

#include <iostream>

int main()
{
  Solution solution;
  solution.emplace_back(Attack{});
  solution.emplace_back(Uncover{5});
  solution.emplace_back(Buy{Item::Spoon});
  solution.emplace_back(Find{Spell::Burndayraz});
  solution.emplace_back(Cast{Spell::Getindare});
  solution.emplace_back(Follow{God::Taurog});
  solution.emplace_back(Convert{Spell::Apheelsik});
  solution.emplace_back(Request{Boon::Humility});
  solution.emplace_back(Desecrate{God::BinlorIronshield});
  solution.emplace_back(Use{Item::BattlemageRing});
  std::cout << toString(solution) << "\n";
  return 0;
}
