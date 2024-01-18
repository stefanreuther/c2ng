/**
  *  \file test/client/downlinktest.cpp
  *  \brief Test for client::Downlink
  */

#include "client/downlink.hpp"

#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/testrunner.hpp"
#include "gfx/nullengine.hpp"
#include "gfx/nullresourceprovider.hpp"
#include "ui/root.hpp"
#include "util/requestthread.hpp"

namespace {
    struct T {
        int i;
    };
}

AFL_TEST("client.Downlink", a)
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

    a.check("01. ok", ok);
    a.checkEqual("02. value", object.i, 43);
}
