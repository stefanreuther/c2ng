/**
  *  \file test/server/play/packerlisttest.cpp
  *  \brief Test for server::play::PackerList
  */

#include "server/play/packerlist.hpp"

#include "afl/data/access.hpp"
#include "afl/test/testrunner.hpp"
#include "server/play/packer.hpp"
#include <memory>

namespace {
    class TestPacker : public server::play::Packer {
     public:
        TestPacker(afl::test::Assert a, bool& gate, String_t name, int value)
            : m_assert(a), m_gate(gate), m_name(name), m_value(value)
            { }

        virtual server::Value_t* buildValue() const
            {
                m_assert.check("buildValue not expected yet", m_gate);
                return server::makeIntegerValue(m_value);
            }

        virtual String_t getName() const
            {
                return m_name;
            }
     private:
        afl::test::Assert m_assert;
        bool& m_gate;
        String_t m_name;
        int m_value;
    };
}

AFL_TEST("server.play.PackerList", a)
{
    server::play::PackerList testee;
    bool gate = false;

    // Populate it
    testee.addNew(new TestPacker(a, gate, "v1", 1));
    testee.addNew(new TestPacker(a, gate, "v2", 2));
    testee.addNew(new TestPacker(a, gate, "v1", 1));
    testee.addNew(0);
    testee.addNew(new TestPacker(a, gate, "v3", 3));

    // Verify
    gate = true;
    std::auto_ptr<server::Value_t> result(testee.buildValue());
    afl::data::Access ap(result.get());
    a.checkEqual("01", ap("v1").toInteger(), 1);
    a.checkEqual("02", ap("v2").toInteger(), 2);
    a.checkEqual("03", ap("v3").toInteger(), 3);
}
