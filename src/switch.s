.syntax unified
.cpu cortex-m4
.thumb

.global context_switch
.thumb_func
context_switch:
    push {r4-r11, lr}
    str  sp, [r0]
    ldr  sp, [r1]
    pop  {r4-r11, lr}
    bx   lr

.global PendSV_Handler
.thumb_func
PendSV_Handler:
    mrs  r0, psp
    stmdb r0!, {r4-r11}

    ldr  r1, =current_task
    ldr  r2, [r1]

    ldr  r3, =task_a_sp
    ldr  r4, =task_b_sp

    cmp  r2, #0
    beq  switch_to_b

switch_to_a:
    str  r0, [r4]
    mov  r2, #0
    str  r2, [r1]
    ldr  r0, [r3]
    b    restore

switch_to_b:
    str  r0, [r3]
    mov  r2, #1
    str  r2, [r1]
    ldr  r0, [r4]

restore:
    ldmia r0!, {r4-r11}
    msr  psp, r0
    ldr  lr, =0xFFFFFFFD
    bx   lr
