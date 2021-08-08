/**
  *  \file u/t_interpreter_vmio_processloadcontext.cpp
  *  \brief Test for interpreter::vmio::ProcessLoadContext
  */

#include <memory>
#include "interpreter/vmio/processloadcontext.hpp"

#include "t_interpreter_vmio.hpp"
#include "afl/io/constmemorystream.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "interpreter/process.hpp"
#include "interpreter/singlecontext.hpp"
#include "interpreter/world.hpp"

/** Test deserialisation for loadMutex().
    There used to be a typo in there. */
void
TestInterpreterVmioProcessLoadContext::testLoadMutex()
{
    // Environment classes
    class TestContext : public interpreter::SingleContext {
     public:
        TestContext(String_t name, String_t note, interpreter::Process* owner)
            : m_name(name), m_note(note), m_owner(owner)
            { }
        virtual PropertyAccessor* lookup(const afl::data::NameQuery& /*name*/, PropertyIndex_t& /*result*/)
            { return 0; }
        virtual TestContext* clone() const
            { return new TestContext(*this); }
        virtual game::map::Object* getObject()
            { return 0; }
        virtual void enumProperties(interpreter::PropertyAcceptor& /*acceptor*/)
            { }
        virtual String_t toString(bool /*readable*/) const
            { return String_t(); }
        virtual void store(interpreter::TagNode& /*out*/, afl::io::DataSink& /*aux*/, afl::charset::Charset& /*cs*/, interpreter::SaveContext& /*ctx*/) const
            { }

        String_t m_name;
        String_t m_note;
        interpreter::Process* m_owner;
    };

    class TestParent : public interpreter::vmio::LoadContext {
     public:
        virtual afl::data::Value* loadBCO(uint32_t /*id*/)
            { return 0; }
        virtual afl::data::Value* loadArray(uint32_t /*id*/)
            { return 0; }
        virtual afl::data::Value* loadHash(uint32_t /*id*/)
            { return 0; }
        virtual afl::data::Value* loadStructureValue(uint32_t /*id*/)
            { return 0; }
        virtual afl::data::Value* loadStructureType(uint32_t /*id*/)
            { return 0; }
        virtual interpreter::Context* loadContext(const interpreter::TagNode& /*tag*/, afl::io::Stream& /*aux*/)
            { return 0; }
        virtual interpreter::Context* loadMutex(const String_t& name, const String_t& note, interpreter::Process* owner)
            {
                return new TestContext(name, note, owner);
            }
        virtual interpreter::Process* createProcess()
            { return 0; }
        virtual void finishProcess(interpreter::Process& /*proc*/)
            { }
    };

    // Environment
    TestParent parent;
    afl::sys::Log log;
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    interpreter::World world(log, tx, fs);
    interpreter::Process proc(world, "testLoadMutex", 99);

    // Test
    interpreter::vmio::ProcessLoadContext testee(parent, proc);

    // Load
    static const uint8_t DATA[] = {
        2,0,0,0, 3,0,0,0,
        'h','i', 'm','o','m'
    };

    // - With "not this process" flag
    {
        afl::io::ConstMemoryStream aux(DATA);
        interpreter::TagNode tag;
        tag.tag = tag.Tag_Mutex;
        tag.value = 0;
        std::auto_ptr<interpreter::Context> result(testee.loadContext(tag, aux));
        TestContext* ctx = dynamic_cast<TestContext*>(result.get());
        TS_ASSERT(ctx != 0);
        TS_ASSERT_EQUALS(ctx->m_name, "hi");
        TS_ASSERT_EQUALS(ctx->m_note, "mom");
        TS_ASSERT(ctx->m_owner == 0);
    }

    // - With "is this process" flag
    {
        afl::io::ConstMemoryStream aux(DATA);
        interpreter::TagNode tag;
        tag.tag = tag.Tag_Mutex;
        tag.value = 1;
        std::auto_ptr<interpreter::Context> result(testee.loadContext(tag, aux));
        TestContext* ctx = dynamic_cast<TestContext*>(result.get());
        TS_ASSERT(ctx != 0);
        TS_ASSERT_EQUALS(ctx->m_name, "hi");
        TS_ASSERT_EQUALS(ctx->m_note, "mom");
        TS_ASSERT_EQUALS(ctx->m_owner, &proc);
    }
}

