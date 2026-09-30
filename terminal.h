#ifndef TERMINAL_H
#define TERMINAL_H

#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_MEMORY 0xC03FF000

/* Hardware test mode color constants */
enum vga_color {
	VGA_COLOR_BLACK = 0,
	VGA_COLOR_BLUE = 1,
	VGA_COLOR_GREEN = 2,
	VGA_COLOR_CYAN = 3,
	VGA_COLOR_RED = 4,
	VGA_COLOR_MAGENTA = 5,
	VGA_COLOR_BROWN = 6,
	VGA_COLOR_LIGHT_GREY = 7,
	VGA_COLOR_DARK_GREY = 8,
	VGA_COLOR_LIGHT_BLUE = 9,
	VGA_COLOR_LIGHT_GREEN = 10,
	VGA_COLOR_LIGHT_CYAN = 11,
	VGA_COLOR_LIGHT_RED = 12,
	VGA_COLOR_LIGHT_MAGENTA = 13,
	VGA_COLOR_LIGHT_BROWN = 14,
	VGA_COLOR_WHITE = 15,
};

struct terminal_info {
	size_t row, col;
	uint8_t color;
	uint16_t* buffer;
};

/* Create a 1 byte VGA color entry */
uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg);

/* Create a 2 byte VGA entry */
uint16_t vga_entry(unsigned char uc, uint8_t color);

/* Initialize the terminal */
void terminal_initialize(void);

/* Set the terminal color */
void terminal_setcolor(uint8_t color);

/* Place a character */
void terminal_putentryat(char c, uint8_t color, size_t x, size_t y);

/* Place character wrapper */
void terminal_putchar(char c);

/* Write single characters at a time */
void terminal_write(const char* data, size_t size);

/* Write a string to the terminal */
void terminal_writestring(const char* data);

/* A wrapper for writing a character, used in kprintf */
void kputchar(uint32_t ch);

/* Wrapper for printing a 'string' */
void kputs(const char* ch);

/* Print a new line */
void print_newline();
#endif
