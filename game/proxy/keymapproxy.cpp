/**
  *  \file game/proxy/keymapproxy.cpp
  *  \class game::proxy::KeymapProxy
  */

#include "game/proxy/keymapproxy.hpp"
#include "interpreter/subroutinevalue.hpp"
#include "interpreter/tokenizer.hpp"
#include "util/keymapinformation.hpp"

namespace {
    const size_t MAX_DEPTH = 5;
}


class game::proxy::KeymapProxy::Trampoline {
 public:
    Trampoline(Session& session, util::RequestSender<KeymapProxy> reply);
    void init(Session& session);
    void setKeymapName(String_t keymapName);
    util::KeymapRef_t getKeymap();
    void update();
    Session& session()
        { return m_session; }
 private:
    Session& m_session;
    afl::base::SignalConnection conn_keymapChange;
    util::RequestSender<KeymapProxy> m_reply;
    String_t m_keymapName;
};


class game::proxy::KeymapProxy::TrampolineFromSession : public afl::base::Closure<Trampoline*(Session&)> {
 public:
    TrampolineFromSession(const util::RequestSender<KeymapProxy>& reply)
        : m_reply(reply)
        { }
    virtual Trampoline* call(Session& session)
        { return new Trampoline(session, m_reply); }
 private:
    util::RequestSender<KeymapProxy> m_reply;
};



// Constructor.
game::proxy::KeymapProxy::KeymapProxy(util::RequestSender<Session> gameSender, util::RequestDispatcher& reply)
    : m_reply(reply, *this),
      m_sender(gameSender.makeTemporary(new TrampolineFromSession(m_reply.getSender()))),
      m_pListener(0)
{ }

// Destructor.
game::proxy::KeymapProxy::~KeymapProxy()
{ }

// Set listener for asynchronous keymap population updates.
void
game::proxy::KeymapProxy::setListener(Listener& listener)
{
    m_pListener = &listener;
}

// Set keymap name.
void
game::proxy::KeymapProxy::setKeymapName(String_t keymap)
{
    class Task : public util::Request<Trampoline> {
     public:
        Task(String_t name)
            : m_name(name)
            { }
        void handle(Trampoline& t)
            { t.setKeymapName(m_name); }
     private:
        String_t m_name;
    };

    m_sender.postNewRequest(new Task(keymap));
}

// Get description of the current keymap.
void
game::proxy::KeymapProxy::getDescription(WaitIndicator& link, util::KeymapInformation& out)
{
    class Task : public util::Request<Trampoline> {
     public:
        Task(util::KeymapInformation& out)
            : m_out(out)
            { }
        void handle(Trampoline& t)
            {
                m_out.clear();
                if (util::KeymapRef_t p = t.getKeymap()) {
                    p->describe(m_out, MAX_DEPTH);
                }
            }
     private:
        util::KeymapInformation& m_out;
    };

    Task t(out);
    link.call(m_sender, t);
}

// Get description of a key.
void
game::proxy::KeymapProxy::getKey(WaitIndicator& link, util::Key_t key, Info& result)
{
    class Task : public util::Request<Trampoline> {
     public:
        Task(util::Key_t key, Info& result)
            : m_key(key),
              m_result(result)
            { }
        void handle(Trampoline& t)
            {
                // Start unassigned
                m_result.result = Unassigned;
                m_result.command.clear();
                m_result.alternateKeymapName.clear();
                m_result.origin.clear();

                // Look up
                if (util::KeymapRef_t p = t.getKeymap()) {
                    util::KeymapRef_t keymap = 0;
                    util::Atom_t a = p->lookupCommand(m_key, keymap);
                    if (keymap != 0) {
                        m_result.command = t.session().world().atomTable().getStringFromAtom(a);
                        m_result.keymapName = keymap->getName();
                        m_result.result = (m_result.command.empty() ? a == 0 ? Cancelled : Internal : Normal);

                        interpreter::Tokenizer tok(m_result.command);
                        if (tok.getCurrentToken() == tok.tIdentifier) {
                            const String_t verb = tok.getCurrentString();

                            // Subroutine
                            if (const interpreter::SubroutineValue* sv = dynamic_cast<const interpreter::SubroutineValue*>(t.session().world().getGlobalValue(verb.c_str()))) {
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
    link.call(m_sender, t);
}

/*
 *  Trampoline
 */

game::proxy::KeymapProxy::Trampoline::Trampoline(Session& session, util::RequestSender<KeymapProxy> reply)
    : m_session(session),
      conn_keymapChange(),
      m_reply(reply),
      m_keymapName()
{
    // Attach to keymap changes.
    // If a script modifies the keymap, we must update our view to make the new key usable.
    class Handler : public afl::base::Closure<void()> {
     public:
        Handler(Trampoline& t)
            : m_trampoline(t)
            { }
        void call()
            { m_trampoline.update(); }
     private:
        Trampoline& m_trampoline;
    };
    conn_keymapChange = session.world().keymaps().sig_keymapChange.addNewClosure(new Handler(*this));
}

void
game::proxy::KeymapProxy::Trampoline::setKeymapName(String_t keymapName)
{
    m_keymapName = keymapName;
    update();
}

util::KeymapRef_t
game::proxy::KeymapProxy::Trampoline::getKeymap()
{
    return m_session.world().keymaps().getKeymapByName(m_keymapName);
}

void
game::proxy::KeymapProxy::Trampoline::update()
{
    class UpdateKeySetTask : public util::Request<KeymapProxy> {
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
    m_reply.postNewRequest(new UpdateKeySetTask(m_session.world().keymaps().getKeymapByName(m_keymapName)));
}
