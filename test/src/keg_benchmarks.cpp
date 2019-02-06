#include <benchmark/benchmark.h>

#include "lager/keg.h"
#include "lager/lager_utils.h"

static void kegWriteOneUint32(benchmark::State& state)
{
    Keg k(".");

    k.addFormat("076ac37b-83dd-4fef-bc9d-16789794be87", "<?xml version=\"1.0\" encoding=\"UTF-8\"?><format version=\"BEERR01\" key=\"test\">"
                "<item name=\"column1\" type=\"uint32_t\" size=\"4\" offset=\"0\"/></format>");

    k.start();

    std::string uuid = lager_utils::getUuid("076ac37b-83dd-4fef-bc9d-16789794be87");
    uint64_t timestamp = 0;
    uint32_t column1 = 0;
    uint32_t dataSize = 0;

    for (auto _ : state)
    {
        state.PauseTiming();
        timestamp = lager_utils::getCurrentTime();
        column1++;

        std::vector<uint8_t> data;
        off_t offset = 0;

        for (size_t i = 0; i < uuid.size(); ++i)
        {
            data.push_back(uuid[i]);
        }

        offset += UUID_SIZE_BYTES;

        data.resize(data.size() + TIMESTAMP_SIZE_BYTES);
        *(reinterpret_cast<uint64_t*>(data.data() + offset)) = lager_utils::htonll(timestamp);

        offset += TIMESTAMP_SIZE_BYTES;

        data.resize(data.size() + 4);
        *(reinterpret_cast<uint32_t*>(data.data() + offset)) = htonl(column1);

        dataSize = data.size() * sizeof(data[0]);
        state.ResumeTiming();
        k.write(data, data.size());
    }

    k.stop();

    state.SetBytesProcessed(static_cast<int64_t>(state.iterations()) * dataSize);
}

BENCHMARK(kegWriteOneUint32);

static void kegWriteTenUint32(benchmark::State& state)
{
    Keg k(".");

    k.addFormat("076ac37b-83dd-4fef-bc9d-16789794be87", "<?xml version=\"1.0\" encoding=\"UTF-8\"?><format version=\"BEERR01\" key=\"test\">"
                "<item name=\"column1\" type=\"uint32_t\" size=\"4\" offset=\"0\"/>"
                "<item name=\"column2\" type=\"uint32_t\" size=\"4\" offset=\"4\"/>"
                "<item name=\"column3\" type=\"uint32_t\" size=\"4\" offset=\"8\"/>"
                "<item name=\"column4\" type=\"uint32_t\" size=\"4\" offset=\"12\"/>"
                "<item name=\"column5\" type=\"uint32_t\" size=\"4\" offset=\"16\"/>"
                "<item name=\"column6\" type=\"uint32_t\" size=\"4\" offset=\"20\"/>"
                "<item name=\"column7\" type=\"uint32_t\" size=\"4\" offset=\"24\"/>"
                "<item name=\"column8\" type=\"uint32_t\" size=\"4\" offset=\"28\"/>"
                "<item name=\"column9\" type=\"uint32_t\" size=\"4\" offset=\"32\"/>"
                "<item name=\"column10\" type=\"uint32_t\" size=\"4\" offset=\"36\"/>"
                "</format>");

    k.start();

    std::string uuid = lager_utils::getUuid("076ac37b-83dd-4fef-bc9d-16789794be87");
    uint64_t timestamp = 0;
    std::vector<uint32_t> items;
    items.resize(10);
    uint32_t dataSize = 0;

    for (auto _ : state)
    {
        state.PauseTiming();
        timestamp = lager_utils::getCurrentTime();

        for (uint8_t i = 0; i < items.size(); ++i)
        {
            items[i]++;
        }

        std::vector<uint8_t> data;
        off_t offset = 0;

        for (size_t i = 0; i < uuid.size(); ++i)
        {
            data.push_back(uuid[i]);
        }

        offset += UUID_SIZE_BYTES;

        data.resize(data.size() + TIMESTAMP_SIZE_BYTES);
        *(reinterpret_cast<uint64_t*>(data.data() + offset)) = lager_utils::htonll(timestamp);

        offset += TIMESTAMP_SIZE_BYTES;

        for (uint8_t i = 0; i < items.size(); ++i)
        {
            data.resize(data.size() + 4);
            *(reinterpret_cast<uint32_t*>(data.data() + offset)) = htonl(items[i]);
        }

        dataSize = data.size() * sizeof(data[0]);
        state.ResumeTiming();
        k.write(data, data.size());
    }

    k.stop();

    state.SetBytesProcessed(static_cast<int64_t>(state.iterations()) * dataSize);
}

BENCHMARK(kegWriteTenUint32);

static void kegWriteHundredUint32(benchmark::State& state)
{
    std::stringstream ss;
    off_t offset = 0;
    ss << "<?xml version=\"1.0\" encoding=\"UTF-8\"?><format version=\"BEERR01\" key=\"test\">";

    for (uint16_t i = 0; i < 100; ++i)
    {
        ss << "<item name=\"column" << i << "\" type=\"uint32_t\" size=\"4\" offset=\"" << offset << "\"/>";
        offset += 4;
    }

    ss << "</format>";

    Keg k(".");

    k.addFormat("076ac37b-83dd-4fef-bc9d-16789794be87", ss.str());

    k.start();

    std::string uuid = lager_utils::getUuid("076ac37b-83dd-4fef-bc9d-16789794be87");
    uint64_t timestamp = 0;
    std::vector<uint32_t> items;
    items.resize(100);
    uint32_t dataSize = 0;

    for (auto _ : state)
    {
        state.PauseTiming();
        timestamp = lager_utils::getCurrentTime();

        for (uint8_t i = 0; i < items.size(); ++i)
        {
            items[i]++;
        }

        std::vector<uint8_t> data;
        off_t offset = 0;

        for (size_t i = 0; i < uuid.size(); ++i)
        {
            data.push_back(uuid[i]);
        }

        offset += UUID_SIZE_BYTES;

        data.resize(data.size() + TIMESTAMP_SIZE_BYTES);
        *(reinterpret_cast<uint64_t*>(data.data() + offset)) = lager_utils::htonll(timestamp);

        offset += TIMESTAMP_SIZE_BYTES;

        for (uint8_t i = 0; i < items.size(); ++i)
        {
            data.resize(data.size() + 4);
            *(reinterpret_cast<uint32_t*>(data.data() + offset)) = htonl(items[i]);
        }

        dataSize = data.size() * sizeof(data[0]);
        state.ResumeTiming();
        k.write(data, data.size());
    }

    k.stop();

    state.SetBytesProcessed(static_cast<int64_t>(state.iterations()) * dataSize);
}

BENCHMARK(kegWriteHundredUint32);

BENCHMARK_MAIN();
