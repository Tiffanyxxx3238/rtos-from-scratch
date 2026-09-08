typedef struct {
    volatile int locked;
} Mutex;

void mutex_init(Mutex *m) {
    m->locked = 0;
}

void mutex_lock(Mutex *m) {
    while (__sync_lock_test_and_set(&m->locked, 1)) {
        while (m->locked);
    }
}

void mutex_unlock(Mutex *m) {
    __sync_lock_release(&m->locked);
}
