#define RCC_BASE    0x40023800
#define GPIOA_BASE  0x40020000
#define USART2_BASE 0x40004400

#define RCC_AHB1ENR  (*((volatile unsigned int *)(RCC_BASE   + 0x30)))
#define RCC_APB1ENR  (*((volatile unsigned int *)(RCC_BASE   + 0x40)))
#define GPIOA_MODER  (*((volatile unsigned int *)(GPIOA_BASE + 0x00)))
#define GPIOA_AFRL   (*((volatile unsigned int *)(GPIOA_BASE + 0x20)))
#define USART2_SR    (*((volatile unsigned int *)(USART2_BASE + 0x00)))
#define USART2_DR    (*((volatile unsigned int *)(USART2_BASE + 0x04)))
#define USART2_BRR   (*((volatile unsigned int *)(USART2_BASE + 0x08)))
#define USART2_CR1   (*((volatile unsigned int *)(USART2_BASE + 0x0C)))

void uart_init(void) {
    RCC_AHB1ENR |= (1 << 0);
    RCC_APB1ENR |= (1 << 17);

    GPIOA_MODER &= ~(3 << 4);
    GPIOA_MODER |=  (2 << 4);

    GPIOA_AFRL &= ~(0xF << 8);
    GPIOA_AFRL |=  (7   << 8);

    USART2_BRR = 0x008B;
    USART2_CR1 = (1 << 3) | (1 << 13);
}

void uart_putc(char c) {
    while (!(USART2_SR & (1 << 7)));
    USART2_DR = c;
}

void uart_puts(const char *s) {
    while (*s) uart_putc(*s++);
}
