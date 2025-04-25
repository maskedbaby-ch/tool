#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct {
	int data;
	char *str;
} AutoFreeStruct;

void auto_free_struct(AutoFreeStruct **ptr) {
	if (*ptr) {
		printf("Auto-freeing struct (data=%d)\n", (*ptr)->data);
		printf("Auto-freeing struct (data=%s)\n", (*ptr)->str);
		free((*ptr)->str);
		free(*ptr);
		*ptr = NULL;
	}
}

#define AUTO_STRUCT(type, name, init_value) \
	__attribute__((cleanup(auto_free_struct))) type *name = init_value

int fun() {
	AUTO_STRUCT(AutoFreeStruct, obj, malloc(sizeof(AutoFreeStruct)));
	if (!obj) return 1;
	obj->str = malloc(12);
	memcpy(obj->str, "ddf", 3);
	obj->data = 100;
	printf("Using struct (data=%d) %s\n", obj->data, obj->str);
	return 0;
}

int main() {
	fun();
	return 0;
}

