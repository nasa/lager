#include <benchmark/benchmark.h>

#include "lager/tap.h"
#include "lager/lager_utils.h"

static void tapInitHundredUints(benchmark::State& state)
{
    int arraySize = 100;
    std::stringstream ss;

    Tap t;
    t.init("localhost", 12345, 1000);

    std::vector<uint32_t> array;

    for (auto _ : state)
    {
        for (unsigned int i = 0; i < arraySize; ++i)
        {
            ss.clear();
            ss << "array" << i;
            array.push_back(uint32_t(0));
            t.addItem(new DataRefItem<uint32_t>(ss.str(), &array[i]));
        }
    }
}

BENCHMARK(tapInitHundredUints);

static void tapInitThousandUints(benchmark::State& state)
{
    int arraySize = 1000;
    std::stringstream ss;

    Tap t;
    t.init("localhost", 12345, 1000);

    std::vector<uint32_t> array;

    for (auto _ : state)
    {
        for (unsigned int i = 0; i < arraySize; ++i)
        {
            ss.clear();
            ss << "array" << i;
            array.push_back(uint32_t(0));
            t.addItem(new DataRefItem<uint32_t>(ss.str(), &array[i]));
        }
    }
}

BENCHMARK(tapInitThousandUints);

BENCHMARK_MAIN();
