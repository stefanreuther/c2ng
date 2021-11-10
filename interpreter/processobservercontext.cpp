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
        { }
    Process* getProcess()
        { return m_process; }

 private:
    void onProcessInvalidate()
        {
            if (m_process != 0) {
                conn_invalidate.disconnect();
                m_process = 0;
            }
        }

    Process* m_process;
    afl::base::SignalConnection conn_invalidate;
};



/************************* ProcessObserverContext ************************/

interpreter::ProcessObserverContext::ProcessObserverContext(afl::base::Ref<State> state)
    : m_state(state)
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

interpreter::ProcessObserverContext*
interpreter::ProcessObserverContext::clone() const
{
    return new ProcessObserverContext(m_state);
}

game::map::Object*
interpreter::ProcessObserverContext::getObject()
{
    return 0;
}

void
interpreter::ProcessObserverContext::enumProperties(PropertyAcceptor& /*acceptor*/)
{
    // We cannot enumerate properties
}

String_t
interpreter::ProcessObserverContext::toString(bool /*readable*/) const
{
    return "#<process>";
}

void
interpreter::ProcessObserverContext::store(TagNode& /*out*/, afl::io::DataSink& /*aux*/, SaveContext& /*ctx*/) const
{
    throw interpreter::Error::notSerializable();
}
