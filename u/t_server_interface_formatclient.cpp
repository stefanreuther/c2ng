/**
  *  \file u/t_server_interface_formatclient.cpp
  *  \brief Test for server::interface::FormatClient
  */

#include <memory>
#include "server/interface/formatclient.hpp"

#include "t_server_interface.hpp"
#include "server/interface/formatserver.hpp"
#include "interpreter/values.hpp"
#include "interpreter/arguments.hpp"
#include "u/helper/callreceiver.hpp"
#include "afl/string/format.hpp"
#include "server/types.hpp"

/** Test chaining of clients and servers. */
void
TestServerInterfaceFormatClient::testChain()
{
    class FormatImpl : public server::interface::Format, public CallReceiver {
     public:
        virtual afl::data::Value* pack(String_t formatName, afl::data::Value* data, afl::base::Optional<String_t> format, afl::base::Optional<String_t> charset)
            {
                checkCall(afl::string::Format("pack('%s', '%s', %s, %s)", formatName, server::toString(data), format.orElse("none"), charset.orElse("none")));
                return interpreter::makeIntegerValue(3);
            }
        virtual afl::data::Value* unpack(String_t formatName, afl::data::Value* data, afl::base::Optional<String_t> format, afl::base::Optional<String_t> charset)
            {
                checkCall(afl::string::Format("unpack('%s', '%s', %s, %s)", formatName, server::toString(data), format.orElse("none"), charset.orElse("none")));
                return interpreter::makeIntegerValue(5);
            }
    };

    // Server/client chain
    FormatImpl step0;
    server::interface::FormatServer step1(step0);
    server::interface::FormatClient step2(step1);
    server::interface::FormatServer step3(step2);
    server::interface::FormatClient testee(step3);

    // Value(s)
    std::auto_ptr<afl::data::Value> data(interpreter::makeIntegerValue(42));
    std::auto_ptr<afl::data::Value> p;
    int32_t v;

    // Verify
    step0.expectCall("pack('text', '42', none, none)");
    p.reset(testee.pack("text", data.get(), afl::base::Nothing, afl::base::Nothing));
    TS_ASSERT(interpreter::checkIntegerArg(v, p.get()));
    TS_ASSERT_EQUALS(v, 3);

    step0.expectCall("pack('other', '42', form, none)");
    p.reset(testee.pack("other", data.get(), String_t("form"), afl::base::Nothing));
    TS_ASSERT(interpreter::checkIntegerArg(v, p.get()));
    TS_ASSERT_EQUALS(v, 3);

    step0.expectCall("pack('other', '42', form, CS)");
    p.reset(testee.pack("other", data.get(), String_t("form"), String_t("CS")));
    TS_ASSERT(interpreter::checkIntegerArg(v, p.get()));
    TS_ASSERT_EQUALS(v, 3);

    step0.expectCall("unpack('more', '42', none, CS)");
    p.reset(testee.unpack("more", data.get(), afl::base::Nothing, String_t("CS")));
    TS_ASSERT(interpreter::checkIntegerArg(v, p.get()));
    TS_ASSERT_EQUALS(v, 5);

    step0.expectCall("unpack('final', '42', F, CS)");
    p.reset(testee.unpack("final", data.get(), String_t("F"), String_t("CS")));
    TS_ASSERT(interpreter::checkIntegerArg(v, p.get()));
    TS_ASSERT_EQUALS(v, 5);

    step0.checkFinish();
}

