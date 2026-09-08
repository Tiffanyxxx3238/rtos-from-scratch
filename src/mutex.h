typedef struct {
    volatile int locked;
} Mutex;

void mutex_init(Mutex *m);
void mutex_lock(Mutex *m);
void mutex_unlock(Mutex *m);
