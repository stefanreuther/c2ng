/**
  *  \file client/si/widgetcommand.cpp
  */

#include "client/si/widgetcommand.hpp"
#include "afl/data/booleanvalue.hpp"
#include "client/si/contextprovider.hpp"
#include "client/si/control.hpp"
#include "client/si/requestlink2.hpp"
#include "client/si/scriptside.hpp"
#include "client/si/usercall.hpp"
#include "client/si/userside.hpp"
#include "client/si/usertask.hpp"
#include "client/si/widgetproperty.hpp"
#include "client/si/widgetreference.hpp"
#include "client/si/widgetvalue.hpp"
#include "client/widgets/controlscreenheader.hpp"
#include "interpreter/processobservercontext.hpp"
#include "ui/layoutablegroup.hpp"
#include "ui/widgets/focusiterator.hpp"
#include "client/si/widgetholder.hpp"
#include "client/si/widgetextraproperty.hpp"
#include "client/si/widgetindexedproperty.hpp"
#include "ui/widgets/stringlistbox.hpp"
#include "client/si/stringlistdialogwidget.hpp"
#include "interpreter/values.hpp"
#include "client/si/contextreceiver.hpp"

namespace {
    void setBooleanProperty(client::si::WidgetProperty p,
                            bool enable,
                            client::si::ScriptSide& ss,
                            const client::si::WidgetReference& ref)
    {
        afl::data::BooleanValue value(enable);
        setWidgetProperty(p, &value, ss, ref);
    }

    void setOptionalBooleanProperty(client::si::WidgetProperty p,
                                    client::si::ScriptSide& ss,
                                    const client::si::WidgetReference& ref,
                                    interpreter::Arguments& args)
    {
        args.checkArgumentCount(0, 1);

        bool enable;
        if (args.getNumArgs() == 0) {
            enable = true;
        } else {
            if (!interpreter::checkBooleanArg(enable, args.getNext())) {
                return;
            }
        }

        setBooleanProperty(p, enable, ss, ref);
    }

    void setIndexedWidgetProperty(client::si::WidgetIndexedProperty wip,
                                  interpreter::Arguments& args,
                                  client::si::ScriptSide& ss,
                                  const client::si::WidgetReference& ref)
    {
        args.checkArgumentCount(2);

        afl::data::Value* index = args.getNext();
        afl::data::Value* value = args.getNext();
        client::si::setWidgetProperty(wip, index, value, ss, ref);
    }

}

void
client::si::IFWidgetRun(game::Session& session, ScriptSide& ss, const WidgetReference& ref, interpreter::Process& proc, interpreter::Arguments& args)
{
    // ContextProvider that allows child processes to access our local variables:
    class RunContextProvider : public ContextProvider {
     public:
        RunContextProvider(RequestLink2 link)
            : m_link(link)
            { }
        void createContext(game::Session& session, ContextReceiver& recv)
            {
                uint32_t pid;
                if (m_link.getProcessId(pid)) {
                    if (interpreter::Process* parent = session.processList().getProcessById(pid)) {
                        recv.addNewContext(interpreter::ProcessObserverContext::create(*parent));
                    }
                }
            }
     private:
        RequestLink2 m_link;
    };

    // Control
    class RunControl : public Control {
     public:
        RunControl(UserSide& iface, ui::Root& root, afl::string::Translator& tx, RequestLink2 link)
            : Control(iface, root, tx),
              m_link(link),
              m_loop(root),
              m_outputState(),
              m_result(0)
            { }
        virtual void handleStateChange(RequestLink2 link, OutputState::Target target)
            { dialogHandleStateChange(link, target, m_outputState, m_loop, 0); }
        virtual void handleEndDialog(RequestLink2 link, int code)
            { dialogHandleEndDialog(link, code, m_outputState, m_loop, code); }
        virtual void handlePopupConsole(RequestLink2 link)
            {
                // FIXME
                interface().continueProcess(link);
            }
        virtual void handleSetViewRequest(RequestLink2 link, String_t name, bool withKeymap)
            { defaultHandleSetViewRequest(link, name, withKeymap); }
        virtual void handleUseKeymapRequest(client::si::RequestLink2 link, String_t name, int prefix)
            { defaultHandleUseKeymapRequest(link, name, prefix); }
        virtual void handleOverlayMessageRequest(RequestLink2 link, String_t text)
            { defaultHandleOverlayMessageRequest(link, text); }
        virtual ContextProvider* createContextProvider()
            {
                return new RunContextProvider(m_link);
            }
        void run(ui::Root& root, ui::Widget& w)
            {
                root.centerWidget(w);
                root.add(w);
                m_result = m_loop.run();
                root.remove(w);
            }
        OutputState& output()
            { return m_outputState; }

        int getResult() const
            { return m_result; }

     private:
        RequestLink2 m_link;
        ui::EventLoop m_loop;
        OutputState m_outputState;
        int m_result;
    };

    // Task to execute on user side
    class RunTask : public UserTask {
     public:
        RunTask(const WidgetReference& ref)
            : m_ref(ref)
            { }
        void handle(Control& ctl, RequestLink2 link)
            {
                UserSide& us = ctl.interface();
                ui::Widget* theWidget = m_ref.get(ctl);
                if (theWidget == 0) {
                    us.continueProcessWithFailure(link, "Internal error: no widget");
                } else {
                    if (ui::LayoutableGroup* g = dynamic_cast<ui::LayoutableGroup*>(theWidget)) {
                        g->pack();
                    }

                    RunControl dlg(us, ctl.root(), ctl.translator(), link);
                    if (m_ref.getHolder().attachControl(dlg)) {
                        dlg.run(ctl.root(), *theWidget);
                        m_ref.getHolder().detachControl(dlg);
                        std::auto_ptr<afl::data::Value> result(interpreter::makeIntegerValue(dlg.getResult()));
                        us.setVariable(link, "UI.RESULT", result);
                        us.joinProcess(link, dlg.output().getProcess());
                        ctl.handleStateChange(link, dlg.output().getTarget());
                    } else {
                        us.continueProcessWithFailure(link, "Already active");
                    }
                }
            }
     private:
        const WidgetReference m_ref;
    };

    // Actual action
    args.checkArgumentCount(0);
    session.notifyListeners();
    ss.postNewTask(RequestLink1(proc, false), new RunTask(ref));
}

void
client::si::IFWidgetFocus(ScriptSide& ss, const WidgetReference& ref, interpreter::Arguments& args)
{
    // A call
    class Focuser : public UserCall {
     public:
        Focuser(const WidgetReference& ref)
            : m_ref(ref)
            { }
        virtual void handle(Control& ctl)
            {
                if (ui::Widget* w = m_ref.get(ctl)) {
                    w->requestFocus();
                }
            }
     private:
        const WidgetReference m_ref;
    };

    // Actual action
    args.checkArgumentCount(0);
    Focuser f(ref);
    ss.call(f);
}

void
client::si::IFKeyboardFocusAdd(ScriptSide& ss, const WidgetReference& ref, interpreter::Arguments& args)
{
    // A call
    class Adder : public UserCall {
     public:
        Adder(const WidgetReference& ref)
            : m_ref(ref), m_widgets()
            { }
        virtual void handle(Control& ctl)
            {
                using ui::widgets::FocusIterator;
                if (FocusIterator* w = dynamic_cast<FocusIterator*>(m_ref.get(ctl))) {
                    for (size_t i = 0, n = m_widgets.size(); i < n; ++i) {
                        if (ui::Widget* target = m_ref.getHolder().get(ctl, m_widgets[i])) {
                            w->add(*target);
                        }
                    }
                }
            }
        void add(size_t slot)
            { m_widgets.push_back(slot); }
     private:
        const WidgetReference m_ref;
        std::vector<size_t> m_widgets;
    };
    Adder a(ref);

    // Parse and validate arguments
    args.checkArgumentCountAtLeast(1);
    while (args.getNumArgs() > 0) {
        if (afl::data::Value* arg = args.getNext()) {
            // Must be a WidgetValue
            WidgetValue* wv = dynamic_cast<WidgetValue*>(arg);
            if (wv == 0) {
                throw interpreter::Error("Type error, expecting widget");
            }

            // Must be in same dialog
            if (&wv->getValue().getHolder() != &ref.getHolder()) {
                throw interpreter::Error("Attempt to use widget from different dialog");
            }

            // Must have a 'Focus' command
            interpreter::Context::PropertyIndex_t index;
            if (wv->lookup("FOCUS", index) == 0) {
                throw interpreter::Error("Widget not focusable");
            }

            // OK
            a.add(wv->getValue().getSlot());
        }
    }

    // Do it
    ss.call(a);
}

void
client::si::IFListboxAddItem(ScriptSide& ss, const WidgetReference& ref, interpreter::Arguments& args)
{
    // ex IntListboxClosure::call (part)
    // ex userint.pas:Listbox_AddItem
    /* @q AddItem id:Int, text:Str (Listbox Command)
       Add an item to the list box.
       The item will be added at the end.
       The %text is displayed on the listbox.
       The %id will be used to select an item and report the user selection.

       If the listbox is used to prepare a menu, the %id should be an {Atom()|atom}.

       @see Listbox(), UI.Menu
       @since PCC 1.1.1, PCC2 1.99.25, PCC2 2.40.1 */

    // Parse args
    args.checkArgumentCount(2);
    int32_t id;
    String_t text;
    if (!interpreter::checkIntegerArg(id, args.getNext()) || !interpreter::checkStringArg(text, args.getNext())) {
        return;
    }

    // Do it
    class Adder : public UserCall {
     public:
        Adder(const WidgetReference& ref, int32_t id, const String_t& text)
            : m_ref(ref), m_id(id), m_text(text)
            { }
        virtual void handle(Control& ctl)
            {
                using ui::widgets::StringListbox;
                if (StringListbox* w = dynamic_cast<StringListbox*>(m_ref.get(ctl))) {
                    w->addItem(m_id, m_text);
                }
            }
     private:
        const WidgetReference m_ref;
        const int32_t m_id;
        const String_t m_text;
    };

    // Must use callAsyncNew here; call() would limit throughput too badly.
    // With the SDL engine operating at 100 Hz, even populating a listbox with a dozen items will show noticeable delay using call().
    // callAsyncNew() will batch-process these requests by not waiting for replies.
    ss.callAsyncNew(new Adder(ref, id, text));
}

void
client::si::IFListboxDialogRun(game::Session& session, ScriptSide& ss, const WidgetReference& ref, interpreter::Process& proc, interpreter::Arguments& args)
{
    // ex IntListboxClosure::call
    // ex userint.pas:Listbox_Run
    /* @q Run (Listbox Command)
       Shows the list box and let the user select an item.
       If the user confirms the selection, the chosen item's %id is stored in {UI.Result}.
       If the user cancels, {UI.Result} is set to EMPTY.

       @see Listbox()
       @since PCC 1.1.1, PCC2 1.99.25 */
    class RunTask : public UserTask {
     public:
        RunTask(const WidgetReference& ref)
            : m_ref(ref)
            { }
        void handle(Control& ctl, RequestLink2 link)
            {
                UserSide& us = ctl.interface();
                StringListDialogWidget* w = dynamic_cast<StringListDialogWidget*>(m_ref.get(ctl));
                if (w == 0 || w->getParent() != 0) {
                    // Cannot-happen events which would make the universe collapse if they happen
                    us.continueProcessWithFailure(link, "Internal error: wrong widget");
                } else {
                    // Do it.
                    std::auto_ptr<afl::data::Value> result;
                    if (w->run(ctl.root(), ctl.translator(), us.gameSender())) {
                        int32_t i;
                        if (w->getCurrentKey(i)) {
                            result.reset(interpreter::makeIntegerValue(i));
                        }
                    }
                    us.setVariable(link, "UI.RESULT", result);
                    us.continueProcess(link);
                }
            }
     private:
        const WidgetReference m_ref;
    };

    // Do it
    args.checkArgumentCount(0);
    session.notifyListeners();
    ss.postNewTask(RequestLink1(proc, false), new RunTask(ref));
}

// @since PCC2 2.40.8
void
client::si::IFListboxDialogRunMenu(game::Session& session, ScriptSide& ss, const WidgetReference& ref, interpreter::Process& proc, interpreter::Arguments& args)
{
    class RunTask : public UserTask {
     public:
        RunTask(const WidgetReference& ref, const String_t& anchor)
            : m_ref(ref), m_anchor(anchor)
            { }
        void handle(Control& ctl, RequestLink2 link)
            {
                UserSide& us = ctl.interface();
                StringListDialogWidget* w = dynamic_cast<StringListDialogWidget*>(m_ref.get(ctl));
                if (w == 0 || w->getParent() != 0) {
                    // Cannot-happen events which would make the universe collapse if they happen
                    us.continueProcessWithFailure(link, "Internal error: wrong widget");
                } else {
                    // Do it.
                    std::auto_ptr<afl::data::Value> result;
                    if (w->runMenu(ctl.root(), m_anchor)) {
                        int32_t i;
                        if (w->getCurrentKey(i)) {
                            result.reset(interpreter::makeIntegerValue(i));
                        }
                    }
                    us.setVariable(link, "UI.RESULT", result);
                    us.continueProcess(link);
                }
            }
     private:
        const WidgetReference m_ref;
        const String_t m_anchor;
    };

    // Do it
    String_t anchor;
    args.checkArgumentCount(1);
    if (!interpreter::checkStringArg(anchor, args.getNext())) {
        return;
    }

    session.notifyListeners();
    ss.postNewTask(RequestLink1(proc, false), new RunTask(ref, anchor));
}

void
client::si::callWidgetCommand(WidgetCommand cmd, game::Session& session, ScriptSide& ss, const WidgetReference& ref, interpreter::Process& proc, interpreter::Arguments& args)
{
    switch (cmd) {
     case wicRun:
        IFWidgetRun(session, ss, ref, proc, args);
        break;

     case wicEnable:
        setOptionalBooleanProperty(wipEnabled, ss, ref, args);
        break;

     case wicDisable:
        args.checkArgumentCount(0);
        setBooleanProperty(wipEnabled, false, ss, ref);
        break;

     case wicFrameSetColor:
        args.checkArgumentCount(1);
        setWidgetProperty(wipFrameColor, args.getNext(), ss, ref);
        break;

     case wicFocus:
        IFWidgetFocus(ss, ref, args);
        break;

     case wicInputSetValue:
        args.checkArgumentCount(1);
        setWidgetProperty(wipInputValue, args.getNext(), ss, ref);
        break;

     case wicKeyboardFocusAdd:
        IFKeyboardFocusAdd(ss, ref, args);
        break;

     case wicControlScreenHeaderSetHeading:
        args.checkArgumentCount(1);
        setWidgetProperty(wxpControlScreenHeaderHeading, args.getNext(), ss, ref);
        break;

     case wicControlScreenHeaderSetSubtitle:
        args.checkArgumentCount(1);
        setWidgetProperty(wxpControlScreenHeaderSubtitle, args.getNext(), ss, ref);
        break;

     case wicControlScreenHeaderSetImage:
        args.checkArgumentCount(1);
        setWidgetProperty(wxpControlScreenHeaderImage, args.getNext(), ss, ref);
        break;

     case wicControlScreenHeaderSetButton:
        setIndexedWidgetProperty(wipControlScreenHeaderButton, args, ss, ref);
        break;

     case wicRichDocumentSetContent:
        args.checkArgumentCount(1);
        setWidgetProperty(wxpRichDocumentContent, args.getNext(), ss, ref);
        break;

     case wicListboxAddItem:
        IFListboxAddItem(ss, ref, args);
        break;

     case wicListboxDialogRun:
        IFListboxDialogRun(session, ss, ref, proc, args);
        break;

     case wicListboxDialogRunMenu:
        IFListboxDialogRunMenu(session, ss, ref, proc, args);
        break;

     case wicCheckboxSetValue:
        args.checkArgumentCount(1);
        setWidgetProperty(wipCheckboxValue, args.getNext(), ss, ref);
        break;

     case wicRadiobuttonSetValue:
        args.checkArgumentCount(1);
        setWidgetProperty(wipRadiobuttonValue, args.getNext(), ss, ref);
        break;

     case wicDataViewSetContent:
        args.checkArgumentCount(1);
        setWidgetProperty(wxpDataViewContent, args.getNext(), ss, ref);
        break;

     case wicDataViewSetButton:
        setIndexedWidgetProperty(wipDataViewButton, args, ss, ref);
        break;

     case wicCommandViewSetButton:
        setIndexedWidgetProperty(wipCommandViewButton, args, ss, ref);
        break;

     case wicCommandViewSetLeftText:
        setIndexedWidgetProperty(wipCommandViewLeftText, args, ss, ref);
        break;

     case wicCommandViewSetRightText:
        setIndexedWidgetProperty(wipCommandViewRightText, args, ss, ref);
        break;

     case wicNumberInputSetValue:
        args.checkArgumentCount(1);
        setWidgetProperty(wipNumberInputValue, args.getNext(), ss, ref);
        break;
    }
}
