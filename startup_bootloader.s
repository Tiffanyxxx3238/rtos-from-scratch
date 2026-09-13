.syntax unified
.cpu cortex-m3
.thumb

.word 0x20002000
.word Reset_Handler

.thumb_func
Reset_Handler:
    bl main
    b .
