#include "ItemClass.hpp"

class Inventory
{
public:
  virtual ~Inventory() = default;

  virtual bool canAddSmallItem() = 0;
  virtual bool canAddSmallItems(int n) = 0;
  virtual bool canAddLargeItem() = 0;
  virtual bool canAddLargeItems(int n) = 0;
  virtual bool canAddSpell() = 0;
  virtual bool canAddSpells(int n) = 0;

  virtual bool has(ItemClass itemClass) = 0;

  virtual void lose(ItemClass itemClass) = 0;
};
