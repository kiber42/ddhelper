#include "Utils.hpp"

ImVec4 colorSafe(0, 0.5f, 0, 1);
ImVec4 colorWin(0.2f, 1, 0.2f, 1);
ImVec4 colorDeath(1, 0, 0, 1);
ImVec4 colorLevelUp(0.5f, 1, 0.5f, 1);
ImVec4 colorNotPossible(0, 0, 0, 1);
ImVec4 colorDebuffedSafe(0, 0.5f, 0.5f, 1);
ImVec4 colorDebuffedWin(0, 0.8f, 0.8f, 1);
ImVec4 colorDebuffedLevelUp(0, 1, 1, 1);
ImVec4 colorUnavailable(0.7f, 0.7f, 0.7f, 1);

const ImVec4& summaryColor(Summary summary, bool debuffed)
{
  switch (summary)
  {
  case Summary::Safe:
    return debuffed ? colorDebuffedSafe : colorSafe;
  case Summary::Win:
    return debuffed ? colorDebuffedWin : colorWin;
  case Summary::Death:
  case Summary::Petrified:
    return colorDeath;
  case Summary::LevelUp:
    return debuffed ? colorDebuffedLevelUp : colorLevelUp;
  case Summary::NotPossible:
    return colorNotPossible;
  }
}

void showStatus(const Hero& hero)
{
  if (!hero.isDefeated())
  {
    ImGui::Text("%s level %i   %i/%i HP  %i/%i MP  damage %i (%i%+i%%)", hero.getName().c_str(), hero.getLevel(),
                hero.getHitPoints(), hero.getHitPointsMax(), hero.getManaPoints(), hero.getManaPointsMax(),
                hero.getDamageVersusStandard(), hero.getBaseDamage(), hero.getDamageBonusPercent());
    ImGui::Text("  %i/%i XP  %i/%i CP  %i piety  %i gold", hero.getXP(), hero.getXPforNextLevel(),
                hero.getConversionPoints(), hero.getConversionThreshold(), hero.getFaith().getPiety(), hero.gold());
    if (hero.getPhysicalResistPercent() > 0)
    {
      ImGui::SameLine();
      ImGui::Text(" %i%% physical resist", hero.getPhysicalResistPercent());
    }
    if (hero.getMagicalResistPercent() > 0)
    {
      ImGui::SameLine();
      ImGui::Text(" %i%% magic resist", hero.getMagicalResistPercent());
    }
    for (int i = 0; i < static_cast<int>(HeroStatus::Last); ++i)
    {
      auto status = static_cast<HeroStatus>(i);
      if (hero.hasStatus(status))
      {
        if (status == HeroStatus::DodgePrediction)
        {
          if (hero.predictDodgeNext())
            ImGui::Text("  will dodge next attack");
          else
            ImGui::Text("  won't dodge next attack");
          continue;
        }
        const bool useIs = status == HeroStatus::Cursed || status == HeroStatus::CurseImmune ||
                           status == HeroStatus::DeathGazeImmune || status == HeroStatus::Exhausted ||
                           status == HeroStatus::ManaBurned || status == HeroStatus::ManaBurnImmune ||
                           status == HeroStatus::Poisoned || status == HeroStatus::Poisonous ||
                           status == HeroStatus::PoisonImmune || status == HeroStatus::Weakened;
        if (canHaveMultiple(status))
          ImGui::Text("  %s %s (x%i)", useIs ? "is" : "has", toString(status), hero.getStatusIntensity(status));
        else
          ImGui::Text("  %s %s", useIs ? "is" : "has", toString(status));
      }
    }
    if (hero.getFollowedDeity())
      ImGui::Text("  follows %s", toString(*hero.getFollowedDeity()));
    for (auto boon :
         {Boon::StoneForm, Boon::BloodCurse, Boon::Humility, Boon::Petition, Boon::Flames, Boon::MysticBalance})
    {
      if (hero.hasBoon(boon))
        ImGui::Text("  has %s", toString(boon));
    }
    if (hero.getFaith().getPact())
      ImGui::Text("  entered %s", toString(*hero.getFaith().getPact()));
    if (hero.getFaith().enteredConsensus())
      ImGui::Text("  reached consensus");
  }
  else
    ImGui::Text("Hero defeated.");
}

void showStatus(const Monster& monster)
{
  if (!monster.isDefeated())
  {
    ImGui::Text("%s has %i/%i HP and does %i damage", monster.getName(), monster.getHitPoints(),
                monster.getHitPointsMax(), monster.getDamage());
    if (monster.hasFirstStrike() && !monster.isSlowed())
      ImGui::Text("  First Strike");
    if (monster.doesMagicalDamage())
      ImGui::Text("  Magical Attack");
    if (monster.doesRetaliate() && !monster.isSlowed())
      ImGui::Text("  Retaliate: Fireball");
    if (monster.getPhysicalResistPercent() > 0)
      ImGui::Text("  Physical resist %i%%", monster.getPhysicalResistPercent());
    if (monster.getMagicalResistPercent() > 0)
      ImGui::Text("  Magical resist %i%%", monster.getMagicalResistPercent());
    if (monster.isPoisonous())
      ImGui::Text("  Poisonous");
    if (monster.hasManaBurn())
      ImGui::Text("  Mana Burn");
    if (monster.bearsCurse())
      ImGui::Text("  Curse bearer");
    if (monster.isCorrosive())
      ImGui::Text("  Corrosive");
    if (monster.isWeakening())
      ImGui::Text("  Weakening Blow");
    if (monster.getDeathGazePercent() > 0)
      ImGui::Text("  Death Gaze %i%%", monster.getDeathGazePercent());
    if (monster.getDeathProtection() > 0)
      ImGui::Text("  Death protection (x%i)", monster.getDeathProtection());
    if (monster.getLifeStealPercent() > 0)
      ImGui::Text("  Life Steal (%i%%)", monster.getLifeStealPercent());
    if (monster.isBurning())
      ImGui::Text("  Burning (burn stack size %i)", monster.getBurnStackSize());
    if (monster.isPoisoned())
      ImGui::Text("  Poisoned (amount: %i)", monster.getPoisonAmount());
    if (monster.isSlowed())
      ImGui::Text("  Slowed");
    if (monster.getCorroded() > 0)
      ImGui::Text("  Corroded (x%i)", monster.getCorroded());
    if (monster.isUndead())
      ImGui::Text("  Undead");
    if (monster.isBloodless())
      ImGui::Text("  Bloodless");
    if (!monster.grantsXP())
      ImGui::Text("  No Experience");
  }
  else
    ImGui::Text("%s defeated.", monster.getName());
}

void showStatus(const State& state)
{
  if (state.hero)
    showStatus(*state.hero);
  if (state.monster)
    showStatus(*state.monster);
}
