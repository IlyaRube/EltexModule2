#ifndef SIMULATION_H
#define SIMULATION_H

#include <stddef.h>
#include <stdint.h>

typedef struct PacketStatistics {
    size_t total;
    size_t local;
    size_t external;
} PacketStatistics;

/*
 * Функция-генератор возвращает очередной случайный IPv4-адрес.
 * Контекст позволяет передать генератору собственные данные.
 */
typedef uint32_t (*PacketGenerator)(void *context);

/*
 * Наблюдатель вызывается после обработки каждого пакета.
 * Благодаря этому логика не зависит от printf и пользовательского интерфейса.
 */
typedef void (*PacketObserver)(size_t packet_number,
                               uint32_t destination,
                               int is_local,
                               void *context);

/*
 * Выполняет имитацию обработки count пакетов.
 * Возвращает 0 при успехе и -1 при неверных аргументах.
 */
int simulation_run(uint32_t gateway,
                   uint32_t mask,
                   size_t count,
                   PacketGenerator generator,
                   void *generator_context,
                   PacketObserver observer,
                   void *observer_context,
                   PacketStatistics *statistics);

#endif
