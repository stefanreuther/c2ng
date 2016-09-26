/**
  *  \file client/widgets/keymapwidget.cpp
  */

#include "client/widgets/keymapwidget.hpp"




client::widgets::KeymapWidget::KeymapWidget(util::RequestSender<game::Session> gameSender,
                                            util::RequestDispatcher& self,
                                            client::si::Control& ctl)
    : m_reply(self, *this),
      m_control(ctl),
      m_keys(),
      m_keymapName(),
      m_slave(gameSender, new Trampoline(m_reply.getSender()))
{ }

client::widgets::KeymapWidget::~KeymapWidget()
{ }

void
client::widgets::KeymapWidget::setKeymapName(String_t keymap)
{
    class SetKeymapNameTask : public util::SlaveRequest<game::Session, Trampoline> {
     public:
        SetKeymapNameTask(String_t name)
            : m_name(name)
            { }
        void handle(game::Session& s, Trampoline& t)
            { t.setKeymapName(s, m_name); }
     private:
        String_t m_name;
    };
    if (keymap != m_keymapName) {
        m_keymapName = keymap;
        m_keys.clear();
        m_slave.postNewRequest(new SetKeymapNameTask(m_keymapName));
    }
}

bool
client::widgets::KeymapWidget::handleKey(util::Key_t key, int prefix)
{
    if (m_keys.find(key) != m_keys.end()) {
        m_control.executeKeyCommandWait(m_keymapName, key, prefix);
        return true;
    } else {
        return false;
    }
}

bool
client::widgets::KeymapWidget::handleMouse(gfx::Point /*pt*/, MouseButtons_t /*pressedButtons*/)
{
    return false;
}

afl::base::Closure<void(int, util::Key_t)>*
client::widgets::KeymapWidget::makeKey()
{
    class Handler : public afl::base::Closure<void(int, util::Key_t)> {
     public:
        Handler(KeymapWidget& self)
            : m_self(self)
            { }
        void call(int prefix, util::Key_t key)
            { m_self.handleKey(key, prefix); }
        Handler* clone() const
            { return new Handler(*this); }
     private:
        KeymapWidget& m_self;
    };
    return new Handler(*this);
}

void
client::widgets::KeymapWidget::addButton(ui::widgets::AbstractButton& btn)
{
    btn.sig_fireKey.addNewClosure(makeKey());
}

void
client::widgets::KeymapWidget::Trampoline::init(game::Session& s)
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
client::widgets::KeymapWidget::Trampoline::done(game::Session& /*s*/)
{
    conn_keymapChange.disconnect();
}

void
client::widgets::KeymapWidget::Trampoline::setKeymapName(game::Session& s, String_t keymapName)
{
    m_keymapName = keymapName;
    update(s);
}

void
client::widgets::KeymapWidget::Trampoline::update(game::Session& s)
{
    class UpdateKeySetTask : public util::Request<client::widgets::KeymapWidget> {
     public:
        UpdateKeySetTask(util::KeymapRef_t p)
            : m_set()
            {
                if (p != 0) {
                    p->enumKeys(m_set);
                }
            }
        void handle(KeymapWidget& w)
            { w.m_keys.swap(m_set); }
     private:
        util::KeySet_t m_set;
    };
    m_reply.postNewRequest(new UpdateKeySetTask(s.world().keymaps().getKeymapByName(m_keymapName)));
}
