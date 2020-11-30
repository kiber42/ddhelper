#include "Solution.hpp"

#include <numeric>

using namespace std::string_literals;

template <class... Ts>
struct overloaded : Ts...
{
  using Ts::operator()...;
};

template <class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

std::string toString(const Step& step)
{
  return std::visit(overloaded{
      [](Attack) { return "Attack"s; },
      [](Cast cast) { return "Cast "s + toString(cast.spell); },
      [](Uncover uncover) { return "Uncover " + std::to_string(uncover.numTiles) + " tile(s)"; },
      [](Buy buy) { return "Buy "s + toString(buy.item); },
      [](Use use) { return "Use "s + toString(use.item); },
      [](Convert convert) {
        return "Convert "s + std::visit(overloaded{
          [](Item item) -> std::string { return toString(item); },
          [](Spell spell) -> std::string { return toString(spell); }
        }, convert.itemOrSpell); },
      [](Find find) { return "Find "s + toString(find.spell); },
      [](Follow follow) { return "Follow "s + toString(follow.deity); },
      [](Request request) { return "Request "s + toString(request.boon); },
      [](Desecrate desecrate) { return "Desecrate "s + toString(desecrate.altar); }
  }, step);
}

std::string toString(const Solution& solution)
{
  if (solution.empty())
    return "Empty solution";
  std::string description =
      std::accumulate(begin(solution), end(solution), "Solution:"s,
                      [](const std::string& str, const Step step) { return str + ' ' + toString(step) + ','; });
  description.pop_back();
  return description;
}
