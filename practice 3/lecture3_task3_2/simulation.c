#include "simulation.h"

#include "ipv4.h"

int simulation_run(uint32_t gateway,
                   uint32_t mask,
                   size_t count,
                   PacketGenerator generator,
                   void *generator_context,
                   PacketObserver observer,
                   void *observer_context,
                   PacketStatistics *statistics)
{
    if (count == 0U || generator == NULL || statistics == NULL) {
        return -1;
    }

    statistics->total = count;
    statistics->local = 0U;
    statistics->external = 0U;

    for (size_t index = 0; index < count; ++index) {
        const uint32_t destination = generator(generator_context);
        const int is_local = ipv4_same_subnet(gateway, destination, mask);

        if (is_local != 0) {
            ++statistics->local;
        } else {
            ++statistics->external;
        }

        if (observer != NULL) {
            observer(index + 1U,
                     destination,
                     is_local,
                     observer_context);
        }
    }

    return 0;
}
