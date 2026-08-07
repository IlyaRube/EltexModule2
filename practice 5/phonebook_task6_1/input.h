#ifndef INPUT_H
#define INPUT_H

#include <stddef.h>

int input_read_line(const char *prompt, char *buffer, size_t buffer_size);
int input_read_int_range(const char *prompt, int min_value, int max_value, int *value);
void input_trim(char *text);

#endif
