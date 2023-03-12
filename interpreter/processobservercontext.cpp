/**
  *  \file interpreter/processobservercontext.cpp
  *  \brief Class interpreter::ProcessObserverContext
  */

#include "interpreter/processobservercontext.hpp"
#include "interpreter/process.hpp"

/** State.
    We will hook a signal.
    To avoid that we have to manage this in each copy of ProcessObserverContext,
    we put it into a state object shared between all clones. */
class interpreter::ProcessObserverContext::State : public afl::base::RefCounted {
 public:
    State(Process& proc)
        : m_process(&proc),
          conn_invalidate(proc.sig_invalidate.add(this, &State::onProcessInvalidate))
        { }
    ~State()
        {
            // This destructor will disconnect the SignalConnection.
            // This is only required if the containing ProcessObserverContext has not been pushed on a Process' stack;
            // otherwise it will be disconnected when the ProcessObserverContext is removed from the stack (onContextLeft).
            // TL;DR: destructor not required for Java{,Script} version.
        }
    Process* getProcess()
        { return m_process; }

    void onProcessInvalidate()
        {
            if (m_process != 0) {
                conn_invalidate.disconnect();
                m_process = 0;
            }
        }

 private:
    Process* m_process;
    afl::base::SignalConnection conn_invalidate;
};



/************************* ProcessObserverContext ************************/

interpreter::ProcessObserverContext::ProcessObserverContext(afl::base::Ref<State> state)
    : m_state(state)
{ }

interpreter::ProcessObserverContext::~ProcessObserverContext()
{ }

interpreter::ProcessObserverContext*
interpreter::ProcessObserverContext::create(Process& p)
{
    return new ProcessObserverContext(*new State(p));
}

interpreter::Context::PropertyAccessor*
interpreter::ProcessObserverContext::lookup(const afl::data::NameQuery& name, PropertyIndex_t& result)
{
    if (Process* p = m_state->getProcess()) {
        return p->lookup(name, result);
    } else {
        return 0;
    }
}

bool
interpreter::ProcessObserverContext::next()
{
    return false;
}

interpreter::ProcessObserverContext*
interpreter::ProcessObserverContext::clone() const
{
    return new ProcessObserverContext(m_state);
}

afl::base::Deletable*
interpreter::ProcessObserverContext::getObject()
{
    return 0;
}

void
interpreter::ProcessObserverContext::enumProperties(PropertyAcceptor& /*acceptor*/) const
{
    // We cannot enumerate properties
}

void
interpreter::ProcessObserverContext::onContextEntered(Process& /*proc*/)
{
    // Ignore; signal is connected upon creation
}

void
interpreter::ProcessObserverContext::onContextLeft()
{
    // Disconnect signal.
    // This means we lose contact once the first clone of ProcessObserverContext is removed from the stack.
    // As of 20230312, we are not creating any clones during normal operation, so this is fine.
    // Otherwise, we'd have to track reference counts somehow.
    m_state->onProcessInvalidate();
}

String_t
interpreter::ProcessObserverContext::toString(bool /*readable*/) const
{
    return "#<process>";
}

void
interpreter::ProcessObserverContext::store(TagNode& out, afl::io::DataSink& aux, SaveContext& ctx) const
{
    rejectStore(out, aux, ctx);
}
