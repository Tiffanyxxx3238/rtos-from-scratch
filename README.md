# rtos-from-scratch

A minimal preemptive RTOS kernel built entirely from scratch in bare-metal C and ARM assembly.
No HAL, no CMSIS, no vendor SDK. Verified on QEMU and on physical STM32F411RE hardware.

## Demo (on real STM32F411RE hardware)

    STM32 RTOS starting...
    Launching...
    [Task A] running
    [Task B] running
    [Task A] running
    [Task B] running
    ...

## What this project demonstrates

- Hand-written ARM Cortex-M vector table and startup code
- A from-scratch context switch in ARM Thumb assembly
- Cooperative and preemptive schedulers (SysTick + PendSV)
- A spinlock mutex and a first-fit memory allocator with coalescing
- A full port from QEMU to physical STM32F411RE hardware, including
  finding and fixing a genuinely subtle stack-corruption bug using GDB
  register-level tracing on real silicon

## Architecture

### 1. Startup code and vector table
On reset, ARM Cortex-M reads a vector table at address 0x00000000 (or
0x08000000 on STM32, where FLASH is mapped). The vector table tells the
CPU where the initial stack pointer is and where to jump on reset. Both
are written by hand in assembly (startup.s for QEMU, startup_stm32.s
for the physical board), including the PendSV and SysTick vector slots
needed by the scheduler.

### 2. Linker script
Defines the memory layout for each target:
- QEMU (lm3s6965evb): FLASH at 0x00000000, SRAM at 0x20000000
- STM32F411RE: FLASH at 0x08000000 (512K), SRAM at 0x20000000 (128K)

### 3. UART driver
Entirely memory-mapped I/O, no HAL. On QEMU this is UART0; on the
STM32 it is USART2 routed through PA2, configured directly via the
GPIO MODER/AFRL and USART CR1/BRR registers.

### 4. Task stack initialization
Each task owns a fixed-size stack. init_task_stack() pre-fills that
stack so it looks exactly like a task that was already interrupted
once, which lets the context switch restore it correctly the first
time it runs. The frame has two parts:
- The hardware exception frame (xPSR, PC, LR, R12, R3, R2, R1, R0),
  which the CPU pushes and pops automatically on entry/exit from an
  exception
- The software frame (R4-R11), which must be saved and restored by
  hand because the hardware does not touch these registers

### 5. Context switch (src/switch.s)
Written entirely in ARM Thumb assembly. The cooperative version simply
swaps the stack pointer:

    context_switch:
        push {r4-r11, lr}
        str  sp, [r0]
        ldr  sp, [r1]
        pop  {r4-r11, lr}
        bx   lr

The preemptive version does the same job inside PendSV_Handler, using
the Process Stack Pointer (PSP) instead of the Main Stack Pointer,
since PendSV runs as an exception on top of whichever task was
executing.

### 6. Schedulers
- Cooperative: tasks call yield() to hand off the CPU voluntarily.
- Preemptive: SysTick fires on a timer and sets the PendSV pending bit
  in ICSR. PendSV, given the lowest interrupt priority, then performs
  the actual context switch. This is the same mechanism FreeRTOS uses.

### 7. Mutex
A spinlock built on __sync_lock_test_and_set, used to protect UART
output from interleaving when two tasks print at the same time.

### 8. Memory allocator
A first-fit allocator with block splitting and coalescing:
- mem_alloc finds the first free block large enough, splitting it if
  there is excess space
- mem_free marks a block free and merges it with adjacent free blocks

### 9. Physical hardware port (STM32F411RE)
The QEMU version was ported to a real Nucleo-F411RE board: a new
linker script and startup file for the STM32 memory map, USART2
configured for the board's actual UART-to-USB bridge, and the same
context switch and scheduler logic, flashed via OpenOCD and an
on-board ST-Link.

## The hardware debugging story

Getting the RTOS running in QEMU took one afternoon. Getting the exact
same logic running correctly on physical hardware took much longer,
for two separate reasons worth recording.

**Environment problem.** The board was flashed and debugged from WSL2,
which requires forwarding the USB device from Windows using usbipd.
This forwarding was unreliable: OpenOCD would intermittently lose the
device mid-session, and Windows and WSL would fight over which one
owned the ST-Link's virtual COM port, so the port that OpenOCD needed
for flashing and the port needed for reading UART output could not
reliably coexist. The fix was to stop using WSL for this stage
entirely and install the ARM toolchain and OpenOCD natively on
Windows, talking to the board directly with no USB passthrough layer
in between.

**Logic bug.** Once the toolchain issue was resolved, the board would
run Task A once, then silently reset. GDB, attached live over the
ST-Link's SWD interface, showed the CPU hitting a double fault after
the first PendSV-triggered context switch. Stepping through
PendSV_Handler instruction by instruction and inspecting registers at
each step showed that task_b_sp, a variable that should have held a
valid pointer into stack_b, instead held small garbage values, and
that stack_a and stack_b bled into each other in memory.

The root cause was an off-by-one in init_task_stack(): building the
initial stack frame requires exactly 16 decrements of the stack
pointer, 8 for the hardware exception frame and 8 for the R4-R11
software frame, but the function only performed 15. The context
switch code in switch.s always pops exactly 8 registers for R4-R11
regardless of how the stack was built, so the missing decrement
silently shifted every subsequent read by one word, corrupting the
exception frame on restore and, eventually, the neighbouring
task_a_sp / task_b_sp / current_task globals. It only broke on the
second context switch because the first one happened to still be
reading valid memory by coincidence of layout.

This kind of bug is a good illustration of why RTOS context switch
code needs to be verified against the actual register-level frame
layout, not just against "it ran once and printed something."

## Project structure

    rtos-from-scratch/
    startup.s              QEMU vector table and reset handler
    startup_stm32.s        STM32F411RE vector table
    linker.ld              QEMU memory layout
    linker_stm32.ld        STM32F411RE memory layout
    Makefile
    README.md
    src/
        main.c              Cooperative scheduler (QEMU)
        main_preemptive.c   Preemptive scheduler (QEMU)
        main_mutex.c        Mutex demo (QEMU)
        main_alloc.c        Memory allocator demo (QEMU)
        main_stm32.c        Full preemptive RTOS on physical hardware
        switch.s            Context switch in ARM assembly
        systick.c           SysTick driver
        mutex.c / mutex.h   Spinlock mutex
        allocator.c / allocator.h  Memory allocator

## Build and run

QEMU (requires arm-none-eabi-gcc and qemu-system-arm):

    make               # cooperative scheduler
    make qemu

    make preemptive    # preemptive scheduler
    make qemu-preemptive

    make mutex         # mutex demo
    make qemu-mutex

    make alloc         # memory allocator demo
    make qemu-alloc

Physical STM32F411RE (Nucleo board, requires OpenOCD and an ST-Link,
built and flashed from a native Windows toolchain to avoid WSL USB
passthrough issues):

    arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -nostdlib -nostartfiles -Isrc \
        startup_stm32.s src/switch.s src/systick.c src/main_stm32.c \
        -T linker_stm32.ld -o rtos_stm32_full.elf

    openocd -f interface/stlink.cfg -f target/stm32f4x.cfg \
        -c "program rtos_stm32_full.elf verify reset exit"

## Environment

- Toolchain: arm-none-eabi-gcc 13.2 (WSL) / 15.3 (Windows native)
- QEMU: 8.2.2, machine lm3s6965evb (Cortex-M3)
- Physical board: STM32F411RE Nucleo (Cortex-M4), ST-Link V2.1,
  flashed and debugged via OpenOCD
- Development: Ubuntu 24.04 on WSL2 for QEMU work; native Windows
  toolchain for hardware flashing and debugging

## Roadmap

- [x] Bare-metal startup and linker script
- [x] UART driver (memory-mapped, no HAL)
- [x] Task stack initialization
- [x] Context switch in ARM assembly
- [x] Two-task cooperative scheduler on QEMU
- [x] Preemptive scheduling via SysTick + PendSV
- [x] Mutex protecting shared resources
- [x] First-fit memory allocator with coalescing
- [x] Port to STM32F411RE physical hardware
- [x] Diagnosed and fixed a stack-frame corruption bug via GDB on real hardware
- [x] Semaphore (counting, atomic compare-and-swap)
- [x] Priority-based scheduling (demonstrates task starvation)
- [ ] Simple bootloader over UART
