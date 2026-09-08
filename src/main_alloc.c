#define UART0_DR (*((volatile unsigned int *)0x4000C000))
#define UART0_FR (*((volatile unsigned int *)0x4000C018))

#include "allocator.h"

void uart_putc(char c) {
    while (UART0_FR & (1 << 5));
    UART0_DR = c;
}

void uart_puts(const char *s) {
    while (*s) uart_putc(*s++);
}

void uart_puthex(unsigned int n) {
    uart_puts("0x");
    for (int i = 28; i >= 0; i -= 4) {
        int d = (n >> i) & 0xF;
        uart_putc(d < 10 ? '0' + d : 'a' + d - 10);
    }
    uart_puts("\n");
}

void main(void) {
    uart_puts("Memory allocator test\n");

    mem_init();

    void *a = mem_alloc(64);
    uart_puts("alloc 64 bytes at: ");
    uart_puthex((unsigned int)a);

    void *b = mem_alloc(128);
    uart_puts("alloc 128 bytes at: ");
    uart_puthex((unsigned int)b);

    mem_free(a);
    uart_puts("freed first block\n");

    void *c = mem_alloc(32);
    uart_puts("alloc 32 bytes at: ");
    uart_puthex((unsigned int)c);

    mem_free(b);
    mem_free(c);
    uart_puts("freed all blocks\n");

    void *d = mem_alloc(200);
    uart_puts("alloc 200 bytes after merge at: ");
    uart_puthex((unsigned int)d);

    uart_puts("done\n");
    while (1);
}

void PendSV_Handler(void) { while(1); }
void SysTick_Handler(void) {}
