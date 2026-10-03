#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <limits.h>
#include "terminal.h"

void print_signed_decimal(int32_t);
void print_unsigned_decimal(uint32_t);
void print_byte_bin(uint8_t);
void print_hex(uint32_t);
char fetch_entry(uint8_t);

void kprintf(const char* format, ...) {
	/* char* used for traversing format */
	const char* traverse;
	
	/* Initialize arguments */
	va_list arg;
	va_start(arg, format);
	
	/* Iterate through all the characters in format.
	 * If we see a normal character, just print it.
	 * If we see %, use cases to determine what and how to print the next character.
	 */
	for (traverse = format; *traverse != '\0'; traverse++) {
		
		if(*traverse == '%' ) {
			traverse++;

			switch(*traverse) {
				/* Byte in binary case (My special extention) */
				case 'b':
					uint8_t b = (uint8_t) va_arg(arg, int);
					print_byte_bin(b);
					break;
				/* Char case */
				case 'c': 
					uint8_t c = (uint8_t) va_arg(arg, int);
					kputchar(c);
					break;
				/* Signed decimal case */
				case 'd':
					int32_t s = (int32_t) va_arg(arg, int);
					print_signed_decimal(s);
					break;
				/* Hex case */
				case 'x':
					uint32_t x = (uint32_t) va_arg(arg, unsigned int);
					print_hex(x);
					break;
				/* Unsigned decimal case */	
				case 'u': 
					uint32_t u = (uint32_t) va_arg(arg, unsigned int);
					print_unsigned_decimal(u);
					break;
				default:
					break;
			}

		} else if (*traverse == '\n') {
			print_newline();
		} else {
			kputchar(*traverse);
		}
	}

	va_end(arg);
}

void print_signed_decimal(int32_t num) {
	if(num == 0) {
		kputchar('0');
	}

	if(num == INT_MIN) {
		kputs("-2147483648");
	}

	if(num < 0) {
		kputchar('-');
		num = -num;
	}

	char buf[11];
	char* loc = buf + 10;
	*loc = '\0';

	while(num != 0) {
		loc--;
		*loc = fetch_entry(num % 10);
		num /= 10;
	}

	kputs(loc);
}

void print_unsigned_decimal(uint32_t num) {
	if(num == 0) {
		kputchar('0');
	}

	char buf[11];
	char* loc = buf + 10;
	*loc = '\0';

	while(num != 0) {
		loc--;
		*loc = fetch_entry(num % 10);
		num /= 10;
	}

	kputs(loc);

}

void print_byte_bin(uint8_t byte){
	char buf[9];
	buf[8] = '\0';
	
	uint8_t div = 128;

	for(size_t i = 0; i <= 7; i++) {
		if(byte	>= div) {
			buf[i] = '1';
			byte -= div;
		} else {
			buf[i] = '0';
		}
		div /= 2;
	}
	kputs(buf);
}

void print_hex(uint32_t hex) {
	if(hex == 0) {
		kputchar('0');
	}
	
	char buf[11];
	char* loc = buf + 10;
	*loc = '\0';

	while(hex != 0) {
		--loc;
		*loc = fetch_entry(hex % 16);
		hex /= 16;
	}

	*--loc = 'x';
	*--loc = '0';
	kputs(loc);
}

char fetch_entry(uint8_t index) {
	char entries[] = {'0', '1', '2', '3', '4', '5', '6', '7', 
			'8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

	return entries[index];
}





	
