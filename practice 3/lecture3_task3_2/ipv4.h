#ifndef IPV4_H
#define IPV4_H

#include <stddef.h>
#include <stdint.h>

#define IPV4_STRING_SIZE 16

/*
 * Преобразует строку вида "192.168.1.10" в 32-битное число.
 * Возвращает 0 при успехе и -1 при ошибке.
 */
int ipv4_parse(const char *text, uint32_t *address);

/*
 * Преобразует 32-битный IPv4-адрес в строку.
 * Буфер должен содержать не менее IPV4_STRING_SIZE байт.
 */
void ipv4_format(uint32_t address, char result[IPV4_STRING_SIZE]);

/*
 * Проверяет, является ли значение корректной маской подсети:
 * сначала идут единицы, затем только нули.
 */
int ipv4_mask_is_valid(uint32_t mask);

/* Возвращает длину префикса корректной маски, например 24 для /24. */
unsigned int ipv4_mask_prefix_length(uint32_t mask);

/* Проверяет принадлежность двух адресов одной подсети. */
int ipv4_same_subnet(uint32_t first, uint32_t second, uint32_t mask);

#endif
