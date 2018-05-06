/**
  *  \file u/t_client_downlink.cpp
  *  \brief Test for client::Downlink
  */

#include "client/downlink.hpp"

#include "t_client.hpp"
#include "gfx/nullengine.hpp"
#include "gfx/nullresourceprovider.hpp"
#include "ui/root.hpp"
#include "afl/sys/log.hpp"
#include "util/requestthread.hpp"

void
TestClientDownlink::testIt()
{
    // UI side
    gfx::NullEngine engine;
    gfx::NullResourceProvider provider;
    ui::Root root(engine, provider, gfx::WindowParameters());

    // Worker side
    afl::sys::Log log;
    util::RequestThread thread("TestClientDownlink", log);
    int object = 42;
    util::RequestReceiver<int> receiver(thread, object);

    // Test
    class Task : public util::Request<int> {
     public:
        void handle(int& i)
            { ++i; }
    };
    client::Downlink testee(root);
    Task t;
    bool ok = testee.call(receiver.getSender(), t);

    TS_ASSERT(ok);
    TS_ASSERT_EQUALS(object, 43);
}

