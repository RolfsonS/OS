ENTRY(_start)



SECTIONS
{
	/* Start at 1 MB, often stuff existing in RAM before this point.*/
	. = 1M;
	
	_kernel_start = .;

	/* Contains the multiboot magic and instructions for loading PTEs. */
	 .multiboot.data : {
            *(.multiboot.data)
        }

       .multiboot.text : {
           *(.multiboot.text)
       }

	. += 0xC0000000;

	.text ALIGN(4K) : AT (ADDR (.text) - 0xC0000000)
	{
		*(.text)
	}


	/* Read-only data. */
	.rodata ALIGN(4K): AT (ADDR (.rodata) - 0xC0000000)
	{
		*(.rodata)
	}

	/* Read-write data (initialized) */
	.data ALIGN(4K) : AT (ADDR (.data) - 0xC0000000)
	{
		*(.data)
	}


	/* Read-write data (uninitialized)*/
	.bss ALIGN(4K) : AT (ADDR (.bss) - 0xC0000000)
	{
		*(COMMON)
		*(.bss)
		*(.stack)
	}
	_kernel_end = .;

}
