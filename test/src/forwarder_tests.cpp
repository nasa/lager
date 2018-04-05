#include <memory>

#include <zmq.hpp>

#include <gtest/gtest.h>

#include "lager/forwarder.h"
#include "lager/lager_utils.h"

class ForwarderTests : public ::testing::Test
{
protected:
    virtual void SetUp()
    {

    }

    virtual void TearDown()
    {

    }
    std::shared_ptr<zmq::context_t> context;
};

TEST_F(ForwarderTests, DoesItWork)
{
    context.reset(new zmq::context_t(1));

    Forwarder f(12345 + 10);
    f.init(context);
    f.start();

    lager_utils::sleepMillis(1000);

    context->close();
    f.stop();
}

int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
