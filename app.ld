OUTPUT_ARCH("i386:x86-64")
ENTRY(start)
SECTIONS
{
	. = 0xffff800000000000;
	.text : { *(.text) }
	.data : { *(.data .rodata) QUAD(ADDR(.bss)+SIZEOF(.bss)) }
	.bss : {
		__bss_start = .;
		*(.bss)
		end = .; _end = .; __end = .;
	}
}
