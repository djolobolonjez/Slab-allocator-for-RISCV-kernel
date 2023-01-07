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
    PROVIDE(KCODE_BEGIN = .);
    EXCLUDE_FILE (*user/*o *test/*o) *(.text .text.*)
    PROVIDE(KCODE_END = .);
    . = ALIGN(0x1000);
    PROVIDE(etext = .);
  }
  
  .rodata : {
    . = ALIGN(16);
    *(.srodata .srodata.*) /* do not need to distinguish this from .rodata */
    . = ALIGN(16);
    *(.rodata .rodata.*)
  }

  .data : {
    PROVIDE(KDATA_BEGIN = .);
    . = ALIGN(16);
    EXCLUDE_FILE (*user/*o *test/*o) *(.sdata .sdata.*) /* do not need to distinguish this from .data */
    . = ALIGN(16);
    EXCLUDE_FILE (*user/*o *test/*o) *(.data .data.*)
  }

  .bss : {
    . = ALIGN(16);
    EXCLUDE_FILE (*user/*o *test/*o) *(.sbss .sbss.*) /* do not need to distinguish this from .bss */
    . = ALIGN(16);
    EXCLUDE_FILE (*user/*o *test/*o) *(.bss .bss.*)
    PROVIDE(KDATA_END = .);
    . = ALIGN(0x1000);
  }
  
  .utext : {
    PROVIDE(USER_CODE_START = .);
    *user/*(.text .text.*)
    *test/*(.text .text.*)
    PROVIDE(USER_CODE_END = .);
    . = ALIGN(0x1000);
  }
    
  .udata : {
    PROVIDE(UDATA_BEGIN = .);
    . = ALIGN(16);
    *user/*(.sdata .sdata.*)
    *test/*(.sdata .sdata.*)
    . = ALIGN(16);
    *user/*(.data .data.*)
    *test/*(.data .data.*)
  }
  
  .ubss : {
    . = ALIGN(16);
    *user/*(.sbss .sbss.*)
    *test/*(.sbss .sbss.*)
    . = ALIGN(16);
    *user/*(.bss .bss.*)
    *test/*(.bss .bss.*)
    PROVIDE(UDATA_END = .);
    . = ALIGN(0x1000);
  }

  PROVIDE(end = .);
}
