#define ICSR   (*((volatile unsigned int *)0xE000ED04))
#define SHPR3  (*((volatile unsigned int *)0xE000ED20))

#include "mutex.h"

extern void uart_init(void);
extern void uart_puts(const char *s);
extern void systick_init(unsigned int ticks);

static Mutex uart_mutex;

void uart_puts_safe(const char *s) {
    mutex_lock(&uart_mutex);
    uart_puts(s);
    mutex_unlock(&uart_mutex);
}

#define STACK_SIZE 256
static unsigned int stack_a[STACK_SIZE];
static unsigned int stack_b[STACK_SIZE];

unsigned int *task_a_sp;
unsigned int *task_b_sp;
unsigned int current_task = 0;

void SysTick_Handler(void) {
    ICSR |= (1 << 28);
}

void task_a(void) {
    while (1) {
        uart_puts_safe("[Task A] running\r\n");
        for (volatile int i = 0; i < 500000; i++);
    }
}

void task_b(void) {
    while (1) {
        uart_puts_safe("[Task B] running\r\n");
        for (volatile int i = 0; i < 500000; i++);
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
    uart_init();
    mutex_init(&uart_mutex);

    uart_puts_safe("STM32 RTOS starting...\r\n");

    SHPR3 |= (0xFF << 16);

    task_a_sp = init_task_stack(stack_a, task_a);
    task_b_sp = init_task_stack(stack_b, task_b);

    systick_init(2000000);

    uart_puts_safe("Launching...\r\n");

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
