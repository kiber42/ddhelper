#include "Spells.hpp"

namespace Cast
{
  constexpr int baseSpellCosts(Spell spell)
  {
    switch (spell)
    {
      case Spell::Apheelsik: return 5;
      case Spell::Bludtupowa: return -3;
      case Spell::Burndayraz: return 6;
      case Spell::Bysseps: return 2;
      case Spell::Cydstepp: return 10;
      case Spell::Endiswal: return 6;
      case Spell::Getindare: return 3;
      case Spell::Halpmeh: return 5;
      case Spell::Imawal: return 5;
      case Spell::Lemmisi: return 2;
      case Spell::Pisorf: return 4;
      case Spell::Weytwut: return 8;
      case Spell::Wonafyt: return 5;
    }
  }

  int spellCosts(Spell spell, const Hero& /*hero*/)
  {
    // Effects to be accounted for:
    // Classes:
    // Berserker: cost +2 (Mageslay trait)
    // Chemist: can have multiple layers of bysseps, cost doubles per layer (Additives trait)
    // Wizard: costs -1 (Magic Affinity trait)
    // God boons:
    // Mystic Balance (Mystera Annur), glyph costs change by +/-2 toward 5 (evaluated after Magic Affinity!)
    return baseSpellCosts(spell);
  }

  bool isPossible(const Hero& hero, const Monster& monster, Spell spell)
  {
    return hero.getManaPoints() >= spellCosts(spell, hero) &&
    monster.getMagicalResistPercent() < 100 &&
    (spell != Spell::Apheelsik || !monster.isUndead()) &&
    (spell != Spell::Bludtupowa || hero.getHitPoints() > 3 * hero.getLevel()) &&
    (spell != Spell::Bysseps || !hero.hasStatus(HeroStatus::Might) || hero.hasTrait(HeroTrait::Additives)) &&
    (spell != Spell::Cydstepp || (hero.getDeathProtection() == 0 && (hero.getHitPoints() * 2 >= hero.getHitPointsMax() || hero.hasTrait(HeroTrait::Defiant)))) &&
    (spell != Spell::Getindare || !hero.hasStatus(HeroStatus::FirstStrike) /* TODO modify for Rogue */);
  }

  Outcome predictOutcome(const Hero& hero, const Monster& monster, Spell spell)
  {
    using Summary = Outcome::Summary;
    auto summary = Summary::Safe;
    Hero heroAfterFight(hero);
    Monster monsterAfterFight(monster);

    switch (spell)
    {
      case Spell::Apheelsik:
      // 10 poison per character level, not on undead
      break;
      case Spell::Bludtupowa:
      // costs 3 health per character level, 1 mana per revealed tile
      break;
      case Spell::Burndayraz:
      // 4 damage per character level, 1 burn stack (max 2 burn stacks per character level)
      // Effects to be accounted for:
      // Classes:
      // - Wizard: double burn stacks (Magic Attunement)
      // God boons:
      // - Flames (Mystera Annur), +1 dmg per caster level
      // Items:
      // - Avatar's Codex -> Heavy Fireball: +4 dmg per caster level, instant max burn stacks, monster will retaliate
      // - Battlemage Ring, +1 dmg per caster level
      break;
      case Spell::Bysseps:
      // adds Might (+30% dmg, +3% erode resistances)
      // Chemist: may cast repeatedly (Additives trait)
      break;
      case Spell::Cydstepp:
      // adds Death Protection
      // Only castable if no death protection yet and >=50 % max HP, except for Warlord (Defiant trait)
      break;
      case Spell::Endiswal:
      // temporary 20% physical resistance boost, stacking
      break;
      case Spell::Getindare:
      // first strike, +5% dodge chance (until actual dodge)
      break;
      case Spell::Halpmeh:
      // 4 health per character level
      // cures poison
      // Paladin: improved healing (Holy Hands trait)
      break;
      case Spell::Imawal:
      // petrifies enemy, no XP awarded, adds temporary XP bonus
      break;
      case Spell::Lemmisi:
      // reveals 3 tiles, recovery of hero and monster
      break;
      case Spell::Pisorf:
      // 60% of base damage as knockback damage if against wall
      // 50% of base damage as knockback damage if against enemy
      // ignores any magic resistance <100% (cannot cast against 100% magic resistance)
      break;
      case Spell::Weytwut:
      // adds Slowed to monster (no blink, retreat, retaliation, +1 bonus XP)
      break;
      case Spell::Wonafyt:
      // adds Slowed to monster, can only be cast against same or lower level
      break;
    }

    // Sorcerer: Every mana point spent regenerates 2 health (Essence Transit)

    // Dragon Soul (15% free cast chance)

    return Outcome{summary, std::move(heroAfterFight), std::move(monsterAfterFight)};
  }
} // namespace Cast
