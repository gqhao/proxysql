typedef void * yd_spinlock_t;

class yd_spinlock
{
public:
  yd_spinlock(){lock_ = 0;}
  ~yd_spinlock(){}

public:
  inline int lock()
  {
    while (__sync_lock_test_and_set(&lock_, 1)){}
    return 0;
  }

  inline int unlock()
  {
    __sync_lock_release(&lock_);
    return 0;
  }

private:
  int lock_;
};


int yd_create_spinlock(yd_spinlock_t *spinlock);
int yd_destroy_spinlock(yd_spinlock_t spinlock);
int yd_spin_lock(yd_spinlock_t spinlock);
int yd_spin_unlock(yd_spinlock_t spinlock);
