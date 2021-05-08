/**
  *  \file u/t_client_downlink.cpp
  *  \brief Test for client::Downlink
  */

#include "client/downlink.hpp"

#include "t_client.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "gfx/nullengine.hpp"
#include "gfx/nullresourceprovider.hpp"
#include "ui/root.hpp"
#include "util/requestthread.hpp"

namespace {
    struct T {
        int i;
    };
}

void
TestClientDownlink::testIt()
{
    // UI side
    gfx::NullEngine engine;
    gfx::NullResourceProvider provider;
    ui::Root root(engine, provider, gfx::WindowParameters());

    // Worker side
    afl::sys::Log log;
    afl::string::NullTranslator tx;
    util::RequestThread thread("TestClientDownlink", log, tx);
    T object = {42};
    util::RequestReceiver<T> receiver(thread, object);

    // Test
    class Task : public util::Request<T> {
     public:
        void handle(T& i)
            { ++i.i; }
    };
    client::Downlink testee(root, tx);
    Task t;
    bool ok = testee.call(receiver.getSender(), t);

    TS_ASSERT(ok);
    TS_ASSERT_EQUALS(object.i, 43);
}

