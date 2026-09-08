CC = arm-none-eabi-gcc
CFLAGS = -mcpu=cortex-m3 -mthumb -nostdlib -nostartfiles -Isrc

all:
	$(CC) $(CFLAGS) startup.s src/switch.s src/main.c -T linker.ld -o rtos.elf

preemptive:
	$(CC) $(CFLAGS) startup.s src/switch.s src/systick.c src/main_preemptive.c -T linker.ld -o rtos_preemptive.elf

mutex:
	$(CC) $(CFLAGS) startup.s src/switch.s src/systick.c src/mutex.c src/main_mutex.c -T linker.ld -o rtos_mutex.elf

qemu:
	qemu-system-arm -machine lm3s6965evb -kernel rtos.elf -nographic

qemu-preemptive:
	qemu-system-arm -machine lm3s6965evb -kernel rtos_preemptive.elf -nographic

qemu-mutex:
	qemu-system-arm -machine lm3s6965evb -kernel rtos_mutex.elf -nographic

clean:
	rm -f rtos.elf rtos_preemptive.elf rtos_mutex.elf
