# rtos-from-scratch

A minimal RTOS kernel built from scratch on ARM Cortex-M, verified on QEMU and physical STM32F411RE hardware.
No HAL, no libraries — pure bare-metal C and ARM assembly.

## Demo

    RTOS starting...
    Launching...
    [Task A] running
    [Task B] running
    [Task A] running
    [Task B] running

## Features

- Bare-metal startup code and linker script (no CMSIS, no vendor SDK)
- UART driver via memory-mapped I/O
- Two-task cooperative scheduler
- Preemptive scheduler via SysTick + PendSV
- Spinlock mutex protecting shared resources
- First-fit memory allocator with block coalescing
- Ported and flashed to physical STM32F411RE (Nucleo board)

## How it works

### 1. Startup code (startup.s)
On reset, ARM Cortex-M reads a vector table at address 0x00000000.
The vector table tells the CPU where the stack starts and where to jump on reset.
This project builds the vector table manually in assembly.

### 2. Linker script (linker.ld)
Defines the memory layout:
- FLASH at 0x00000000 — read-only, stores code
- SRAM at 0x20000000 — read-write, stores variables and stacks

### 3. UART driver
UART0 on the lm3s6965evb is memory-mapped:
- 0x4000C000 — Data Register
- 0x4000C018 — Flag Register (bit 5 = TX FIFO full)

uart_putc() polls the flag register and writes directly to the data register.

### 4. Task stack initialization
Each task gets its own stack.
init_task_stack() pre-fills the stack to look like the task was already interrupted,
so the context switch can restore it correctly.

Key fields:
- PC: address of the task function
- xPSR: 0x01000000 (Thumb mode)
- LR: return address

### 5. Context switch (src/switch.s)
Written entirely in ARM Thumb assembly.

Swapping the stack pointer is all it takes — once SP points to a different task's stack,
the CPU is in a completely different execution context.

### 6. Cooperative scheduler
Tasks voluntarily yield the CPU by calling yield().

### 7. Preemptive scheduler
SysTick fires every N cycles and triggers PendSV.
PendSV_Handler performs the context switch using PSP.
Tasks are switched without any cooperative yield.

### 8. Mutex
Spinlock mutex using atomic test-and-set.
Protects shared resources like UART from race conditions.

### 9. Memory allocator
First-fit allocator with block splitting and coalescing.
- mem_alloc: finds first free block, splits if oversized
- mem_free: marks free and merges adjacent free blocks

## Project structure

    rtos-from-scratch/
    startup.s              # QEMU vector table and reset handler
    startup_stm32.s        # STM32F411RE vector table
    linker.ld              # QEMU memory layout
    linker_stm32.ld        # STM32F411RE memory layout
    Makefile
    README.md
    src/
        main.c             # Cooperative scheduler
        main_preemptive.c  # Preemptive scheduler
        main_mutex.c       # Mutex demo
        main_alloc.c       # Memory allocator demo
        main_stm32.c       # STM32 port
        switch.s           # Context switch in ARM assembly
        systick.c          # SysTick driver
        mutex.c / mutex.h  # Spinlock mutex
        allocator.c / allocator.h  # Memory allocator
        uart_stm32.c       # STM32 USART2 driver

## Build and run

Requires arm-none-eabi-gcc and qemu-system-arm.

    make              # cooperative scheduler
    make qemu

    make preemptive   # preemptive scheduler
    make qemu-preemptive

    make mutex        # mutex demo
    make qemu-mutex

    make alloc        # memory allocator demo
    make qemu-alloc

    make stm32        # STM32F411RE build
    make flash        # flash via OpenOCD + ST-Link

## Environment

- Toolchain: arm-none-eabi-gcc 13.2
- QEMU: 8.2.2, machine lm3s6965evb (Cortex-M3)
- Physical: STM32F411RE Nucleo (Cortex-M4), flashed via OpenOCD
- Host: Ubuntu 24.04 on WSL2

## Roadmap

- [x] Bare-metal startup and linker script
- [x] UART driver (memory-mapped, no HAL)
- [x] Task stack initialization
- [x] Context switch in ARM assembly
- [x] Two-task cooperative scheduler on QEMU
- [x] Preemptive scheduling via SysTick + PendSV
- [x] Mutex protecting shared resources
- [x] First-fit memory allocator with coalescing
- [x] Port to STM32F411RE — compiled, flashed, LED blink verified on real hardware
- [ ] UART output on physical STM32
- [ ] Semaphore
- [ ] Priority-based scheduling
