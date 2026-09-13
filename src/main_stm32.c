#define RCC_APB1ENR (*((volatile unsigned int *)0x40023840))
#define RCC_AHB1ENR (*((volatile unsigned int *)0x40023830))
#define GPIOA_MODER (*((volatile unsigned int *)0x40020000))
#define GPIOA_AFRL  (*((volatile unsigned int *)0x40020020))
#define USART2_BRR  (*((volatile unsigned int *)0x40004408))
#define USART2_CR1  (*((volatile unsigned int *)0x4000440C))
#define USART2_SR   (*((volatile unsigned int *)0x40004400))
#define USART2_DR   (*((volatile unsigned int *)0x40004404))
#define ICSR   (*((volatile unsigned int *)0xE000ED04))
#define SHPR3  (*((volatile unsigned int *)0xE000ED20))

void uart_putc(char c) {
    while (!(USART2_SR & (1 << 7)));
    USART2_DR = c;
}

void uart_puts(const char *s) {
    while (*s) uart_putc(*s++);
}

extern void systick_init(unsigned int ticks);

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
        uart_puts("[Task A] running\r\n");
        for (volatile int i = 0; i < 500000; i++);
    }
}

void task_b(void) {
    while (1) {
        uart_puts("[Task B] running\r\n");
        for (volatile int i = 0; i < 500000; i++);
    }
}

unsigned int *init_task_stack(unsigned int *stack_top, void (*task_func)(void)) {
    stack_top += STACK_SIZE;
    stack_top--; *stack_top = 0x01000000;              /* xPSR */
    stack_top--; *stack_top = (unsigned int)task_func; /* PC */
    stack_top--; *stack_top = (unsigned int)task_func; /* LR */
    stack_top--; *stack_top = 0;  /* R12 */
    stack_top--; *stack_top = 0;  /* R3 */
    stack_top--; *stack_top = 0;  /* R2 */
    stack_top--; *stack_top = 0;  /* R1 */
    stack_top--; *stack_top = 0;  /* R0 */
    stack_top--; *stack_top = 0;  /* R11 */
    stack_top--; *stack_top = 0;  /* R10 */
    stack_top--; *stack_top = 0;  /* R9 */
    stack_top--; *stack_top = 0;  /* R8 */
    stack_top--; *stack_top = 0;  /* R7 */
    stack_top--; *stack_top = 0;  /* R6 */
    stack_top--; *stack_top = 0;  /* R5 */
    stack_top--; *stack_top = 0;  /* R4 */
    return stack_top;
}

void main(void) {
    RCC_AHB1ENR |= (1 << 0);
    RCC_APB1ENR |= (1 << 17);

    GPIOA_MODER &= ~(3 << 4);
    GPIOA_MODER |=  (2 << 4);

    GPIOA_AFRL &= ~(0xF << 8);
    GPIOA_AFRL |=  (7   << 8);

    USART2_BRR = 0x008B;
    USART2_CR1 = (1 << 3) | (1 << 13);

    uart_puts("STM32 RTOS starting...\r\n");

    SHPR3 |= (0xFF << 16);

    task_a_sp = init_task_stack(stack_a, task_a);
    task_b_sp = init_task_stack(stack_b, task_b);

    systick_init(2000000);

    uart_puts("Launching...\r\n");

    __asm volatile (
        "ldr r0, =task_a_sp\n"
        "ldr r0, [r0]\n"
        "add r0, r0, #64\n"
        "msr psp, r0\n"
        "mov r0, #3\n"
        "msr control, r0\n"
        "isb\n"
    );

    task_a();
}
