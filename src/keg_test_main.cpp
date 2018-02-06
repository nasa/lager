#include "keg.h"

int main(int argc, char* argv[])
{
    Keg k("/home/chuck/keg_test");

    k.start();

    std::vector<uint8_t> testData;
    testData.push_back(1);
    testData.push_back(2);
    testData.push_back(3);
    testData.push_back(4);
    testData.push_back(5);
    testData.push_back(6);

    k.write(testData, testData.size());

    k.stop();
}
