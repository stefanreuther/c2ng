/**
  *  \file client/si/widgetwrapper.cpp
  *  \brief Class client::si::WidgetWrapper
  */

#include "client/si/widgetwrapper.hpp"
#include "afl/string/format.hpp"
#include "game/proxy/objectlistener.hpp"
#include "client/si/genericwidgetvalue.hpp"
#include "client/si/scriptside.hpp"
#include "client/si/userside.hpp"
#include "client/si/widgetholder.hpp"
#include "game/interface/planetcontext.hpp"
#include "game/interface/shipcontext.hpp"
#include "game/map/planet.hpp"
#include "game/map/ship.hpp"
#include "client/si/widgetreference.hpp"
#include "game/interface/iteratorcontext.hpp"

namespace {
    // FIXME: if updates happen faster than scripts are executed, this will spam the queue.
    // This should somehow ensure that this queueing does not happen.
    // Idea: make an "active" bit, hook onProcessGroupFinish, do not respawn tasks before onProcessGroupFinish confirms.

    /*
     *  Request to run a process group Id through the ScriptSide.
     *  This must happen separately to break possible callback recursion.
     */
    class Runner : public util::Request<game::Session> {
     public:
        Runner(uint32_t pgid)
            : m_pgid(pgid)
            { }
        virtual void handle(game::Session& session)
            {
                // FIXME: can we log errors if this process fails?
                if (client::si::ScriptSide* ss = session.extra().get(client::si::SCRIPTSIDE_ID)) {
                    session.processList().startProcessGroup(m_pgid);
                    ss->runProcesses(session);
                }
            }
     private:
        const uint32_t m_pgid;
    };

    /*
     *  ObjectListener that triggers a script callback.
     *  Normally, ObjectListener's job is to call back into the UI thread and let that decide what happens.
     *  We can do everything in one callback, though.
     */
    class Listener : public game::proxy::ObjectListener {
     public:
        Listener(const client::si::WidgetReference& ref,
                 util::RequestSender<game::Session> gameSender,
                 afl::base::Memory<const interpreter::NameTable> properties,
                 String_t command)
            : m_ref(ref), m_gameSender(gameSender), m_properties(properties), m_command(command)
            { }

        virtual void handle(game::Session& session, game::map::Object* obj)
            {
                client::si::ScriptSide* ss = session.extra().get(client::si::SCRIPTSIDE_ID);
                if (ss != 0 && !m_command.empty()) {
                    try {
                        // Compile
                        interpreter::BCORef_t bco = session.world().compileCommand(m_command);

                        // Create process
                        interpreter::ProcessList& processList = session.processList();
                        interpreter::Process& proc = processList.create(session.world(), "<Update>");

                        // - object context
                        if (interpreter::Context* ctx = game::interface::createObjectContext(obj, session)) {
                            proc.pushNewContext(ctx);
                        }

                        // - widget context
                        proc.pushNewContext(new client::si::GenericWidgetValue(m_properties, session, ss, m_ref));

                        // Prepare for execution
                        const uint32_t pgid = processList.allocateProcessGroup();
                        proc.pushFrame(bco, false);
                        processList.resumeProcess(proc, pgid);

                        // Run it. Must be started from a different callback in a clean stack frame.
                        m_gameSender.postNewRequest(new Runner(pgid));
                    }
                    catch (std::exception& e) {
                        // Log error
                        if (interpreter::Error* pe = dynamic_cast<interpreter::Error*>(&e)) {
                            session.logError(*pe);
                        } else {
                            session.logError(interpreter::Error(e.what()));
                        }

                        // Do not try again
                        session.log().write(afl::sys::LogListener::Error, "script.error",
                                            afl::string::Format(session.translator().translateString("Disabling update callback \"%s\" due to error").c_str(),
                                                                m_command));
                        m_command.clear();
                    }
                }
            }
     private:
        const client::si::WidgetReference m_ref;
        util::RequestSender<game::Session> m_gameSender;
        const afl::base::Memory<const interpreter::NameTable> m_properties;
        String_t m_command;
    };

}


client::si::WidgetWrapper::WidgetWrapper(client::si::UserSide& user,
                                         std::auto_ptr<ui::Widget> theWidget,
                                         afl::base::Memory<const interpreter::NameTable> properties)
    : m_holder(*new client::si::WidgetHolder(user.userSender())),
      m_slot(m_holder->addNewWidget(user, theWidget.release())),
      m_gameSender(user.gameSender()),
      m_properties(properties)
{
    if (ui::Widget* p = m_holder->get(user, m_slot)) {
        addChild(*p, 0);
    }
}

client::si::WidgetWrapper::~WidgetWrapper()
{ }

void
client::si::WidgetWrapper::draw(gfx::Canvas& can)
{
    defaultDrawChildren(can);
}

void
client::si::WidgetWrapper::handleStateChange(State /*st*/, bool /*enable*/)
{ }

void
client::si::WidgetWrapper::requestChildRedraw(Widget& /*child*/, const gfx::Rectangle& area)
{
    requestRedraw(area);
}

void
client::si::WidgetWrapper::handleChildAdded(Widget& child)
{
    child.setExtent(getExtent());
    requestRedraw();
}

void
client::si::WidgetWrapper::handleChildRemove(Widget& /*child*/)
{
    requestRedraw();
}

void
client::si::WidgetWrapper::handlePositionChange(gfx::Rectangle& /*oldPosition*/)
{
    if (Widget* p = getFirstChild()) {
        p->setExtent(getExtent());
    }
}

void
client::si::WidgetWrapper::handleChildPositionChange(Widget& /*child*/, gfx::Rectangle& /*oldPosition*/)
{
    requestRedraw();
}

ui::layout::Info
client::si::WidgetWrapper::getLayoutInfo() const
{
    if (Widget* p = getFirstChild()) {
        return p->getLayoutInfo();
    } else {
        return ui::layout::Info();
    }
}

bool
client::si::WidgetWrapper::handleKey(util::Key_t key, int prefix)
{
    return defaultHandleKey(key, prefix);
}

bool
client::si::WidgetWrapper::handleMouse(gfx::Point pt, MouseButtons_t pressedButtons)
{
    return defaultHandleMouse(pt, pressedButtons);
}

void
client::si::WidgetWrapper::attach(game::proxy::ObjectObserver& oop, String_t command)
{
    oop.addNewListener(new Listener(WidgetReference(m_holder, m_slot), m_gameSender, m_properties, command));
}
