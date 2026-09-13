#define UART0_DR (*((volatile unsigned int *)0x4000C000))
#define UART0_FR (*((volatile unsigned int *)0x4000C018))
#define ICSR   (*((volatile unsigned int *)0xE000ED04))
#define SHPR3  (*((volatile unsigned int *)0xE000ED20))

void uart_putc(char c) {
    while (UART0_FR & (1 << 5));
    UART0_DR = c;
}

void uart_puts(const char *s) {
    while (*s) uart_putc(*s++);
}

#define STACK_SIZE 256
#define NUM_TASKS 3

static unsigned int stack_high[STACK_SIZE];
static unsigned int stack_med[STACK_SIZE];
static unsigned int stack_low[STACK_SIZE];

unsigned int *task_sp[NUM_TASKS];
unsigned int task_priority[NUM_TASKS];
unsigned int task_ready[NUM_TASKS];
unsigned int current_task = 0;

extern void systick_init(unsigned int ticks);
extern void context_switch(unsigned int **old_sp, unsigned int **new_sp);

unsigned int pick_next_task(void) {
    int best = -1;
    unsigned int best_prio = 0xFFFFFFFF;
    for (int i = 0; i < NUM_TASKS; i++) {
        if (task_ready[i] && task_priority[i] < best_prio) {
            best_prio = task_priority[i];
            best = i;
        }
    }
    return (best >= 0) ? (unsigned int)best : current_task;
}

void task_high(void) {
    while (1) {
        uart_puts("[HIGH] running\n");
        for (volatile int i = 0; i < 300000; i++);
    }
}

void task_med(void) {
    while (1) {
        uart_puts("[MED] running\n");
        for (volatile int i = 0; i < 300000; i++);
    }
}

void task_low(void) {
    while (1) {
        uart_puts("[LOW] running\n");
        for (volatile int i = 0; i < 300000; i++);
    }
}

unsigned int *init_task_stack(unsigned int *stack_top, void (*task_func)(void)) {
    stack_top += STACK_SIZE;
    stack_top--; *stack_top = 0x01000000;
    stack_top--; *stack_top = (unsigned int)task_func;
    stack_top--; *stack_top = (unsigned int)task_func;
    stack_top--; *stack_top = 0;
    stack_top--; *stack_top = 0;
    stack_top--; *stack_top = 0;
    stack_top--; *stack_top = 0;
    stack_top--; *stack_top = 0;
    stack_top--; *stack_top = 0;
    stack_top--; *stack_top = 0;
    stack_top--; *stack_top = 0;
    stack_top--; *stack_top = 0;
    stack_top--; *stack_top = 0;
    stack_top--; *stack_top = 0;
    stack_top--; *stack_top = 0;
    stack_top--; *stack_top = 0;
    return stack_top;
}

void SysTick_Handler(void) {
    ICSR |= (1 << 28);
}

void PendSV_Handler(void) {
    unsigned int next = pick_next_task();
    if (next != current_task) {
        unsigned int prev = current_task;
        current_task = next;
        context_switch(&task_sp[prev], &task_sp[next]);
    }
}

void main(void) {
    uart_puts("Priority scheduler starting...\n");

    SHPR3 |= (0xFF << 16);

    task_sp[0] = init_task_stack(stack_high, task_high);
    task_sp[1] = init_task_stack(stack_med, task_med);
    task_sp[2] = init_task_stack(stack_low, task_low);

    task_priority[0] = 0;
    task_priority[1] = 1;
    task_priority[2] = 2;

    task_ready[0] = 1;
    task_ready[1] = 1;
    task_ready[2] = 1;

    current_task = 0;

    systick_init(1000000);

    uart_puts("Launching HIGH priority task...\n");

    __asm volatile (
        "ldr r0, =task_sp\n"
        "ldr r0, [r0]\n"
        "add r0, r0, #64\n"
        "msr psp, r0\n"
        "mov r0, #3\n"
        "msr control, r0\n"
        "isb\n"
    );

    task_high();
}
