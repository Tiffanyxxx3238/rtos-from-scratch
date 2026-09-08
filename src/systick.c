#define SYSTICK_CTRL   (*((volatile unsigned int *)0xE000E010))
#define SYSTICK_LOAD   (*((volatile unsigned int *)0xE000E014))
#define SYSTICK_VAL    (*((volatile unsigned int *)0xE000E018))

void systick_init(unsigned int ticks) {
    SYSTICK_LOAD = ticks - 1;
    SYSTICK_VAL  = 0;
    SYSTICK_CTRL = (1 << 0) |
                   (1 << 1) |
                   (1 << 2);
}
