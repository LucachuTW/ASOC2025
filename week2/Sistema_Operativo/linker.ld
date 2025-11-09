ENTRY(_start)
SECTIONS {
	. = 0x8000;
	
	.text : {
		*(.text*)
	} :text
	
	.rodata : {
		*(.rodata*)
	} :rodata
	
	.data : {
		*(.data*)
	} :data
	
	.bss : {
		*(.bss*)
		*(COMMON)
	} :bss
}

PHDRS {
	text PT_LOAD FLAGS(5);    /* R-X: Read + Execute */
	rodata PT_LOAD FLAGS(4);  /* R--: Read only */
	data PT_LOAD FLAGS(6);    /* RW-: Read + Write */
	bss PT_LOAD FLAGS(6);     /* RW-: Read + Write */
}