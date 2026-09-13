typedef struct {
    volatile int count;
} Semaphore;

void sem_init(Semaphore *s, int initial_count) {
    s->count = initial_count;
}

void sem_wait(Semaphore *s) {
    while (1) {
        int old = s->count;
        if (old > 0) {
            if (__sync_bool_compare_and_swap(&s->count, old, old - 1)) {
                return;
            }
        }
    }
}

void sem_signal(Semaphore *s) {
    __sync_fetch_and_add(&s->count, 1);
}
