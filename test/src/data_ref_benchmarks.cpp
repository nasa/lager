#include <benchmark/benchmark.h>

#include "data_ref_item.h"

static void createRefItem(benchmark::State& state)
{
    uint32_t uint1 = 0;
    AbstractDataRefItem* item;

    for (auto _ : state)
    {
        item = new DataRefItem<uint32_t>("uint1", &uint1);
    }

    delete item;
}

BENCHMARK(createRefItem);

BENCHMARK_MAIN();
