#include <benchmark/benchmark.h>

#include "lager/data_ref_item.h"

static void createUint8(benchmark::State& state)
{
    uint8_t uint1 = 0;
    AbstractDataRefItem* item;

    for (auto _ : state)
    {
        item = new DataRefItem<uint8_t>("uint1", &uint1);
    }

    delete item;
}

BENCHMARK(createUint8);

static void createUint16(benchmark::State& state)
{
    uint16_t uint1 = 0;
    AbstractDataRefItem* item;

    for (auto _ : state)
    {
        item = new DataRefItem<uint16_t>("uint1", &uint1);
    }

    delete item;
}

BENCHMARK(createUint16);

static void createUint32(benchmark::State& state)
{
    uint32_t uint1 = 0;
    AbstractDataRefItem* item;

    for (auto _ : state)
    {
        item = new DataRefItem<uint32_t>("uint1", &uint1);
    }

    delete item;
}

BENCHMARK(createUint32);

static void createUint64(benchmark::State& state)
{
    uint64_t uint1 = 0;
    AbstractDataRefItem* item;

    for (auto _ : state)
    {
        item = new DataRefItem<uint64_t>("uint1", &uint1);
    }

    delete item;
}

BENCHMARK(createUint64);

static void createFloat(benchmark::State& state)
{
    float uint1 = 0;
    AbstractDataRefItem* item;

    for (auto _ : state)
    {
        item = new DataRefItem<float>("uint1", &uint1);
    }

    delete item;
}

BENCHMARK(createFloat);

static void createDouble(benchmark::State& state)
{
    double uint1 = 0;
    AbstractDataRefItem* item;

    for (auto _ : state)
    {
        item = new DataRefItem<double>("uint1", &uint1);
    }

    delete item;
}

BENCHMARK(createDouble);

BENCHMARK_MAIN();
