MEMORY
{
  FLASH (rx)  : ORIGIN = 0x00008000, LENGTH = 224K
  SRAM  (rwx) : ORIGIN = 0x20002000, LENGTH = 8K
}

SECTIONS
{
  .text : { *(.text*) } > FLASH
  .data : { *(.data*) } > SRAM AT > FLASH
  .bss  : { *(.bss*) } > SRAM
}
