#pragma once

class MonsterStatus
{
public:
  bool isBurning() const;
  bool isPoisoned() const;
  bool isSlowed() const;
  bool isZotted() const;
  bool isWickedSick() const;

  int getBurnStackSize() const;
  int getPoisonAmount() const;
  int getCorroded() const;

  void setBurn(int nStacks);
  void setPoison(int poisonAmount);
  void setCorroded(int numCorrosionStacks);
  void setSlowed(bool slowed);
  void setZotted();
  void setWickedSick();

private:
  int burnStackSize{0};
  int poisonAmount{0};
  int corroded{0};
  bool slowed{false};
  bool zotted{false};
  bool wickedSick{false};
};
