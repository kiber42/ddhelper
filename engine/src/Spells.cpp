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
    void burndayraz(Hero& hero, Monster& monster)
    {
      const bool heavy = hero.hasStatus(HeroStatus::HeavyFireball);
      const bool monsterSlowed = monster.isSlowed();
      const bool monsterWasBurning = monster.isBurning();
      const int multiplier =
          4 + hero.hasBoon(Boon::Flames) + (heavy ? 4 : 0) /* + (hero.hasItem(Items::BattlemageRing) ? 1 : 0) */;

      // Damage and burning
      monster.takeFireballDamage(hero.getLevel(), multiplier);
      const int maxBurnStackSize = 2 * hero.getLevel();
      if (heavy)
        monster.burnMax(maxBurnStackSize);
      else if (hero.hasTrait(HeroTrait::MagicAttunement))
        monster.burn(maxBurnStackSize);

      // Retaliation
      if (!monster.isDefeated() && !monsterSlowed && (heavy || monster.doesRetaliate()))
      {
        hero.takeDamage(monster.getDamage() / 2, monster.doesMagicalDamage());
        if (hero.hasTrait(HeroTrait::ManaShield))
          monster.takeManaShieldDamage(hero.getLevel());
      }

      if (monster.isDefeated())
        hero.collect(hero.getFaith().monsterKilled(monster, hero.getLevel(), monsterWasBurning));

      // Curses
      Combat::detail::monsterDefeatedCurse(hero, monster);
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
           (spell != Spell::Halpmeh || hero.hasStatus(HeroStatus::Poisoned) ||
            hero.getHitPoints() < hero.getHitPointsMax());
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
  }

  void untargeted(Hero& hero, Spell spell)
  {
    if (!isPossible(hero, spell))
      return;

    const int manaCosts = spellCosts(spell, hero);
    hero.loseManaPoints(manaCosts);

    hero.startPietyCollection();

    switch (spell)
    {
    case Spell::Bludtupowa:
    {
      if (hero.getFollowedDeity() != God::GlowingGuardian)
      {
        // uncover 3 tiles without health recovery (neither hero nor monsters)
        const int uncoveredTiles = 3;
        hero.loseHitPointsOutsideOfFight(3 * hero.getLevel());
        hero.recoverManaPoints(uncoveredTiles);
      }
      break;
    }
    case Spell::Bysseps:
      hero.addStatus(HeroStatus::Might);
      break;
    case Spell::Cydstepp:
      hero.addStatus(HeroStatus::DeathProtection);
      break;
    case Spell::Endiswal:
      hero.addStatus(HeroStatus::StoneSkin);
      if (hero.hasBoon(Boon::StoneForm))
        hero.addStatus(HeroStatus::Might);
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

    // Imawal changes its function when cast without target
    if (spell != Spell::Imawal)
      hero.collect(hero.getFaith().spellCast(spell, manaCosts));
    else
      hero.collect(hero.getFaith().imawalCreateWall(manaCosts));
    hero.applyCollectedPiety();

    applyCastingSideEffects(hero, manaCosts);
  }

  Summary targeted(Hero& hero, Monster& monster, Spell spell)
  {
    if (!isPossible(hero, monster, spell))
      return Summary::NotPossible;

    if (!needsMonster(spell) && !monsterIsOptional(spell))
    {
      untargeted(hero, spell);
      return Summary::Safe;
    }

    const bool monsterWasSlowed = monster.isSlowed();

    const int manaCosts = spellCosts(spell, hero);
    hero.loseManaPoints(manaCosts);

    hero.startPietyCollection();

    switch (spell)
    {
    case Spell::Apheelsik:
      if (monster.poison(10 * hero.getLevel()))
        hero.collect(hero.getFaith().monsterPoisoned(monster));
      break;
    case Spell::Burndayraz:
      burndayraz(hero, monster);
      break;
    case Spell::Imawal:
      // Imawal is handled below, since no XP is awarded for the kill (except +1 bonus XP for slowing)
      break;
    case Spell::Lemmisi:
    {
      const int uncoveredTiles = 3;
      hero.recover(uncoveredTiles);
      monster.recover(uncoveredTiles);
      break;
    }
    case Spell::Pisorf:
      // 60% of base damage as knockback damage if against wall
      // (TODO?) 50% of base damage as knockback damage if against enemy
      monster.takeDamage(hero.getBaseDamage() * 6 / 10, false);
      break;
    case Spell::Weytwut:
      // adds Slowed to monster (no blink, retreat, retaliation, +1 bonus XP)
      monster.slow();
      break;
    case Spell::Wonafyt:
      // adds Slowed to monster, can only be cast against same or lower level
      monster.slow();
      break;
    default:
      assert(false);
      break;
    }

    hero.collect(hero.getFaith().spellCast(spell, manaCosts));
    hero.applyCollectedPiety();

    applyCastingSideEffects(hero, manaCosts);

    if (spell != Spell::Imawal)
      return Combat::detail::summaryAndExperience(hero, monster, monsterWasSlowed);

    const bool levelBefore = hero.getLevel() + hero.getPrestige();
    if (monster.isSlowed())
      hero.gainExperience(0, true);
    monster.petrify();
    hero.addStatus(HeroStatus::ExperienceBoost);
    return hero.getLevel() + hero.getPrestige() > levelBefore ? Summary::LevelUp : Summary::Safe;
  }
} // namespace Cast
