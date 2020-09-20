#include "Spells.hpp"

#include "Combat.hpp"

#include <cassert>

namespace Cast
{
  namespace
  {
    // Rules for Burndayray glyph:
    // 4 damage per character level, 1 burn stack (max 2 burn stacks per character level)
    // (existing burn stack size is added to damage as usual)
    // Effects to be accounted for:
    // Classes:
    // - Wizard: double burn stacks (Magic Attunement)
    // God boons:
    // - Flames (Mystera Annur), +1 dmg per caster level
    // Items:
    // - Avatar's Codex -> Heavy Fireball: +4 dmg per caster level, instant max burn stacks, monster will
    // retaliate if not defeated
    // - Battlemage Ring, +1 dmg per caster level
    // - Witchalok pendant: add stone skin when burndayraz is cast
    // Note: Attack counts for Crusader's momentum
    Outcome::Debuffs burndayraz(Hero& hero, Monster& monster)
    {
      using Summary = Outcome::Summary;
      const bool heavy = hero.hasStatus(HeroStatus::HeavyFireball);
      const bool monsterSlowed = monster.isSlowed();
      const int multiplier =
          4 + hero.hasTrait(HeroTrait::Flames) + (heavy ? 4 : 0) /* + (hero.hasItem(Items::BattlemageRing) ? 1 : 0) */;
      monster.takeFireballDamage(hero.getLevel(), multiplier);
      const int maxBurnStackSize = 2 * hero.getLevel();
      if (heavy)
        monster.burnMax(maxBurnStackSize);
      else if (hero.hasTrait(HeroTrait::MagicAttunement))
        monster.burn(maxBurnStackSize);
      if (!monster.isDefeated() && !monsterSlowed && (heavy || monster.doesRetaliate()))
        return Combat::retaliate(hero, monster);
      if (monster.isDefeated())
      {
        if (monster.bearsCurse())
        {
          hero.addStatus(HeroStatus::Cursed);
          return {Outcome::Debuff::Cursed};
        }
        else
          hero.removeStatus(HeroStatus::Cursed, false);
      }
      return {};
    }
  } // namespace

  constexpr int baseSpellCosts(Spell spell)
  {
    switch (spell)
    {
    case Spell::Apheelsik:
      return 5;
    case Spell::Bludtupowa:
      return 0;
    case Spell::Burndayraz:
      return 6;
    case Spell::Bysseps:
      return 2;
    case Spell::Cydstepp:
      return 10;
    case Spell::Endiswal:
      return 6;
    case Spell::Getindare:
      return 3;
    case Spell::Halpmeh:
      return 5;
    case Spell::Imawal:
      return 5;
    case Spell::Lemmisi:
      return 2;
    case Spell::Pisorf:
      return 4;
    case Spell::Weytwut:
      return 8;
    case Spell::Wonafyt:
      return 5;
    }
  }

  int spellCosts(Spell spell, const Hero& hero)
  {
    // Effects to be accounted for:
    // Classes:
    // Chemist: can have multiple layers of bysseps, cost doubles per layer (Additives trait)
    // Wizard: costs -1 (Magic Affinity trait)
    // God boons:
    // Mystic Balance (Mystera Annur), glyph costs change by +/-2 toward 5 (evaluated after Magic Affinity!)
    int costs = baseSpellCosts(spell);
    if (hero.hasTrait(HeroTrait::Mageslay))
      costs += 2;
    if (hero.hasTrait(HeroTrait::MagicAffinity))
      costs -= 1;
    return costs;
  }

  bool isPossible(const Hero& hero, Spell spell)
  {
    const bool validWithoutTarget = !needsMonster(spell);
    assert(validWithoutTarget);
    return validWithoutTarget && hero.getManaPoints() >= spellCosts(spell, hero) &&
           (spell != Spell::Bludtupowa || hero.getHitPoints() > 3 * hero.getLevel()) &&
           (spell != Spell::Bysseps || !hero.hasStatus(HeroStatus::Might) || hero.hasTrait(HeroTrait::Additives)) &&
           (spell != Spell::Cydstepp ||
            (!hero.hasStatus(HeroStatus::DeathProtection) &&
             (hero.getHitPoints() * 2 >= hero.getHitPointsMax() || hero.hasTrait(HeroTrait::Defiant)))) &&
           (spell != Spell::Getindare || !hero.hasStatus(HeroStatus::FirstStrikeTemporary)) &&
           (spell != Spell::Halpmeh || hero.hasStatus(HeroStatus::Poisoned) || hero.getHitPoints() < hero.getHitPointsMax());
  }

  bool isPossible(const Hero& hero, const Monster& monster, Spell spell)
  {
    if (!needsMonster(spell))
      return isPossible(hero, spell);
    return hero.getManaPoints() >= spellCosts(spell, hero) && monster.getMagicalResistPercent() < 100 &&
           (spell != Spell::Apheelsik || !monster.isUndead()) &&
           (spell != Spell::Wonafyt || hero.getLevel() >= monster.getLevel());
  }

  void applyCastingSideEffects(Hero& hero, int manaCosts)
  {
    if (hero.isDefeated())
      return;
    // Sorcerer: Every mana point spent regenerates 2 health (Essence Transit)
    if (hero.hasTrait(HeroTrait::EssenceTransit))
      hero.healHitPoints(2 * manaCosts);
    // Transmuter: Gain conversion points (Inner Focus)
    // Dragon Soul (15% free cast chance)
    // God Likes and Dislikes
  }

  Hero untargeted(Hero hero, Spell spell)
  {
    if (!isPossible(hero, spell))
      return hero;

    const int manaCosts = spellCosts(spell, hero);
    hero.loseManaPoints(manaCosts);

    switch (spell)
    {
    case Spell::Bludtupowa:
    {
      // uncover 3 tiles without health recovery (neither hero nor monsters)
      const int uncoveredTiles = 3;
      hero.loseHitPointsOutsideOfFight(3 * hero.getLevel());
      hero.recoverManaPoints(uncoveredTiles);
      break;
    }
    case Spell::Bysseps:
      hero.addStatus(HeroStatus::Might);
      break;
    case Spell::Cydstepp:
      hero.addStatus(HeroStatus::DeathProtection);
      break;
    case Spell::Endiswal:
      hero.changePhysicalResistPercent(+20);
      break;
    case Spell::Getindare:
      // first strike, +5% dodge chance (until actual dodge)
      hero.addStatus(HeroStatus::FirstStrike);
      hero.addStatus(HeroStatus::FirstStrikeTemporary);
      hero.addDodgeChangePercent(5, false);
      break;
    case Spell::Halpmeh:
      hero.healHitPoints(hero.getLevel() * (hero.hasTrait(HeroTrait::HolyHands) ? 5 : 4));
      hero.removeStatus(HeroStatus::Poisoned, true);
      break;
    case Spell::Imawal:
      // No real effect if not targeting monster
      hero.recoverManaPoints(2);
      break;
    case Spell::Lemmisi:
    {
      // reveals 3 tiles, recovery of hero (and monster)
      const int uncoveredTiles = 3;
      hero.recover(uncoveredTiles);
      break;
    }
    default:
      assert(false);
    }

    applyCastingSideEffects(hero, manaCosts);
    return hero;
  }

  Outcome predictOutcome(const Hero& hero, const Monster& monster, Spell spell)
  {
    using Summary = Outcome::Summary;
    Outcome outcome{Summary::Safe, {}, hero, monster};

    if (!isPossible(hero, monster, spell))
    {
      outcome.summary = Summary::NotPossible;
    }
    else
    {
      if (!needsMonster(spell) && !monsterIsOptional(spell))
        outcome.hero = untargeted(std::move(outcome.hero), spell);
      else
      {
        const int manaCosts = spellCosts(spell, hero);
        outcome.hero.loseManaPoints(manaCosts);

        switch (spell)
        {
        case Spell::Apheelsik:
          outcome.monster->poison(10 * hero.getLevel());
          break;
        case Spell::Burndayraz:
          outcome.debuffs = burndayraz(outcome.hero, *outcome.monster);
          break;
        case Spell::Imawal:
          outcome.monster->petrify();
          outcome.hero.addStatus(HeroStatus::ExperienceBoost);
          break;
        case Spell::Lemmisi:
        {
          const int uncoveredTiles = 3;
          outcome.hero.recover(uncoveredTiles);
          outcome.monster->recover(uncoveredTiles);
          break;
        }
        case Spell::Pisorf:
          // 60% of base damage as knockback damage if against wall
          // (TODO?) 50% of base damage as knockback damage if against enemy
          outcome.monster->takeDamage(hero.getBaseDamage() * 6 / 10, false);
          break;
        case Spell::Weytwut:
          // adds Slowed to monster (no blink, retreat, retaliation, +1 bonus XP)
          outcome.monster->slow();
          break;
        case Spell::Wonafyt:
          // adds Slowed to monster, can only be cast against same or lower level
          outcome.monster->slow();
          break;
        default:
          assert(false);
          break;
        }

        applyCastingSideEffects(outcome.hero, manaCosts);

        outcome.summary = Combat::summaryAndExperience(outcome.hero, *outcome.monster, monster.isSlowed());
      }
    }

    return outcome;
  }
} // namespace Cast
