/**
  *  \file u/t_interpreter_filefunctions.cpp
  *  \brief Test for interpreter::FileFunctions
  */

#include "interpreter/filefunctions.hpp"

#include "t_interpreter.hpp"
#include "afl/data/floatvalue.hpp"
#include "afl/io/filemapping.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/bytecodeobject.hpp"
#include "interpreter/defaultstatementcompilationcontext.hpp"
#include "interpreter/memorycommandsource.hpp"
#include "interpreter/process.hpp"
#include "interpreter/singlecontext.hpp"
#include "interpreter/statementcompilationcontext.hpp"
#include "interpreter/statementcompiler.hpp"
#include "interpreter/values.hpp"
#include "interpreter/world.hpp"

namespace {
    class GlobalContextMock : public interpreter::SingleContext, public interpreter::Context::PropertyAccessor {
     public:
        explicit GlobalContextMock(interpreter::World& world)
            : m_world(world)
            { }
        ~GlobalContextMock()
            { }

        // Context:
        virtual Context::PropertyAccessor* lookup(const afl::data::NameQuery& name, PropertyIndex_t& result)
            {
                afl::data::NameMap::Index_t ix = m_world.globalPropertyNames().getIndexByName(name);
                if (ix != afl::data::NameMap::nil) {
                    result = PropertyIndex_t(ix);
                    return this;
                } else {
                    return 0;
                }
            }
        virtual void set(PropertyIndex_t index, const afl::data::Value* value)
            { m_world.globalValues().set(index, value); }
        virtual afl::data::Value* get(PropertyIndex_t index)
            { return afl::data::Value::cloneOf(m_world.globalValues().get(index)); }
        virtual GlobalContextMock* clone() const
            { return new GlobalContextMock(m_world); }
        virtual afl::base::Deletable* getObject()
            { return 0; }
        virtual void enumProperties(interpreter::PropertyAcceptor& /*acceptor*/) const
            { }

        // BaseValue:
        virtual String_t toString(bool /*readable*/) const
            { return "<gcm>"; }
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const
            { rejectStore(out, aux, ctx); }

     private:
        interpreter::World& m_world;
    };

    class StreamMock : public afl::io::Stream {
     public:
        StreamMock()
            : m_pos(0),
              m_size(0)
            { }
        virtual size_t read(Bytes_t m)
            { m.fill(0); return m.size(); }
        virtual size_t write(ConstBytes_t m)
            { return m.size(); }
        virtual void flush()
            { }
        virtual void setPos(FileSize_t pos)
            { m_pos = pos; }
        virtual FileSize_t getPos()
            { return m_pos; }
        virtual FileSize_t getSize()
            { return m_size; }
        virtual uint32_t getCapabilities()
            { return CanRead|CanWrite|CanSeek; }
        virtual String_t getName()
            { return String_t(); }
        virtual afl::base::Ref<Stream> createChild()
            { throw "geht ned"; }
        virtual afl::base::Ptr<afl::io::FileMapping> createFileMapping(FileSize_t /*limit*/)
            { return 0; }
        void setSize(FileSize_t sz)
            { m_size = sz; }
     private:
        FileSize_t m_pos;
        FileSize_t m_size;
    };


    void checkStatement(interpreter::World& world, const char* stmt)
    {
        // Build a command source
        interpreter::MemoryCommandSource mcs;
        const char* q = stmt;
        while (const char* p = strchr(q, '\n')) {
            mcs.addLine(String_t(q, p - q));
            q = p+1;
        }
        mcs.addLine(q);

        // Build compilation environment
        interpreter::Process exec(world, "checkStatement", 9);
        interpreter::DefaultStatementCompilationContext scc(world);
        scc.withStaticContext(&exec);
        scc.withFlag(scc.LinearExecution);
        scc.withFlag(scc.ExpressionsAreStatements);

        interpreter::BCORef_t bco = interpreter::BytecodeObject::create(true);
        interpreter::StatementCompiler::Result result = interpreter::StatementCompiler(mcs).compileList(*bco, scc);

        TSM_ASSERT_DIFFERS(stmt, result, interpreter::StatementCompiler::CompiledExpression);

        exec.pushFrame(bco, false);
        exec.run();
        TSM_ASSERT_EQUALS(stmt, exec.getState(), interpreter::Process::Ended);
        TSM_ASSERT_EQUALS(stmt, exec.getError().what(), String_t());
    }

    void checkInteger(interpreter::World& world, const char* name, int32_t expectedValue)
    {
        afl::data::NameMap::Index_t index = world.globalPropertyNames().getIndexByName(name);
        TSM_ASSERT(name, index != afl::data::NameMap::nil);

        int32_t foundValue;
        TSM_ASSERT(name, interpreter::checkIntegerArg(foundValue, world.globalValues().get(index)));
        TSM_ASSERT_EQUALS(name, foundValue, expectedValue);
    }

    void checkFloat(interpreter::World& world, const char* name, double expectedValue)
    {
        afl::data::NameMap::Index_t index = world.globalPropertyNames().getIndexByName(name);
        TSM_ASSERT(name, index != afl::data::NameMap::nil);

        afl::data::FloatValue* fv = dynamic_cast<afl::data::FloatValue*>(world.globalValues().get(index));
        TSM_ASSERT(name, fv != 0);
        if (fv != 0) {
            TSM_ASSERT_EQUALS(name, fv->getValue(), expectedValue);
        }
    }
}

/** Test Set commands. */
void
TestInterpreterFileFunctions::testSet()
{
    // Environment
    afl::sys::Log logger;
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    interpreter::World world(logger, tx, fs);

    world.addNewGlobalContext(new GlobalContextMock(world));
    registerFileFunctions(world);

    // SetWord, GetByte
    checkStatement(world,
                   "dim block\n"
                   "setword block, 3, 12345\n"
                   "a:=getbyte(block, 3)\n"
                   "b:=getbyte(block, 4)\n");
    checkInteger(world, "A", 57);
    checkInteger(world, "B", 48);

    // SetByte, GetWord
    checkStatement(world,
                   "dim block\n"
                   "setbyte block, 100, 57, 48\n"
                   "a:=getword(block, 100)\n");
    checkInteger(world, "A", 12345);
}

/** Test FPos(), FSize() functions. */
void
TestInterpreterFileFunctions::testPositionFunctions()
{
    // Environment
    afl::sys::Log logger;
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    interpreter::World world(logger, tx, fs);

    world.addNewGlobalContext(new GlobalContextMock(world));
    registerFileFunctions(world);

    // Configure files
    afl::base::Ref<StreamMock> stream(*new StreamMock());
    world.fileTable().setMaxFiles(5);
    world.fileTable().openFile(1, stream);

    // Test
    // - program: set A to position, B to size; catch error in B (to simplify testing overflow case)
    const char*const STATEMENT = "a:=fpos(#1)\nb:=7\ntry b:=fsize(#1)\n";

    // - initial state
    checkStatement(world, STATEMENT);
    checkInteger(world, "A", 0);
    checkInteger(world, "B", 0);

    // - average case
    stream->setPos(10000);
    stream->setSize(20000);
    checkStatement(world, STATEMENT);
    checkInteger(world, "A", 10000);
    checkInteger(world, "B", 20000);

    // - 32-bit boundary
    stream->setPos(0x7FFFFFFFU);
    stream->setSize(0x80000000U);
    checkStatement(world, STATEMENT);
    checkInteger(world, "A", 0x7FFFFFFFU);
    checkFloat(world, "B", 2147483648.0);

    // - 53-bit boundary
    stream->setPos(9007199254740992ULL);
    stream->setSize(9007199254740993ULL);
    checkStatement(world, STATEMENT);
    checkFloat(world, "A", 9007199254740992.0);
    checkInteger(world, "B", 7);
}

