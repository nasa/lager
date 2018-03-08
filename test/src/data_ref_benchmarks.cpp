#include <benchmark/benchmark.h>

#include "data_ref_item.h"

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

static void getUint8(benchmark::State& state)
{
    uint8_t uint1 = 0;
    uint8_t* uint2 = new uint8_t;
    AbstractDataRefItem* item;
    item = new DataRefItem<uint8_t>("uint1", &uint1);

    for (auto _ : state)
    {
        item->getNetworkDataRef(uint2);
    }

    delete item;
    delete uint2;
}

BENCHMARK(getUint8);

static void getUint16(benchmark::State& state)
{
    uint16_t uint1 = 0;
    uint16_t* uint2 = new uint16_t;
    AbstractDataRefItem* item;
    item = new DataRefItem<uint16_t>("uint1", &uint1);

    for (auto _ : state)
    {
        item->getNetworkDataRef(uint2);
    }

    delete item;
    delete uint2;
}

BENCHMARK(getUint16);

static void getUint32(benchmark::State& state)
{
    uint32_t uint1 = 0;
    uint32_t* uint2 = new uint32_t;
    AbstractDataRefItem* item;
    item = new DataRefItem<uint32_t>("uint1", &uint1);

    for (auto _ : state)
    {
        item->getNetworkDataRef(uint2);
    }

    delete item;
    delete uint2;
}

BENCHMARK(getUint32);

static void getUint64(benchmark::State& state)
{
    uint64_t uint1 = 0;
    uint64_t* uint2 = new uint64_t;
    AbstractDataRefItem* item;
    item = new DataRefItem<uint64_t>("uint1", &uint1);

    for (auto _ : state)
    {
        item->getNetworkDataRef(uint2);
    }

    delete item;
    delete uint2;
}

BENCHMARK(getUint64);

static void getFloat(benchmark::State& state)
{
    float uint1 = 0;
    float* uint2 = new float;
    AbstractDataRefItem* item;
    item = new DataRefItem<float>("uint1", &uint1);

    for (auto _ : state)
    {
        item->getNetworkDataRef(uint2);
    }

    delete item;
    delete uint2;
}

BENCHMARK(getFloat);

static void getDouble(benchmark::State& state)
{
    double uint1 = 0;
    double* uint2 = new double;
    AbstractDataRefItem* item;
    item = new DataRefItem<double>("uint1", &uint1);

    for (auto _ : state)
    {
        item->getNetworkDataRef(uint2);
    }

    delete item;
    delete uint2;
}

BENCHMARK(getDouble);

BENCHMARK_MAIN();
