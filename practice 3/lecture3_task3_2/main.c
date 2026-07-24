#include "ipv4.h"
#include "simulation.h"

#include <errno.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static void print_usage(const char *program_name)
{
    fprintf(stderr,
            "Использование:\n"
            "  %s <IP шлюза> <маска подсети> <количество пакетов>\n\n"
            "Пример:\n"
            "  %s 192.168.1.1 255.255.255.0 20\n",
            program_name,
            program_name);
}

static int parse_packet_count(const char *text, size_t *count)
{
    char *end = NULL;
    uintmax_t value;

    if (text == NULL || count == NULL || text[0] == '\0' || text[0] == '-') {
        return -1;
    }

    errno = 0;
    value = strtoumax(text, &end, 10);

    if (errno != 0 || end == text || *end != '\0'
        || value == 0U || value > (uintmax_t)SIZE_MAX) {
        return -1;
    }

    *count = (size_t)value;
    return 0;
}

static uint32_t generate_random_ipv4(void *context)
{
    (void)context;

    /*
     * rand() не обязан возвращать все 32 случайных бита.
     * Поэтому адрес собирается из четырёх случайных октетов.
     */
    return ((uint32_t)(rand() & 0xFF) << 24)
         | ((uint32_t)(rand() & 0xFF) << 16)
         | ((uint32_t)(rand() & 0xFF) << 8)
         | (uint32_t)(rand() & 0xFF);
}

static void print_packet(size_t packet_number,
                         uint32_t destination,
                         int is_local,
                         void *context)
{
    char destination_text[IPV4_STRING_SIZE];

    (void)context;

    ipv4_format(destination, destination_text);

    printf("Пакет %zu: %-15s -> %s\n",
           packet_number,
           destination_text,
           is_local != 0 ? "своя подсеть" : "другая сеть");
}

int main(int argc, char *argv[])
{
    uint32_t gateway;
    uint32_t mask;
    size_t packet_count;
    PacketStatistics statistics;
    char gateway_text[IPV4_STRING_SIZE];
    char mask_text[IPV4_STRING_SIZE];
    char network_text[IPV4_STRING_SIZE];
    const double local_percent_divisor = 100.0;
    double local_percent;
    double external_percent;

    if (argc != 4) {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    if (ipv4_parse(argv[1], &gateway) != 0) {
        fprintf(stderr, "Ошибка: некорректный IP-адрес шлюза: %s\n", argv[1]);
        return EXIT_FAILURE;
    }

    if (ipv4_parse(argv[2], &mask) != 0 || ipv4_mask_is_valid(mask) == 0) {
        fprintf(stderr, "Ошибка: некорректная маска подсети: %s\n", argv[2]);
        return EXIT_FAILURE;
    }

    if (parse_packet_count(argv[3], &packet_count) != 0) {
        fprintf(stderr,
                "Ошибка: количество пакетов должно быть положительным целым числом.\n");
        return EXIT_FAILURE;
    }

    srand((unsigned int)time(NULL));

    ipv4_format(gateway, gateway_text);
    ipv4_format(mask, mask_text);
    ipv4_format(gateway & mask, network_text);

    printf("Шлюз:          %s\n", gateway_text);
    printf("Маска:         %s (/%u)\n",
           mask_text,
           ipv4_mask_prefix_length(mask));
    printf("Адрес подсети:  %s\n", network_text);
    printf("Пакетов:       %zu\n\n", packet_count);

    if (simulation_run(gateway,
                       mask,
                       packet_count,
                       generate_random_ipv4,
                       NULL,
                       print_packet,
                       NULL,
                       &statistics) != 0) {
        fprintf(stderr, "Ошибка при выполнении имитации.\n");
        return EXIT_FAILURE;
    }

    /*
     * Деление выполняется в double, чтобы получить проценты
     * с дробной частью.
     */
    local_percent =
        (double)statistics.local * local_percent_divisor
        / (double)statistics.total;

    external_percent =
        (double)statistics.external * local_percent_divisor
        / (double)statistics.total;

    printf("\n=== Статистика ===\n");
    printf("Своя подсеть:  %zu пакетов (%.2f%%)\n",
           statistics.local,
           local_percent);
    printf("Другие сети:   %zu пакетов (%.2f%%)\n",
           statistics.external,
           external_percent);
    printf("Всего:         %zu пакетов (100.00%%)\n",
           statistics.total);

    return EXIT_SUCCESS;
}
