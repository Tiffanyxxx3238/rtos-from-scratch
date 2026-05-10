CC = arm-none-eabi-gcc
CFLAGS = -mcpu=cortex-m3 -mthumb -nostdlib -nostartfiles

all:
	$(CC) $(CFLAGS) startup.s src/switch.s src/main.c -T linker.ld -o rtos.elf

qemu:
	qemu-system-arm -machine lm3s6965evb -kernel rtos.elf -nographic

clean:
	rm -f rtos.elf
