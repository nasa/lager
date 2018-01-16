#include <memory>

#include <gtest/gtest.h>

#include <zmq.hpp>

#include "chp_server.h"
#include "chp_client.h"
#include "lager_utils.h"

class ChpTests : public ::testing::Test
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

TEST_F(ChpTests, ServerAddRemoveKeys)
{
    context.reset(new zmq::context_t(1));
    ChpServer c(12345);
    c.init(context);
    c.start();
    ASSERT_EQ(c.getHashMap().size(), 0);
    c.addOrUpdateKeyValue("testkey", "testvalue");
    lager_utils::sleep(100);
    ASSERT_STREQ(c.getHashMap()["testkey"].c_str(), "testvalue");
    c.removeKey("testkey");
    lager_utils::sleep(100);
    ASSERT_EQ(c.getHashMap().size(), 0);
    context->close();
    c.stop();
}

TEST_F(ChpTests, BothAddRemoveKeys)
{
    context.reset(new zmq::context_t(1));
    ChpServer s(12345);
    ChpClient c("localhost", 12345, 1000);

    s.init(context);
    c.init(context, lager_utils::getUuid());

    s.start();
    c.start();
    ASSERT_EQ(s.getHashMap().size(), 0);
    c.addOrUpdateKeyValue("testkey", "testvalue");
    lager_utils::sleep(100);
    ASSERT_STREQ(s.getHashMap()["testkey"].c_str(), "testvalue");
    c.removeKey("testkey");
    lager_utils::sleep(100);
    ASSERT_EQ(s.getHashMap().size(), 0);
    context->close();
    c.stop();
    s.stop();
}

TEST_F(ChpTests, AddMultipleKeys)
{
    context.reset(new zmq::context_t(1));
    ChpServer s(12345);
    ChpClient c("localhost", 12345, 1000);

    s.init(context);
    c.init(context, lager_utils::getUuid());

    s.start();
    c.start();
    ASSERT_EQ(s.getHashMap().size(), 0);
    c.addOrUpdateKeyValue("testkey1", "testvalue1");
    c.addOrUpdateKeyValue("testkey2", "testvalue2");
    lager_utils::sleep(100);
    ASSERT_STREQ(s.getHashMap()["testkey1"].c_str(), "testvalue1");
    c.removeKey("testkey1");
    lager_utils::sleep(100);
    ASSERT_EQ(s.getHashMap().size(), 1);
    context->close();
    c.stop();
    s.stop();
}
