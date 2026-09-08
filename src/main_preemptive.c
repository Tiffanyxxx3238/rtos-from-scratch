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
static unsigned int stack_a[STACK_SIZE];
static unsigned int stack_b[STACK_SIZE];

unsigned int *task_a_sp;
unsigned int *task_b_sp;
unsigned int current_task = 0;

extern void systick_init(unsigned int ticks);

void SysTick_Handler(void) {
    ICSR |= (1 << 28);
}

void task_a(void) {
    while (1) {
        uart_puts("[Task A] running\n");
        for (volatile int i = 0; i < 200000; i++);
    }
}

void task_b(void) {
    while (1) {
        uart_puts("[Task B] running\n");
        for (volatile int i = 0; i < 200000; i++);
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

void main(void) {
    uart_puts("Preemptive RTOS starting...\n");

    SHPR3 |= (0xFF << 16);

    task_a_sp = init_task_stack(stack_a, task_a);
    task_b_sp = init_task_stack(stack_b, task_b);

    systick_init(1000000);

    uart_puts("Launching...\n");

    __asm volatile (
        "ldr r0, =task_a_sp\n"
        "ldr r0, [r0]\n"
        "add r0, r0, #32\n"
        "msr psp, r0\n"
        "mov r0, #3\n"
        "msr control, r0\n"
        "isb\n"
    );

    task_a();
}
