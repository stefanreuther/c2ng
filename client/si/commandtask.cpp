/**
  *  \file client/si/commandtask.cpp
  */

#include "client/si/commandtask.hpp"
#include "interpreter/statementcompiler.hpp"
#include "interpreter/memorycommandsource.hpp"
#include "interpreter/defaultstatementcompilationcontext.hpp"
#include "interpreter/error.hpp"
#include "interpreter/contextreceiver.hpp"
#include "interpreter/values.hpp"

using interpreter::Process;
using interpreter::ProcessList;
using interpreter::StatementCompiler;

namespace {
    class DefaultFinalizer : public interpreter::Process::Finalizer {
     public:
        DefaultFinalizer(game::Session& session, bool showResult)
            : m_session(session),
              m_showResult(showResult)
            { }
        virtual void finalizeProcess(interpreter::Process& p)
            {
                using afl::sys::LogListener;
                LogListener& log = m_session.log();
                switch (p.getState()) {
                 case interpreter::Process::Suspended:
                    log.write(LogListener::Info, "script.state", m_session.translator().translateString("Suspended."));
                    break;
                 case interpreter::Process::Frozen:
                    log.write(LogListener::Info, "script.state", m_session.translator().translateString("Frozen."));
                    break;
                 case interpreter::Process::Runnable:
                 case interpreter::Process::Running:
                 case interpreter::Process::Waiting:
                    break;
                 case interpreter::Process::Ended:
                    if (m_showResult) {
                        const afl::data::Value* result = p.getResult();
                        if (result == 0) {
                            log.write(LogListener::Info, "script.empty", "Empty");
                        } else {
                            log.write(LogListener::Info, "script.result", interpreter::toString(result, true));
                        }
                    }
                    break;
                 case interpreter::Process::Terminated:
                    // Terminated, i.e. "End" statement. Log only when user specified an expression,
                    // to tell them why they don't get a result.
                    if (m_showResult) {
                        log.write(LogListener::Info, "script.state", m_session.translator().translateString("Terminated."));
                    }
                    break;

                 case interpreter::Process::Failed:
                    // Logged by ProcessList::run.
                    break;
                }
            }
     private:
        game::Session& m_session;
        bool m_showResult;
    };
}

client::si::CommandTask::CommandTask(String_t command, bool verbose, String_t name, std::auto_ptr<game::interface::ContextProvider> ctxp)
    : m_command(command),
      m_verbose(verbose),
      m_name(name),
      m_contextProvider(ctxp)
{ }

void
client::si::CommandTask::execute(uint32_t pgid, game::Session& session)
{
    // Log it
    if (m_verbose) {
        session.log().write(afl::sys::LogListener::Info, "script.input", m_command);
    }

    // Create process
    interpreter::ProcessList& processList = session.processList();
    interpreter::Process& proc = processList.create(session.world(), m_name);

    // Create BCO
    interpreter::BCORef_t bco = interpreter::BytecodeObject::create(true);
    if (m_contextProvider.get() != 0) {
        m_contextProvider->createContext(session, proc);
        proc.markContextTOS();
    }

    // Create compilation context
    interpreter::MemoryCommandSource mcs(m_command);
    interpreter::DefaultStatementCompilationContext scc(session.world());
    scc.withStaticContext(&proc);
    scc.withFlag(scc.RefuseBlocks);
    scc.withFlag(scc.LinearExecution);
    if (!m_verbose) {
        scc.withFlag(scc.ExpressionsAreStatements);
    }

    // Compile
    try {
        StatementCompiler sc(mcs);
        StatementCompiler::Result result = sc.compile(*bco, scc);
        sc.finishBCO(*bco, scc);

        // Build process and add to process group
        proc.pushFrame(bco, false);
        processList.resumeProcess(proc, pgid);

        if (m_verbose) {
            proc.setNewFinalizer(new DefaultFinalizer(session, result == StatementCompiler::CompiledExpression));
        }
    }
    catch (std::exception& e) {
        if (interpreter::Error* pe = dynamic_cast<interpreter::Error*>(&e)) {
            session.logError(*pe);
        } else {
            session.logError(interpreter::Error(e.what()));
        }

        // Immediately fail it
        proc.setState(Process::Failed);
    }
}
