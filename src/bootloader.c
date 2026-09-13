#define UART0_DR (*((volatile unsigned int *)0x4000C000))
#define UART0_FR (*((volatile unsigned int *)0x4000C018))

void uart_putc(char c) {
    while (UART0_FR & (1 << 5));
    UART0_DR = c;
}

void uart_puts(const char *s) {
    while (*s) uart_putc(*s++);
}

char uart_getc(void) {
    while (UART0_FR & (1 << 4));
    return (char)UART0_DR;
}

void jump_to_app(unsigned int app_addr) {
    typedef void (*app_entry_t)(void);
    unsigned int *vector_table = (unsigned int *)app_addr;
    unsigned int app_stack = vector_table[0];
    unsigned int app_reset = vector_table[1];

    __asm volatile (
        "msr msp, %0\n"
        "bx %1\n"
        : : "r"(app_stack), "r"(app_reset)
    );
}

void main(void) {
    uart_puts("Bootloader v1.0\n");
    uart_puts("Waiting for app upload...\n");
    uart_puts("Press 'g' to jump to existing app, or send app bytes.\n");

    char c = uart_getc();

    if (c == 'g') {
        uart_puts("Jumping to app at 0x8000...\n");
        jump_to_app(0x8000);
    }

    uart_puts("Unknown command\n");
    while (1);
}
