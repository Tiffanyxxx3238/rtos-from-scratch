#define UART0_DR (*((volatile unsigned int *)0x4000C000))
#define UART0_FR (*((volatile unsigned int *)0x4000C018))

void uart_putc(char c) {
    while (UART0_FR & (1 << 5));
    UART0_DR = c;
}

void uart_puts(const char *s) {
    while (*s) uart_putc(*s++);
}

void main(void) {
    uart_puts("App running! Loaded by bootloader.\n");
    while (1) {
        uart_puts("App heartbeat\n");
        for (volatile int i = 0; i < 1000000; i++);
    }
}
