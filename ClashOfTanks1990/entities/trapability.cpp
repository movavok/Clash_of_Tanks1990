#include "trapability.h"

TrapAbility::TrapAbility(const QPointF& pos, Tank* own)
    : Entity(pos), owner(own) {}
