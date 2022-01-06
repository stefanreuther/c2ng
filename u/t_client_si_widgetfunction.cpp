/**
  *  \file u/t_client_si_widgetfunction.cpp
  *  \brief Test for client::si::WidgetFunction
  */

#include "client/si/widgetfunction.hpp"

#include "t_client_si.hpp"
#include "afl/data/segment.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/semaphore.hpp"
#include "client/si/control.hpp"
#include "client/si/scriptside.hpp"
#include "client/si/userside.hpp"
#include "client/si/widgetholder.hpp"
#include "client/si/widgetreference.hpp"
#include "gfx/nullengine.hpp"
#include "gfx/nullresourceprovider.hpp"
#include "interpreter/test/contextverifier.hpp"
#include "interpreter/values.hpp"
#include "ui/group.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/root.hpp"
#include "util/messagecollector.hpp"
#include "util/requestthread.hpp"

namespace {
    using interpreter::test::ContextVerifier;

    class NullControl : public client::si::Control {
     public:
        NullControl(client::si::UserSide& iface)
            : Control(iface)
            { }
        virtual void handleStateChange(client::si::RequestLink2 link, client::si::OutputState::Target /*target*/)
            { interface().continueProcessWithFailure(link, "doesn't work"); }
        virtual void handleEndDialog(client::si::RequestLink2 link, int /*code*/)
            { interface().continueProcess(link); }
        virtual void handlePopupConsole(client::si::RequestLink2 link)
            { interface().continueProcess(link); }
        virtual void handleScanKeyboardMode(client::si::RequestLink2 link)
            { interface().continueProcessWithFailure(link, "Context error"); }
        virtual void handleSetView(client::si::RequestLink2 link, String_t /*name*/, bool /*withKeymap*/)
            { interface().continueProcessWithFailure(link, "Context error"); }
        virtual void handleUseKeymap(client::si::RequestLink2 link, String_t /*name*/, int /*prefix*/)
            { interface().continueProcessWithFailure(link, "Context error"); }
        virtual void handleOverlayMessage(client::si::RequestLink2 link, String_t /*text*/)
            { interface().continueProcessWithFailure(link, "Context error"); }
        virtual client::si::ContextProvider* createContextProvider()
            { return 0; }
    };

    class WidgetVerifier {
     public:
        virtual afl::data::Value* create(game::Session& session, client::si::ScriptSide& ss, const client::si::WidgetReference& ref) = 0;
        virtual void verify(afl::data::Value* value) = 0;

        void run();
    };
}

void
WidgetVerifier::run()
{
    // Infrastructure
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    afl::sys::Log log;

    // Session (must be first, d'ooh!)
    game::Session session(tx, fs);

    // Start two worker threads
    util::RequestThread gameThread("testNewButton:game", log, tx);
    util::RequestThread userThread("testNewButton:user", log, tx);

    // Session
    util::RequestReceiver<game::Session> gameReceiver(gameThread, session);
    
    // Now everything has been set up. Do the test.
    {
        // GUI mock
        gfx::NullEngine engine;
        gfx::NullResourceProvider provider;
        ui::Root root(engine, provider, gfx::WindowParameters());

        // ScriptSide/UserSide/Control. A Control is needed because UserSide does not process callbacks without one.
        util::MessageCollector console;
        client::si::UserSide user(root, gameReceiver.getSender(), tx, userThread, console, log);
        NullControl ctl(user);

        // WidgetReference. We're operating in lock-step, so it doesn't matter that I cheat on the thread that creates it.
        afl::base::Ref<client::si::WidgetHolder> h(*new client::si::WidgetHolder(user.userSender()));
        size_t parentId = h->addNewWidget(ctl, new ui::Group(ui::layout::HBox::instance0));

        // Call function on script side. Must be done using a ScriptRequest because this is the only way to access the ScriptSide.
        afl::sys::Semaphore sem(0);
        std::auto_ptr<afl::data::Value> result;
        class Actor : public client::si::UserSide::ScriptRequest {
         public:
            Actor(WidgetVerifier& parent,
                  afl::sys::Semaphore& sem,
                  std::auto_ptr<afl::data::Value>& result,
                  const client::si::WidgetReference& ref)
                : m_parent(parent), m_semaphore(sem), m_result(result), m_ref(ref)
                { }
            void handle(client::si::ScriptSide& ss)
                {
                    m_result.reset(m_parent.create(ss.session(), ss, m_ref));
                    m_semaphore.post();
                }
         private:
            WidgetVerifier& m_parent;
            afl::sys::Semaphore& m_semaphore;
            std::auto_ptr<afl::data::Value>& m_result;
            const client::si::WidgetReference m_ref;
        };

        user.postNewRequest(new Actor(*this, sem, result, client::si::WidgetReference(h, parentId)));
        sem.wait();

        // Examine result
        verify(result.get());
    }

    // Destruction of objects like UserSide will still post events into the threads.
    // Make sure these are all executed before we finish.
    afl::sys::Semaphore semFinish(0);
    class Finisher : public afl::base::Runnable {
     public:
        Finisher(afl::sys::Semaphore& sem)
            : m_semaphore(sem)
            { }
        virtual void run()
            { m_semaphore.post(); }
     private:
        afl::sys::Semaphore& m_semaphore;
    };
    gameThread.postNewRunnable(new Finisher(semFinish));
    userThread.postNewRunnable(new Finisher(semFinish));
    semFinish.wait();
    semFinish.wait();
}

/** Test "NewButton" function. */
void
TestClientSiWidgetFunction::testNewButton()
{
    class ButtonVerifier : public WidgetVerifier {
     public:
        virtual afl::data::Value* create(game::Session& session, client::si::ScriptSide& ss, const client::si::WidgetReference& ref)
            {
                // Arguments
                afl::data::Segment argSegment;
                argSegment.pushBackNew(interpreter::makeStringValue("OK"));
                argSegment.pushBackNew(interpreter::makeStringValue("ret"));
                argSegment.pushBackNew(interpreter::makeStringValue("UI.EndDialog"));
                interpreter::Arguments args(argSegment, 0, 3);

                return client::si::IFWidgetNewButton(session, ss, ref, args);
            }
        virtual void verify(afl::data::Value* value)
            {
                // Examine result
                TS_ASSERT(value != 0);

                interpreter::Context* ctx = dynamic_cast<interpreter::Context*>(value);
                TS_ASSERT(ctx != 0);
                ContextVerifier t(*ctx, "testNewButton");
                t.verifyTypes();
                t.verifyBoolean("ENABLED", true);
            }
    };
    ButtonVerifier().run();
}

/** Test "NewInput" function. */
void
TestClientSiWidgetFunction::testNewInput()
{
    class InputVerifier : public WidgetVerifier {
     public:
        virtual afl::data::Value* create(game::Session& session, client::si::ScriptSide& ss, const client::si::WidgetReference& ref)
            {
                // Arguments
                afl::data::Segment argSegment;
                interpreter::Arguments args(argSegment, 0, 0);
                return client::si::IFWidgetNewInput(session, ss, ref, args);
            }
        virtual void verify(afl::data::Value* value)
            {
                // Examine result
                TS_ASSERT(value != 0);

                interpreter::Context* ctx = dynamic_cast<interpreter::Context*>(value);
                TS_ASSERT(ctx != 0);
                ContextVerifier t(*ctx, "testNewInput");
                t.verifyTypes();
                t.verifyBoolean("ENABLED", true);
                t.verifyString("VALUE", "");
            }
    };
    InputVerifier().run();
}

