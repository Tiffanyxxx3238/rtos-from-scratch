# rtos-from-scratch

A minimal RTOS kernel built from scratch on ARM Cortex-M3, verified on QEMU.  
No HAL, no libraries — pure bare-metal C and ARM assembly.

## What this project does

Two tasks run concurrently on a simulated Cortex-M3 CPU, switching between each other via a hand-written context switch routine in ARM assembly.

RTOS starting...
Launching...
[Task A] running
[Task B] running
[Task A] running
[Task B] running
...

## How it works

### 1. Startup code (`startup.s`)
On reset, ARM Cortex-M reads a **vector table** at address `0x00000000`.  
The vector table tells the CPU where the stack starts and where to jump on reset.  
This project builds the vector table manually in assembly — no CMSIS, no vendor SDK.

### 2. Linker script (`linker.ld`)
Defines the memory layout of the target:
- `FLASH` at `0x00000000` — read-only, stores code
- `SRAM` at `0x20000000` — read-write, stores variables and stacks

### 3. UART driver (bare-metal)
UART0 on the lm3s6965evb is memory-mapped:
- `0x4000C000` — Data Register (write a byte to transmit)
- `0x4000C018` — Flag Register (bit 5 = TX FIFO full)

`uart_putc()` polls the flag register and writes directly to the data register.  
No interrupts, no DMA, no HAL — just raw memory-mapped I/O.

### 4. Task stack initialization
Each task gets its own stack (256 words).  
`init_task_stack()` pre-fills the stack to look like the task was already interrupted once, so the context switch can restore it correctly.

Key fields placed on the stack:
- **PC** → address of the task function (where to jump on restore)
- **xPSR** → `0x01000000` (Thumb mode flag, required on Cortex-M)
- **LR** → return address
- **R4–R11** → callee-saved registers (initialized to 0)

### 5. Context switch (`src/switch.s`)
The heart of the RTOS. Written entirely in ARM Thumb assembly.

```asm
context_switch:
    push {r4-r11, lr}   @ save current task's callee-saved registers
    str  sp, [r0]        @ save current stack pointer
    ldr  sp, [r1]        @ load next task's stack pointer
    pop  {r4-r11, lr}   @ restore next task's registers
    bx   lr              @ jump into next task
```

Swapping the stack pointer is all it takes — once SP points to a different task's stack, the CPU is in a completely different execution context.

### 6. Cooperative scheduler
Tasks voluntarily yield the CPU by calling `yield()`.  
The scheduler alternates between Task A and Task B using `context_switch()`.

This is **cooperative multitasking** — tasks decide when to switch.  
The next milestone is **preemptive scheduling** using SysTick timer interrupts.

## Project structure

rtos-from-scratch/
├── startup.s       # ARM vector table and reset handler
├── linker.ld       # Memory layout (FLASH / SRAM)
├── Makefile        # Build and QEMU launch targets
└── src/
├── main.c      # Tasks, stack init, scheduler
└── switch.s    # Context switch in ARM assembly

## Build and run

Requires `arm-none-eabi-gcc` and `qemu-system-arm`.

```bash
# Build
make

# Run on QEMU
make qemu

# Exit QEMU
Ctrl+A then X
```

## Environment

- Toolchain: `arm-none-eabi-gcc 13.2`
- QEMU: `8.2.2`, machine `lm3s6965evb` (Cortex-M3)
- Host: Ubuntu 24.04 on WSL2

## Roadmap

- [x] Bare-metal startup and linker script
- [x] UART driver (memory-mapped, no HAL)
- [x] Task stack initialization
- [x] Context switch in ARM assembly
- [x] Two-task cooperative scheduler on QEMU
- [ ] Preemptive scheduling via SysTick interrupt
- [ ] Port to physical STM32F411RE (Nucleo board)
- [ ] Mutex and semaphore
- [ ] Simple memory allocator