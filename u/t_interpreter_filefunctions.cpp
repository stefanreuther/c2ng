/**
  *  \file u/t_interpreter_filefunctions.cpp
  *  \brief Test for interpreter::FileFunctions
  */

#include "interpreter/filefunctions.hpp"

#include "t_interpreter.hpp"
#include "interpreter/world.hpp"
#include "interpreter/memorycommandsource.hpp"
#include "interpreter/bytecodeobject.hpp"
#include "interpreter/statementcompiler.hpp"
#include "interpreter/statementcompilationcontext.hpp"
#include "interpreter/process.hpp"
#include "interpreter/defaultstatementcompilationcontext.hpp"
#include "afl/sys/log.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "interpreter/singlecontext.hpp"
#include "interpreter/values.hpp"
#include "interpreter/arguments.hpp"

namespace {
    class GlobalContextMock : public interpreter::SingleContext {
     public:
        explicit GlobalContextMock(interpreter::World& world)
            : m_world(world)
            { }
        ~GlobalContextMock()
            { }

        // Context:
        virtual GlobalContextMock* lookup(const afl::data::NameQuery& name, PropertyIndex_t& result)
            {
                afl::data::NameMap::Index_t ix = m_world.globalPropertyNames().getIndexByName(name);
                if (ix != afl::data::NameMap::nil) {
                    result = ix;
                    return this;
                } else {
                    return 0;
                }
            }
        virtual void set(PropertyIndex_t index, afl::data::Value* value)
            { m_world.globalValues().set(index, value); }
        virtual afl::data::Value* get(PropertyIndex_t index)
            { return afl::data::Value::cloneOf(m_world.globalValues().get(index)); }
        virtual GlobalContextMock* clone() const
            { return new GlobalContextMock(m_world); }
        virtual game::map::Object* getObject()
            { return 0; }
        virtual void enumProperties(interpreter::PropertyAcceptor& /*acceptor*/)
            { }

        // BaseValue:
        virtual String_t toString(bool /*readable*/) const
            { return "<gcm>"; }
        virtual void store(interpreter::TagNode& /*out*/, afl::io::DataSink& /*aux*/, afl::charset::Charset& /*cs*/, interpreter::SaveContext& /*ctx*/) const
            { throw interpreter::Error::notSerializable(); }

     private:
        interpreter::World& m_world;
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
        scc.withContextProvider(&exec);
        scc.withFlag(scc.LinearExecution);
        scc.withFlag(scc.ExpressionsAreStatements);

        interpreter::BCORef_t bco = *new interpreter::BytecodeObject();
        interpreter::StatementCompiler::StatementResult result = interpreter::StatementCompiler(mcs).compileList(*bco, scc);

        TSM_ASSERT_DIFFERS(stmt, result, interpreter::StatementCompiler::CompiledExpression);

        exec.pushFrame(bco, false);
        bool ok = exec.runTemporary();
        TSM_ASSERT(stmt, ok);
    }

    void checkInteger(interpreter::World& world, const char* name, int32_t expectedValue)
    {
        afl::data::NameMap::Index_t index = world.globalPropertyNames().getIndexByName(name);
        TSM_ASSERT(name, index != afl::data::NameMap::nil);

        int32_t foundValue;
        TSM_ASSERT(name, interpreter::checkIntegerArg(foundValue, world.globalValues().get(index)));
        TSM_ASSERT_EQUALS(name, foundValue, expectedValue);
    }
}

/** Test Set commands. */
void
TestInterpreterFileFunctions::testSet()
{
    // Environment
    afl::sys::Log logger;
    afl::io::NullFileSystem fs;
    interpreter::World world(logger, fs);

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

