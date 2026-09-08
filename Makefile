CC = arm-none-eabi-gcc
CFLAGS = -mcpu=cortex-m4 -mthumb -nostdlib -nostartfiles -Isrc

all:
	$(CC) $(CFLAGS) startup.s src/switch.s src/main.c -T linker.ld -o rtos.elf

preemptive:
	$(CC) $(CFLAGS) startup.s src/switch.s src/systick.c src/main_preemptive.c -T linker.ld -o rtos_preemptive.elf

mutex:
	$(CC) $(CFLAGS) startup.s src/switch.s src/systick.c src/mutex.c src/main_mutex.c -T linker.ld -o rtos_mutex.elf

alloc:
	$(CC) $(CFLAGS) startup.s src/main_alloc.c src/allocator.c -T linker.ld -o rtos_alloc.elf

stm32:
	$(CC) -mcpu=cortex-m4 -mthumb -nostdlib -nostartfiles -Isrc startup_stm32.s src/switch.s src/systick.c src/mutex.c src/uart_stm32.c src/main_stm32.c -T linker_stm32.ld -o rtos_stm32.elf

flash:
	openocd -f interface/stlink.cfg -f target/stm32f4x.cfg -c "program rtos_stm32.elf verify reset exit"

qemu:
	qemu-system-arm -machine lm3s6965evb -kernel rtos.elf -nographic

qemu-preemptive:
	qemu-system-arm -machine lm3s6965evb -kernel rtos_preemptive.elf -nographic

qemu-mutex:
	qemu-system-arm -machine lm3s6965evb -kernel rtos_mutex.elf -nographic

qemu-alloc:
	qemu-system-arm -machine lm3s6965evb -kernel rtos_alloc.elf -nographic

clean:
	rm -f rtos.elf rtos_preemptive.elf rtos_mutex.elf rtos_alloc.elf rtos_stm32.elf
