#include "engine/Magic.hpp"

#include "engine/Combat.hpp"
#include "engine/Items.hpp"

#include <cassert>
#include <iostream>

namespace Magic
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
    void burndayraz(Hero& hero, Monster& monster, Monsters& allMonsters)
    {
      const bool heavy = hero.has(HeroStatus::HeavyFireball);
      const bool monsterSlowed = monster.isSlowed();
      const unsigned multiplier =
          4 + hero.has(Boon::Flames) + (heavy ? 4 : 0) + (hero.has(ShopItem::BattlemageRing) ? 1 : 0);

      // Damage and burning
      monster.takeFireballDamage(hero.getLevel(), multiplier);
      if (hero.has(ShopItem::PiercingWand))
        monster.erodeResitances();
      const auto maxBurnStackSize = 2 * hero.getLevel();
      if (heavy)
        monster.burnMax(maxBurnStackSize);
      else if (hero.has(HeroTrait::MagicAttunement))
        monster.burn(maxBurnStackSize);

      // Retaliation
      if (!monster.isDefeated() && !monsterSlowed && (heavy || monster.has(MonsterTrait::Retaliate)))
      {
        hero.takeDamage(monster.getDamage() / 2, monster.damageType(), allMonsters);
        if (hero.has(HeroTrait::ManaShield))
          monster.takeManaShieldDamage(hero.getLevel());
      }

      // Side effects
      if (hero.has(ShopItem::WitchalokPendant))
        hero.add(HeroStatus::StoneSkin);

      hero.adjustMomentum(monster.isDefeated());
    }

    void applyCastingSideEffects(Hero& hero, unsigned manaCosts, Monsters& allMonsters)
    {
      if (hero.isDefeated())
        return;
      if (hero.has(HeroTrait::EssenceTransit))
        hero.healHitPoints(2 * manaCosts);
      if (hero.has(HeroTrait::InnerFocus))
        hero.addConversionPoints(3, allMonsters);
      if (hero.has(ShopItem::DragonSoul))
        hero.applyDragonSoul(manaCosts);
      if (manaCosts >= 3)
      {
        if (hero.has(ShopItem::FireHeart))
          hero.chargeFireHeart();
        if (hero.has(ShopItem::CrystalBall))
          hero.chargeCrystalBall();
      }
    }

    constexpr unsigned baseSpellCosts(Spell spell)
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
  } // namespace

  unsigned spellCosts(Spell spell, const Hero& hero)
  {
    unsigned costs = baseSpellCosts(spell);
    if (spell == Spell::Bysseps)
      costs <<= hero.getIntensity(HeroStatus::ByssepsStacks);
    if (hero.has(HeroTrait::Mageslay))
      costs += 2;
    if (hero.has(HeroTrait::MagicAffinity))
      costs -= 1;
    if (hero.has(Boon::MysticBalance))
    {
      if (costs > 5)
        costs -= 2;
      else if (costs > 0 && costs < 5)
        costs += 2;
    }
    return costs;
  }

  unsigned healthCostsBludtupowa(const Hero& hero)
  {
    if (hero.has(HeroTrait::EssenceTransit))
      return hero.getLevel() * 3 + 4;
    return hero.getLevel() * 3;
  }

  bool isPossible(const Hero& hero, Spell spell, const Resources& resources)
  {
    const bool validWithoutTarget = !needsMonster(spell);
    if (hero.getManaPoints() < spellCosts(spell, hero) || !validWithoutTarget)
      return false;

    switch (spell)
    {
    case Spell::Bludtupowa:
      return hero.getHitPoints() > healthCostsBludtupowa(hero);
    case Spell::Bysseps:
      return !hero.has(HeroStatus::Might) || hero.has(HeroTrait::Additives);
    case Spell::Cydstepp:
      return !hero.has(HeroStatus::DeathProtection) &&
             (hero.getHitPoints() * 2 >= hero.getHitPointsMax() || hero.has(HeroTrait::Defiant));
    case Spell::Endiswal:
      return resources().numWalls > 0;
    case Spell::Getindare:
      return !hero.has(HeroStatus::FirstStrikeTemporary);
    case Spell::Halpmeh:
      return hero.getHitPoints() < hero.getHitPointsMax() || hero.has(HeroDebuff::Poisoned);
    case Spell::Pisorf:
      // TODO: Currently Pisorf is assumed to always push a monster into a (visible) wall
      return resources().numWalls > 0;
    default:
      return true;
    }
  }

  bool isPossible(const Hero& hero, const Monster& monster, Spell spell, const Resources& resources)
  {
    if (!needsMonster(spell))
      return isPossible(hero, spell, resources);
    if (hero.getManaPoints() < spellCosts(spell, hero) || monster.getMagicalResistPercent() >= 100)
      return false;
    switch (spell)
    {
    case Spell::Apheelsik:
      return !monster.has(MonsterTrait::Undead);
    case Spell::Wonafyt:
      return hero.getLevel() >= monster.getLevel();
    default:
      return true;
    }
  }

  void cast(Hero& hero, Spell spell, Monsters& allMonsters, Resources& resources)
  {
    if (!isPossible(hero, spell, resources))
    {
      std::cerr << "Cast is not possible." << std::endl;
      return;
    }
    if (hero.isDefeated())
    {
      std::cerr << "Dead hero cannot cast." << std::endl;
      return;
    }

    const auto manaCosts = spellCosts(spell, hero);
    hero.loseManaPoints(manaCosts);

    PietyCollection collectPiety(hero);

    switch (spell)
    {
    case Spell::Bludtupowa:
    {
      // uncover up to 3 tiles, monsters do not recover, hero converts health to mana
      const auto uncoveredTiles = std::min(3u, resources.numHiddenTiles);
      resources.revealTiles(uncoveredTiles);
      if (hero.getFollowedDeity() != God::GlowingGuardian)
      {
        hero.loseHitPointsOutsideOfFight(healthCostsBludtupowa(hero), allMonsters);
        hero.recoverManaPoints(uncoveredTiles);
      }
      break;
    }
    case Spell::Bysseps:
      hero.add(HeroStatus::Might);
      hero.add(HeroStatus::ByssepsStacks);
      break;
    case Spell::Cydstepp:
      hero.add(HeroStatus::DeathProtection);
      break;
    case Spell::Endiswal:
      --(resources().numWalls);
      hero.wallDestroyed();
      hero.add(HeroStatus::StoneSkin);
      break;
    case Spell::Getindare:
      // first strike, +5% dodge chance (until actual dodge)
      hero.add(HeroStatus::FirstStrikeTemporary);
      hero.addDodgeChancePercent(5, false);
      break;
    case Spell::Halpmeh:
      hero.healHitPoints(hero.getLevel() * (hero.has(HeroTrait::HolyHands) ? 5 : 4));
      hero.reset(HeroDebuff::Poisoned);
      break;
    case Spell::Imawal:
      hero.recoverManaPoints(2);
      break;
    case Spell::Lemmisi:
    {
      const auto uncoveredTiles = std::min(3u, resources.numHiddenTiles);
      resources.numHiddenTiles -= uncoveredTiles;
      hero.recover(uncoveredTiles, allMonsters);
      for (auto& monster : allMonsters)
        monster.recover(uncoveredTiles);
      break;
    }
    default:
      assert(false);
    }

    // Imawal changes its function when cast without target
    if (spell != Spell::Imawal)
      collectPiety(hero.getFaith().spellCast(spell, manaCosts));
    else
      collectPiety(hero.getFaith().imawalCreateWall(manaCosts));
    hero.applyCollectedPiety(allMonsters);

    applyCastingSideEffects(hero, manaCosts, allMonsters);
  }

  Summary cast(Hero& hero, Monster& monster, Spell spell, Monsters& allMonsters, Resources& resources)
  {
    if (!needsMonster(spell) && !monsterIsOptional(spell))
    {
      Magic::cast(hero, spell, allMonsters, resources);
      return Summary::Safe;
    }

    if (!isPossible(hero, monster, spell, resources))
    {
      std::cerr << "Cast is not possible." << std::endl;
      return Summary::NotPossible;
    }

    if (monster.isDefeated())
    {
      std::cerr << "Cannot cast against defeated monster." << std::endl;
      return Summary::NotPossible;
    }

    const bool monsterWasSlowed = monster.isSlowed();
    const bool monsterWasBurning = monster.isBurning();

    const auto manaCosts = spellCosts(spell, hero);
    hero.loseManaPoints(manaCosts);

    PietyCollection collection(hero);

    switch (spell)
    {
    case Spell::Apheelsik:
      if (monster.poison(10 * hero.getLevel()))
        collection(hero.getFaith().monsterPoisoned(monster));
      break;
    case Spell::Burndayraz:
      burndayraz(hero, monster, allMonsters);
      break;
    case Spell::Imawal:
      // Imawal is handled below, since no XP is awarded for the kill (except +1 bonus XP for slowing)
      break;
    case Spell::Lemmisi:
    {
      const auto uncoveredTiles = std::min(resources.numHiddenTiles, 3u);
      resources.revealTiles(uncoveredTiles);
      hero.recover(uncoveredTiles, allMonsters);
      monster.recover(uncoveredTiles);
      break;
    }
    case Spell::Pisorf:
    {
      // 60% of base damage as physical damage if against wall
      // (TODO?) slightly less than 50% of base damage as physical damage if against enemy + corrosion of first enemy as
      // typeless damage. Net damage to first enemy is applied as typeless damage to second enemy; second enemy cannot
      // drop below 1 HP.
      auto& numWalls = resources().numWalls;
      if (numWalls > 0)
      {
        --numWalls;
        hero.wallDestroyed();
        monster.takeDamage(hero.getBaseDamage() * 6 / 10, DamageType::Physical);
        hero.reset(HeroStatus::SpiritStrength);
      }
      break;
    }
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

    collection(hero.getFaith().spellCast(spell, manaCosts));
    applyCastingSideEffects(hero, manaCosts, allMonsters);

    if (spell == Spell::Imawal)
    {
      const auto levelBefore = hero.getLevel() + hero.getPrestige();
      if (monster.grantsXP())
        hero.gainExperienceForPetrification(monster.isSlowed(), allMonsters);
      monster.petrify(resources);
      hero.add(HeroStatus::ExperienceBoost);
      // Remove one curse stack, even for cursed monsters
      hero.reduce(HeroDebuff::Cursed);
      hero.applyCollectedPiety(allMonsters);
      ++(resources().numGoldPiles);
      return hero.getLevel() + hero.getPrestige() > levelBefore ? Summary::LevelUp : Summary::Safe;
    }

    const bool triggerBurnDown = spell == Spell::Burndayraz || spell == Spell::Pisorf;
    return Combat::detail::finalizeAttack(hero, monster, monsterWasSlowed, monsterWasBurning, triggerBurnDown,
                                          allMonsters, resources);
  }
} // namespace Magic
