#include <stdlib.h>
#include "yd_spinlock.h"

int yd_create_spinlock(yd_spinlock_t *spinlock)
{
  *spinlock = new yd_spinlock();
  if (spinlock == NULL)
    return -1;
  return 0;
}

int yd_destroy_spinlock(yd_spinlock_t spinlock)
{
  delete (yd_spinlock*)spinlock;
  return 0;
}

int yd_spin_lock(yd_spinlock_t spinlock)
{
  return ((yd_spinlock*)spinlock)->lock();
}

int yd_spin_unlock(yd_spinlock_t spinlock)
{
  return ((yd_spinlock*)spinlock)->unlock();
}
