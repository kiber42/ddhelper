#pragma once

class MonsterStatus
{
public:
  MonsterStatus();

  bool isBurning() const;
  bool isPoisoned() const;
  bool isSlowed() const;

  int getBurnStackSize() const;
  int getPoisonAmount() const;
  int getCorroded() const;
  int getWeakened() const;

  void setBurn(int nStacks);
  void setPoison(int poisonAmount);
  void setSlowed(bool slowed);
  void setCorroded(int numCorrosionStacks);
  void setWeakened(int numWeakened);

private:
  int burnStackSize;
  int poisonAmount;
  bool slowed;
  int corroded;
  int weakened;
};
