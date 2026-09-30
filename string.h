#ifndef STRING_H
#define STRING_H

/* Copy a segment of memory to a new location
 *
 * Param: void* dest, the destination of the memory segment
 * Param: const void* src, the source of the memory segment
 * Param: size_t n, how many bytes to write to destination address
 *
 * Return: void*, a void* to the start of the destination address
 */
void* memcpy(void* dest, const void* src, size_t n);

/* Fill memory with a value
 *
 * Param: void* dest, the destination of where to store the filled memory
 * Param: int c, the integer value to fill the memory location
 * Param: size_t n, the number of bytes to write to the destination address
 *
 * Return: void*, a void* to the start of the newly filled destination
 */
void* memset(void* dest, int c, size_t n);

/* Return the size of a string
 *
 * Param: const char* str, the string we are looking at
 *
 * Return: size_t, the size of str
 */
size_t strlen(const char* str);

/* Copy a segment of memory to a new location
 *
 * Param: void* dest, the destination of the memory segment
 * Param: const void* src, the source of the memory segment
 * Param: size_t n, how many bytes to write to destination address
 *
 * Return: void*, a void* to the start of the destination address
 */
void* memmove(void* dest, const void* src, size_t n);

/* Compare a segment of memory for equivalence.
 *
 * Param: const void* ptr1, the first memory segment for comparison
 * Param: const void* ptr2, the second memory segment for comparison
 * Param: size_t num, the number of bytes to compare
 *
 * Return: int, <0 the first byte that does not match has a lower value in ptr1
 * 		>0 the first byte that does not match has a greater value in ptr1
 * 		=0 the contents of both memory blocks are equal
 */
int memcmp(const void* ptr1, const void* ptr2, size_t num);

/* Compares the C strings str1 to the C string str2.
 *
 * Param: const char* str1, first string to be compared
 * Param: const char* str2, second string to be compared
 *
 * Return: int, <0 the first char that does not match has a lower value in str1
 *		>0 the first char that does not match has a greater value in str1
 *		=0 the contents of both memory blocks are equal
 */
int strcmp(const char* str1, const char* str2);

/* Compares up to num characters of the C string str1 to those of the C string str2.
 *
 * Param: const char* str1, first string to be compared
 * Param: const char* str2, second string to be compared
 * Param: size_t num, maximum number of characters to be compared
 *
 * Return: int, <0 the first char that does not match has a lower value in str1
 *		>0 the first char that does not match has a greater value in str1
 *		=0 the contents of both memory blocks are equal
 */
int strncmp(const char* str1, const char* str2, size_t num);

/* Copy the C string pointed by source to the destination memory segment
 *
 * Param: char* dest, pointer to destination array where content is being copied
 * Param: const char* src, C string to be copied
 *
 * Return: char*, returns a pointer to the start of the destination
 */
char* strcpy(char* dest, const char* src);

/* Copies the first num characters of source to destination. If the end of the source C string is found
 * before num characters have been copied, destination is padded with zeros until a total of num characters
 * have been written to it.
 *
 * Param: char* dest, Pointer to a ddestination array where the content is to be copied
 * Param: const char* src, C string to be copied
 * Param: size_t num, Maximum number of characters to be copied form source
 *
 * Return: char*, Pointer to the start of the destination
 */ 
char* strncpy(char* dest, const char* src, size_t num);

#endif
