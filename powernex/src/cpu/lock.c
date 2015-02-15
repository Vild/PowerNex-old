#include <powernex/cpu/lock.h>

static spinlock_t spinlock_atomicTrySet(volatile spinlock_t * lock) {
	register spinlock_t value = SPINLOCK_UNLOCKED;

	__asm__ volatile("lock  \
                    xchgl %0, %1"
									 : "=q" (value), "=m" (*lock)
									 : "0" (value));
	return value;
}

void spinlock_init(spinlock_t * lock) {
	*lock = SPINLOCK_UNLOCKED;
}

void spinlock_destroy(UNUSED spinlock_t * lock) {
}

void spinlock_lock(spinlock_t * lock) {
	while (spinlock_atomicTrySet(lock) == SPINLOCK_LOCKED) {
		//TODO: Yield();
	}
}

void spinlock_unlock(spinlock_t * lock) {
	*lock = SPINLOCK_UNLOCKED;
}

