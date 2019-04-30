/**
  *  \file client/si/commandtask.cpp
  */

#include "client/si/commandtask.hpp"
#include "interpreter/statementcompiler.hpp"
#include "interpreter/memorycommandsource.hpp"
#include "interpreter/defaultstatementcompilationcontext.hpp"
#include "interpreter/error.hpp"
#include "client/si/contextreceiver.hpp"

using interpreter::Process;
using interpreter::ProcessList;
using interpreter::StatementCompiler;

client::si::CommandTask::CommandTask(String_t command, bool verbose, String_t name, std::auto_ptr<ContextProvider> ctxp)
    : m_command(command),
      m_verbose(verbose),
      m_name(name),
      m_contextProvider(ctxp)
{ }

interpreter::Process*
client::si::CommandTask::execute(uint32_t pgid, game::Session& session, Verbosity& v)
{
    // Helper to push contexts into a process
    class ProcessContextReceiver : public ContextReceiver {
     public:
        ProcessContextReceiver(Process& proc)
            : m_process(proc)
            { }
        virtual void addNewContext(interpreter::Context* pContext)
            { m_process.pushNewContext(pContext); }
     private:
        Process& m_process;
    };

    // Log it
    if (m_verbose) {
        session.log().write(afl::sys::LogListener::Info, "script.input", m_command);
    }

    // Create process
    interpreter::ProcessList& processList = session.world().processList();
    interpreter::Process& proc = processList.create(session.world(), m_name);

    // Create BCO
    interpreter::BCORef_t bco = *new interpreter::BytecodeObject();
    if (m_contextProvider.get() != 0) {
        ProcessContextReceiver recv(proc);
        m_contextProvider->createContext(session, recv);
    }

    // Create compilation context
    interpreter::MemoryCommandSource mcs(m_command);
    interpreter::DefaultStatementCompilationContext scc(session.world());
    scc.withContextProvider(&proc);
    scc.withFlag(scc.RefuseBlocks);
    scc.withFlag(scc.LinearExecution);
    if (!m_verbose) {
        scc.withFlag(scc.ExpressionsAreStatements);
    }

    // Compile
    try {
        StatementCompiler sc(mcs);
        StatementCompiler::StatementResult result = sc.compile(*bco, scc);
        sc.finishBCO(*bco, scc);

        // Build process and add to process group
        proc.pushFrame(bco, false);
        processList.resumeProcess(proc, pgid);

        v = m_verbose ? result == StatementCompiler::CompiledExpression ? ScriptTask::Result : ScriptTask::Verbose : ScriptTask::Default;
        return &proc;
    }
    catch (std::exception& e) {
        if (interpreter::Error* pe = dynamic_cast<interpreter::Error*>(&e)) {
            session.logError(*pe);
        } else {
            session.logError(interpreter::Error(e.what()));
        }

        // Immediately fail it
        proc.setState(Process::Failed);
        return 0;
    }
}
