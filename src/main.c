#define UART0_DR (*((volatile unsigned int *)0x4000C000))
#define UART0_FR (*((volatile unsigned int *)0x4000C018))

void uart_putc(char c) {
    while (UART0_FR & (1 << 5));
    UART0_DR = c;
}

void uart_puts(const char *s) {
    while (*s) uart_putc(*s++);
}

#define STACK_SIZE 256
static unsigned int stack_a[STACK_SIZE];
static unsigned int stack_b[STACK_SIZE];

unsigned int *task_a_sp;
unsigned int *task_b_sp;
unsigned int current_task = 0;

extern void context_switch(unsigned int **old_sp, unsigned int **new_sp);

void yield(void) {
    if (current_task == 0) {
        current_task = 1;
        context_switch(&task_a_sp, &task_b_sp);
    } else {
        current_task = 0;
        context_switch(&task_b_sp, &task_a_sp);
    }
}

void task_a(void) {
    while (1) {
        uart_puts("[Task A] running\n");
        for (volatile int i = 0; i < 500000; i++);
        yield();
    }
}

void task_b(void) {
    while (1) {
        uart_puts("[Task B] running\n");
        for (volatile int i = 0; i < 500000; i++);
        yield();
    }
}

unsigned int *init_task_stack(unsigned int *stack_top, void (*task_func)(void)) {
    stack_top += STACK_SIZE;
    stack_top--; *stack_top = (unsigned int)task_func; // lr (返回位址)
    stack_top--; *stack_top = 0;  // R11
    stack_top--; *stack_top = 0;  // R10
    stack_top--; *stack_top = 0;  // R9
    stack_top--; *stack_top = 0;  // R8
    stack_top--; *stack_top = 0;  // R7
    stack_top--; *stack_top = 0;  // R6
    stack_top--; *stack_top = 0;  // R5
    stack_top--; *stack_top = 0;  // R4
    return stack_top;
}

static unsigned int dummy_sp;
static unsigned int *dummy_sp_ptr = &dummy_sp;

void main(void) {
    uart_puts("RTOS starting...\n");

    task_a_sp = init_task_stack(stack_a, task_a);
    task_b_sp = init_task_stack(stack_b, task_b);

    uart_puts("Launching...\n");

    current_task = 0;
    context_switch(&dummy_sp_ptr, &task_a_sp);

    while (1);
}