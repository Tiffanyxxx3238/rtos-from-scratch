#define RCC_AHB1ENR (*((volatile unsigned int *)0x40023830))
#define GPIOA_MODER (*((volatile unsigned int *)0x40020000))
#define GPIOA_ODR   (*((volatile unsigned int *)0x40020014))

extern void uart_init(void);
extern void uart_puts(const char *s);

void PendSV_Handler(void) { while(1); }
void SysTick_Handler(void) {}

void main(void) {
    RCC_AHB1ENR |= (1 << 0);
    GPIOA_MODER &= ~(3 << 10);
    GPIOA_MODER |=  (1 << 10);

    uart_init();

    while (1) {
        GPIOA_ODR ^= (1 << 5);
        uart_puts("tick\r\n");
        for (volatile int i = 0; i < 1000000; i++);
    }
}
