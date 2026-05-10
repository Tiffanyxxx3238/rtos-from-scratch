.syntax unified
.cpu cortex-m3
.thumb

.global context_switch
.thumb_func
context_switch:
    push {r4-r11, lr}
    str  sp, [r0]
    ldr  sp, [r1]
    pop  {r4-r11, lr}
    bx   lr