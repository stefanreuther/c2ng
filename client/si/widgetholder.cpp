/**
  *  \file client/si/widgetholder.cpp
  *  \brief Class client::si::WidgetHolder
  */

#include "client/si/widgetholder.hpp"
#include "client/si/scriptside.hpp"
#include "client/si/control.hpp"
#include "afl/string/format.hpp"
#include "afl/base/weaklink.hpp"
#include "util/requestsender.hpp"
#include "util/requestreceiver.hpp"

namespace {
    class Integer : public afl::base::Deletable, public afl::base::Observable<int> {
    };
}

class client::si::WidgetHolder::Impl {
 public:
    Impl()
        : m_widgets(),
          m_deleter()
        { }
    ~Impl()
        {
            // Clear m_widgets.
            // This way, get() will be guaranteed to return null during the destruction of m_deleter.
            // (Such calls should not happen, though.)
            m_widgets.clear();
        }
    afl::base::Deleter& deleter()
        {
            return m_deleter;
        }
    size_t addNewWidget(ui::Widget* w)
        {
            m_deleter.addNew(w);
            m_widgets.push_back(w);
            return m_widgets.size()-1;
        }
    ui::Widget* get(size_t n) const
        {
            return n < m_widgets.size() ? m_widgets[n] : 0;
        }
 private:
    std::vector<ui::Widget*> m_widgets;
    afl::base::Deleter m_deleter;
};

// Constructor.
client::si::WidgetHolder::WidgetHolder(util::RequestSender<UserSide> userSender)
    // We normally must only access the Impl object only from the user-interface thread.
    // However, as we're only creating empty containers, 'new Impl()' here is fine.
    : m_impl(new Impl()),
      m_userSender(userSender),
      m_pControl(0)
{ }

// Destructor.
client::si::WidgetHolder::~WidgetHolder()
{
    // The Impl object contains user-interface objects that must be destroyed within the user-interface thread.
    // We therefore post the DeleterTask into that thread.
    class DeleterTask : public util::Request<UserSide> {
     public:
        DeleterTask(std::auto_ptr<Impl> impl)
            : m_impl(impl)
            { }
        void handle(UserSide& /*ui*/)
            { m_impl.reset(); }
     private:
        std::auto_ptr<Impl> m_impl;
    };
    m_userSender.postNewRequest(new DeleterTask(m_impl));
}

// Add new widget.
size_t
client::si::WidgetHolder::addNewWidget(UserSide& /*user*/, ui::Widget* w)
{
    return m_impl->addNewWidget(w);
}

size_t
client::si::WidgetHolder::addNewWidget(Control& /*ctl*/, ui::Widget* w)
{
    return m_impl->addNewWidget(w);
}

// Get widget.
ui::Widget*
client::si::WidgetHolder::get(UserSide& /*user*/, size_t n) const
{
    return m_impl->get(n);
}

ui::Widget*
client::si::WidgetHolder::get(Control& /*ctl*/, size_t n) const
{
    return m_impl->get(n);
}

// Get deleter.
afl::base::Deleter&
client::si::WidgetHolder::deleter(Control& /*ctl*/)
{
    return m_impl->deleter();
}

// Create integer value.
afl::base::Observable<int>&
client::si::WidgetHolder::createInteger(Control& ctl)
{
    return deleter(ctl).addNew(new Integer());
}

// Attach Control.
bool
client::si::WidgetHolder::attachControl(Control& ctl)
{
    if (m_pControl != 0) {
        return false;
    } else {
        m_pControl = &ctl;
        return true;
    }
}

// Detach Control.
void
client::si::WidgetHolder::detachControl(Control& ctl)
{
    if (m_pControl == &ctl) {
        m_pControl = 0;
    }
}

// Get attached Control.
client::si::Control*
client::si::WidgetHolder::getControl()
{
    return m_pControl;
}

// Make command event.
afl::base::Closure<void(int)>*
client::si::WidgetHolder::makeCommand(util::Atom_t cmd)
{
    class Command : public afl::base::Closure<void(int)> {
     public:
        Command(WidgetHolder& holder, util::Atom_t cmd)
            : m_holder(&holder),
              m_command(cmd)
            { }
        void call(int prefix)
            {
                if (WidgetHolder* p = m_holder.get()) {
                    if (Control* ctl = p->getControl()) {
                        ctl->executeCommandWait(afl::string::Format("C2$Eval %d, %d, ''", m_command, prefix),
                                                false,
                                                ctl->translator().translateString("Event Callback"));
                    }
                }
            }
     private:
        // The resulting command will be part of the WidgetHolder.
        // We therefore cannot use a Ref here which would prevent the WidgetHolder from being deleted.
        afl::base::WeakLink<WidgetHolder> m_holder;
        util::Atom_t m_command;
    };
    return new Command(*this, cmd);
}
