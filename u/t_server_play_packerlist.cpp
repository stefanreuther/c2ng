/**
  *  \file u/t_server_play_packerlist.cpp
  *  \brief Test for server::play::PackerList
  */

#include <memory>
#include "server/play/packerlist.hpp"

#include "t_server_play.hpp"
#include "server/play/packer.hpp"
#include "afl/data/access.hpp"

namespace {
    class TestPacker : public server::play::Packer {
     public:
        TestPacker(bool& gate, String_t name, int value)
            : m_gate(gate), m_name(name), m_value(value)
            { }

        virtual server::Value_t* buildValue() const
            {
                TS_ASSERT(m_gate);
                return server::makeIntegerValue(m_value);
            }
        
        virtual String_t getName() const
            {
                return m_name;
            }
     private:
        bool& m_gate;
        String_t m_name;
        int m_value;
    };
}

void
TestServerPlayPackerList::testIt()
{
    server::play::PackerList testee;
    bool gate = false;

    // Populate it
    testee.addNew(new TestPacker(gate, "v1", 1));
    testee.addNew(new TestPacker(gate, "v2", 2));
    testee.addNew(new TestPacker(gate, "v1", 1));
    testee.addNew(0);
    testee.addNew(new TestPacker(gate, "v3", 3));

    // Verify
    gate = true;
    std::auto_ptr<server::Value_t> result(testee.buildValue());
    afl::data::Access a(result.get());
    TS_ASSERT_EQUALS(a("v1").toInteger(), 1);
    TS_ASSERT_EQUALS(a("v2").toInteger(), 2);
    TS_ASSERT_EQUALS(a("v3").toInteger(), 3);
}

