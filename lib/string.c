#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <lib/string.h>

void* memcpy(void* dest, const void* src, size_t n) {
	uint8_t* byte_dest = (uint8_t*) dest;
	uint8_t* byte_src = (uint8_t*) src;

	for (size_t i = 0; i < n; i++) {
		byte_dest[i] = byte_src[i];
	}

	return byte_dest;
}

void* memset(void* dest, int c, size_t n) {
	uint8_t* int_dest = (uint8_t*) dest;

	for (size_t i = 0; i < n; i++) {
		int_dest[i] = (uint8_t)c;
	}

	return int_dest;
}

size_t strlen(const char* str) {
	size_t len = 0;
	while(str[len]) {
		len++;
	}
	return len;
}

void* memmove(void* dest, const void* src, size_t n) {
	uint8_t* byte_dest = (uint8_t*) dest;
	uint8_t* byte_src = (uint8_t*) src;

	if(src < dest) {
		for (size_t i = n; i != 0; i--) {
			byte_dest[i-1] = byte_src[i-1];
		}
	} else {
		byte_dest = (uint8_t*) memcpy(dest, src, n);
	}

	return byte_dest;
}

int memcmp(const void* ptr1, const void* ptr2, size_t num) {
	uint8_t* byte_ptr1 = (uint8_t*) ptr1;
	uint8_t* byte_ptr2 = (uint8_t*) ptr2;



	for(size_t i = 0; i < num; i++) {
		if (byte_ptr1[i] < byte_ptr2[i]) return -1;
		if (byte_ptr1[i] > byte_ptr2[i]) return 1;
	}
	
	return 0;
}

int strcmp(const char* str1, const char* str2) {
	size_t index = 0;

	while(str1[index] && str2[index]) {
		if(str1[index] < str2[index]) return -1;
		if(str1[index] > str2[index]) return 1;
		index++;
	}

	if(!str1[index] && !str2[index]) return 0;
	if(!str1[index]) return -1;

	return 1;
}

int strncmp(const char* str1, const char* str2, size_t num) {
	size_t i = 0;

	for(; i < num && (str1[i] && str2[i]); i++) {
		if(str1[i] < str2[i]) return -1;
		if(str1[i] > str2[i]) return 1;
	}

	if(!str1[i] && !str2[i]) return 0;
	if(!str1[i]) return -1;
	if(!str2[i]) return 1;

	return 0;
}

char* strcpy(char* dest, const char* src) {
	char* ret = dest;

	while(*src) {
		*dest = *src;
		dest++;
		src++;
	}

	*dest = *src;

	return ret;
}

char* strncpy(char* dest, const char* src, size_t num) {
	char* ret = dest;
	bool passed_null = false;

	for(size_t i = 0; i < num; i++) {
		if(!passed_null) {
			if(!src[i]) passed_null = true;
		}

		if(passed_null) {
			dest[i] = (char) 0;
		} else {
			dest[i] = src[i];
		}
	}

	return ret;
}




