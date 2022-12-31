OUTPUT_ARCH( "riscv" )
ENTRY( _entry )

SECTIONS
{
  /*
   * ensure that entry.S / _entry is at 0x80000000,
   * where qemu's -kernel jumps.
   */
  . = 0x80000000;

  .text : {
    *(.entry_os) . = ALIGN(0x1000);
    *(EXCLUDE_FILE(*user/*o *test/*o) .text .text.*)
    . = ALIGN(0x1000);
    PROVIDE(etext = .);
  }
  
  .u_text : {
    . = ALIGN(0x1000);
    PROVIDE(USER_CODE_START = .);
    *user/*(.text .text.*)
    *test/*(.text .text.*)
    PROVIDE(USER_CODE_END = .);
    . = ALIGN(0x1000);
  }

  .rodata : {
    . = ALIGN(16);
    *(.srodata .srodata.*) /* do not need to distinguish this from .rodata */
    . = ALIGN(16);
    *(.rodata .rodata.*)
  }

  .data : {
    . = ALIGN(16);
    *(.sdata .sdata.*) /* do not need to distinguish this from .data */
    . = ALIGN(16);
    *(.data .data.*)
  }

  .bss : {
    . = ALIGN(16);
    *(.sbss .sbss.*) /* do not need to distinguish this from .bss */
    . = ALIGN(16);
    *(.bss .bss.*)
  }

  PROVIDE(end = .);
}
