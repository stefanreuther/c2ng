/**
  *  \file client/proxy/keymapproxy.cpp
  *  \brief Class client::proxy::KeymapProxy
  */

#include "client/proxy/keymapproxy.hpp"
#include "interpreter/subroutinevalue.hpp"
#include "interpreter/tokenizer.hpp"
#include "util/keymapinformation.hpp"

namespace {
    const size_t MAX_DEPTH = 5;
}

// Constructor.
client::proxy::KeymapProxy::KeymapProxy(util::RequestDispatcher& reply, util::RequestSender<game::Session> gameSender)
    : m_reply(reply, *this),
      m_slave(gameSender, new Trampoline(m_reply.getSender())),
      m_pListener(0)
{ }

// Destructor.
client::proxy::KeymapProxy::~KeymapProxy()
{ }

// Set listener for asynchronous keymap population updates.
void
client::proxy::KeymapProxy::setListener(Listener& listener)
{
    m_pListener = &listener;
}

// Set keymap name.
void
client::proxy::KeymapProxy::setKeymapName(String_t keymap)
{
    class Task : public util::SlaveRequest<game::Session, Trampoline> {
     public:
        Task(String_t name)
            : m_name(name)
            { }
        void handle(game::Session& s, Trampoline& t)
            { t.setKeymapName(s, m_name); }
     private:
        String_t m_name;
    };

    m_slave.postNewRequest(new Task(keymap));
}

// Get description of the current keymap.
void
client::proxy::KeymapProxy::getDescription(Downlink& link, util::KeymapInformation& out)
{
    class Task : public util::SlaveRequest<game::Session, Trampoline> {
     public:
        Task(util::KeymapInformation& out)
            : m_out(out)
            { }
        void handle(game::Session& s, Trampoline& t)
            {
                m_out.clear();
                if (util::KeymapRef_t p = t.getKeymap(s)) {
                    p->describe(m_out, MAX_DEPTH);
                }
            }
     private:
        util::KeymapInformation& m_out;
    };

    Task t(out);
    link.call(m_slave, t);
}

// Get description of a key.
void
client::proxy::KeymapProxy::getKey(Downlink& link, util::Key_t key, Info& result)
{
    class Task : public util::SlaveRequest<game::Session, Trampoline> {
     public:
        Task(util::Key_t key, Info& result)
            : m_key(key),
              m_result(result)
            { }
        void handle(game::Session& s, Trampoline& t)
            {
                // Start unassigned
                m_result.result = Unassigned;
                m_result.command.clear();
                m_result.alternateKeymapName.clear();
                m_result.origin.clear();

                // Look up
                if (util::KeymapRef_t p = t.getKeymap(s)) {
                    util::KeymapRef_t keymap = 0;
                    util::Atom_t a = p->lookupCommand(m_key, keymap);
                    if (keymap != 0) {
                        m_result.command = s.world().atomTable().getStringFromAtom(a);
                        m_result.keymapName = keymap->getName();
                        m_result.result = (m_result.command.empty() ? a == 0 ? Cancelled : Internal : Normal);

                        interpreter::Tokenizer tok(m_result.command);
                        if (tok.getCurrentToken() == tok.tIdentifier) {
                            const String_t verb = tok.getCurrentString();

                            // Subroutine
                            if (interpreter::SubroutineValue* sv = dynamic_cast<interpreter::SubroutineValue*>(s.world().getGlobalValue(verb.c_str()))) {
                                m_result.origin = sv->getBytecodeObject()->getOrigin();
                            }

                            // Keymap
                            if (verb == "USEKEYMAP") {
                                if (tok.readNextToken() == tok.tIdentifier) {
                                    m_result.alternateKeymapName = tok.getCurrentString();
                                }
                            }
                        }
                    }
                }
            }
     private:
        util::Key_t m_key;
        Info& m_result;
    };
    Task t(key, result);
    link.call(m_slave, t);
}

/*
 *  Trampoline
 */

void
client::proxy::KeymapProxy::Trampoline::init(game::Session& s)
{
    // Attach to keymap changes.
    // If a script modifies the keymap, we must update our view to make the new key usable.
    class Handler : public afl::base::Closure<void()> {
     public:
        Handler(game::Session& s, Trampoline& t)
            : m_session(s),
              m_trampoline(t)
            { }
        void call()
            { m_trampoline.update(m_session); }
        Handler* clone() const
            { return new Handler(*this); }
     private:
        game::Session& m_session;
        Trampoline& m_trampoline;
    };
    conn_keymapChange = s.world().keymaps().sig_keymapChange.addNewClosure(new Handler(s, *this));
}

void
client::proxy::KeymapProxy::Trampoline::done(game::Session& /*s*/)
{
    conn_keymapChange.disconnect();
}

void
client::proxy::KeymapProxy::Trampoline::setKeymapName(game::Session& s, String_t keymapName)
{
    m_keymapName = keymapName;
    update(s);
}

util::KeymapRef_t
client::proxy::KeymapProxy::Trampoline::getKeymap(game::Session& s)
{
    return s.world().keymaps().getKeymapByName(m_keymapName);
}

void
client::proxy::KeymapProxy::Trampoline::update(game::Session& s)
{
    class UpdateKeySetTask : public util::Request<client::proxy::KeymapProxy> {
     public:
        UpdateKeySetTask(util::KeymapRef_t p)
            : m_set()
            {
                if (p != 0) {
                    p->enumKeys(m_set);
                }
            }
        void handle(KeymapProxy& w)
            {
                if (w.m_pListener != 0) {
                    w.m_pListener->updateKeyList(m_set);
                }
            }
     private:
        util::KeySet_t m_set;
    };
    m_reply.postNewRequest(new UpdateKeySetTask(s.world().keymaps().getKeymapByName(m_keymapName)));
}
