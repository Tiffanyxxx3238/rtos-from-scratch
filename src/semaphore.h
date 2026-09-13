typedef struct {
    volatile int count;
} Semaphore;

void sem_init(Semaphore *s, int initial_count);
void sem_wait(Semaphore *s);
void sem_signal(Semaphore *s);
