#ifndef LOCK_H_
#define LOCK_H_

#include <powernex/powernex.h>

typedef uint32_t spinlock_t;
enum {
	SPINLOCK_UNLOCKED = 0,
	SPINLOCK_LOCKED = 1
};

void spinlock_init(spinlock_t * lock);
void spinlock_destroy(spinlock_t * lock);
void spinlock_lock(spinlock_t * lock);
void spinlock_unlock(spinlock_t * lock);

#endif
