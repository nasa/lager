#include <benchmark/benchmark.h>

#include "lager/data_format.h"
#include "lager/data_format_parser.h"

static void parseFromFile(benchmark::State& state)
{
    DataFormatParser p("data_format.xsd");

    for (auto _ : state)
    {
        p.parseFromFile("good_format.xml");
    }
}

BENCHMARK(parseFromFile);

static void parseFromString(benchmark::State& state)
{
    DataFormatParser p("data_format.xsd");

    for (auto _ : state)
    {
        p.parseFromString("<?xml version=\"1.0\" encoding=\"UTF-8\"?><format version=\"BEERR01\" key=\"test\">"
                          "<item name=\"column1\" type=\"string\" size=\"255\" offset=\"0\"/>"
                          "<item name=\"column2\" type=\"integer\" size=\"4\" offset=\"255\"/></format>");
    }
}

BENCHMARK(parseFromString);

static void createFromTenDataRefs(benchmark::State& state)
{
    int arraySize = 10;

    std::stringstream ss;
    DataFormatParser p("data_format.xsd");
    std::vector<AbstractDataRefItem*> dataRefItems;
    std::vector<uint32_t> data;

    for (unsigned int i = 0; i < arraySize; ++i)
    {
        ss.clear();
        ss << "data" << i;
        data.push_back(uint32_t(0));
        dataRefItems.push_back(new DataRefItem<uint32_t>(ss.str(), &data[i]));
    }

    for (auto _ : state)
    {
        p.createFromDataRefItems(dataRefItems, "test", "test_key");
    }
}

BENCHMARK(createFromTenDataRefs);

static void createFromHundredDataRefs(benchmark::State& state)
{
    int arraySize = 100;

    std::stringstream ss;
    DataFormatParser p("data_format.xsd");
    std::vector<AbstractDataRefItem*> dataRefItems;
    std::vector<uint32_t> data;

    for (unsigned int i = 0; i < arraySize; ++i)
    {
        ss.clear();
        ss << "data" << i;
        data.push_back(uint32_t(0));
        dataRefItems.push_back(new DataRefItem<uint32_t>(ss.str(), &data[i]));
    }

    for (auto _ : state)
    {
        p.createFromDataRefItems(dataRefItems, "test", "test_key");
    }
}

BENCHMARK(createFromHundredDataRefs);

static void createFromThousandDataRefs(benchmark::State& state)
{
    int arraySize = 1000;

    std::stringstream ss;
    DataFormatParser p("data_format.xsd");
    std::vector<AbstractDataRefItem*> dataRefItems;
    std::vector<uint32_t> data;

    for (unsigned int i = 0; i < arraySize; ++i)
    {
        ss.clear();
        ss << "data" << i;
        data.push_back(uint32_t(0));
        dataRefItems.push_back(new DataRefItem<uint32_t>(ss.str(), &data[i]));
    }

    for (auto _ : state)
    {
        p.createFromDataRefItems(dataRefItems, "test", "test_key");
    }
}

BENCHMARK(createFromThousandDataRefs);

BENCHMARK_MAIN();
