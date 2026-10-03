#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "terminal.h"
#include "string.h"

/* Global terminal_info used throughout function calls */
struct terminal_info terminal;

uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg) {
	return fg | bg << 4;
}

uint16_t vga_entry(unsigned char uc, uint8_t color) {
	return (uint16_t) uc | (uint16_t) color << 8;
}

void terminal_initialize(void) {
	terminal.row = 0;
	terminal.col = 0;
	terminal.color = vga_entry_color(VGA_COLOR_LIGHT_GREY,
					    VGA_COLOR_BLACK);
	terminal.buffer = (uint16_t*) VGA_MEMORY;

	for (size_t y = 0; y < VGA_HEIGHT; y++) {
		for(size_t x = 0; x < VGA_WIDTH; x++) {
			const size_t index = y * VGA_WIDTH + x;
			terminal.buffer[index] = vga_entry(' ', 
						    terminal.color);
		}
	}
}

void terminal_setcolor(uint8_t color) {
	terminal.color = color; 
}


void terminal_putentryat(char c, uint8_t color, size_t x, size_t y) {
	const size_t index = y * VGA_WIDTH + x;
	terminal.buffer[index] = vga_entry(c, color);
}

void terminal_putchar(char c) {
	terminal_putentryat(c, terminal.color, terminal.col, terminal.row);

	if ( ++(terminal.col) == VGA_WIDTH) {
		terminal.col = 0;
		if ( ++(terminal.row) == VGA_HEIGHT) {
			terminal.row = 0;
		}
	}
}

void terminal_write(const char* data, size_t size) {
	for (size_t i = 0; i < size; i++) {
		terminal_putchar(data[i]);
	}
}

void terminal_writestring(const char* data) {
	terminal_write(data, strlen(data));
}

void kputchar(uint32_t ch) {
	char character = (char) ch;

	terminal_putchar(character);
}

void kputs(const char* ch) {
	terminal_writestring(ch);
}

void print_newline() {
	if ( ++(terminal.row) == VGA_HEIGHT) {
		terminal.row = 0;
	}
	terminal.col = 0;
}
