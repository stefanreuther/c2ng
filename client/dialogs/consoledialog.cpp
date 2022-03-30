/**
  *  \file client/dialogs/consoledialog.cpp
  */

#include "client/dialogs/consoledialog.hpp"
#include "afl/base/optional.hpp"
#include "afl/charset/utf8.hpp"
#include "afl/string/format.hpp"
#include "client/dialogs/helpdialog.hpp"
#include "client/downlink.hpp"
#include "client/si/control.hpp"
#include "client/widgets/consoleview.hpp"
#include "game/interface/completionlist.hpp"
#include "game/interface/contextprovider.hpp"
#include "game/interface/propertylist.hpp"
#include "game/map/planet.hpp"
#include "game/map/ship.hpp"
#include "game/proxy/scripteditorproxy.hpp"
#include "interpreter/contextreceiver.hpp"
#include "interpreter/values.hpp"
#include "ui/eventloop.hpp"
#include "ui/group.hpp"
#include "ui/invisiblewidget.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/spacer.hpp"
#include "ui/widgets/button.hpp"
#include "ui/widgets/inputline.hpp"
#include "ui/widgets/standarddialogbuttons.hpp"
#include "ui/widgets/stringlistbox.hpp"
#include "ui/window.hpp"
#include "util/messagecollector.hpp"
#include "util/messagenotifier.hpp"
#include "util/string.hpp"
#include "util/stringparser.hpp"

using game::interface::PropertyList;

namespace {
    const int NLINES = 15;

    const int NAME_EMS = 15;
    const int VALUE_EMS = 25;


    class PropertyListbox : public ui::widgets::AbstractListbox {
     public:
        PropertyListbox(ui::Root& root, const PropertyList& content)
            : m_root(root),
              m_content(content)
            {
                // ex WPropertyList::WPropertyList
            }

        virtual size_t getNumItems()
            { return m_content.infos.size(); }
        virtual bool isItemAccessible(size_t /*n*/)
            { return true; }
        virtual int getItemHeight(size_t /*n*/)
            { return getItemHeight(); }
        virtual int getHeaderHeight() const
            { return 0; }
        virtual int getFooterHeight() const
            { return 0; }
        virtual void drawHeader(gfx::Canvas& /*can*/, gfx::Rectangle /*area*/)
            { }
        virtual void drawFooter(gfx::Canvas& /*can*/, gfx::Rectangle /*area*/)
            { }
        virtual void drawItem(gfx::Canvas& can, gfx::Rectangle area, size_t item, ItemState state)
            {
                // ex WPropertyList::drawPart
                afl::base::Deleter del;
                gfx::Context<util::SkinColor::Color> ctx(can, getColorScheme());
                prepareColorListItem(ctx, area, state, m_root.colorScheme(), del);

                if (item < m_content.infos.size()) {
                    afl::base::Ref<gfx::Font> normalFont = m_root.provider().getFont(gfx::FontRequest());
                    const PropertyList::Info& e = m_content.infos[item];
                    ctx.setTextAlign(gfx::LeftAlign, gfx::TopAlign);
                    ctx.useFont(*normalFont);
                    ctx.setColor(util::SkinColor::Static);
                    area.consumeX(5);
                    outTextF(ctx, area.splitX(area.getWidth() * NAME_EMS / (NAME_EMS + VALUE_EMS)), e.name);
                    ctx.setColor(e.valueColor);
                    outTextF(ctx, area, e.value);
                }
            }
        virtual void handlePositionChange(gfx::Rectangle& /*oldPosition*/)
            { }
        virtual ui::layout::Info getLayoutInfo() const
            {
                int lines = int(std::min(size_t(20), std::max(m_content.infos.size(), size_t(5))));
                int width = NAME_EMS + VALUE_EMS;
                gfx::Point size = m_root.provider().getFont(gfx::FontRequest())->getCellSize()
                    .scaledBy(width, lines);
                return ui::layout::Info(size, size, ui::layout::Info::GrowBoth);
            }
        virtual bool handleKey(util::Key_t key, int prefix)
            { return defaultHandleKey(key, prefix); }

     private:
        int getItemHeight() const
            { return m_root.provider().getFont(gfx::FontRequest())->getLineHeight(); }

        ui::Root& m_root;
        const PropertyList& m_content;
    };



    class ConsoleController : public ui::InvisibleWidget {
     public:
        ConsoleController(client::widgets::ConsoleView& view,
                          ui::widgets::InputLine& input,
                          client::si::UserSide& user,
                          ui::Root& root,
                          afl::string::Translator& tx)
            : m_scrollback(0),
              m_view(view),
              m_input(input),
              m_user(user),
              m_root(root),
              m_translator(tx),
              m_collector(user.console()),
              m_notifier(root.engine().dispatcher()),
              m_format()
            {
                m_format.setConfiguration("*@-Debug=dim:"
                                          "script.error=right,red:"
                                          "script.trace=right:"
                                          "*@Error=red:"
                                          "script.input=bold:"
                                          "script.result=bold,right:"
                                          "script.empty=dim,right");
                m_recall.setConfiguration("script.input=input:"
                                          "script.result=result");
                user.mainLog().addListener(m_notifier);
                m_notifier.sig_change.add(this, &ConsoleController::onUpdate);
                onUpdate();
            }

        void onUpdate()
            {
                util::MessageCollector::MessageNumber_t nr = m_collector.getNewestPosition();

                // Skip scrollback
                int n = m_scrollback;
                while (n > 0 && m_collector.readOlderMessage(nr, 0, nr)) {
                    --n;
                }

                // Render
                m_view.clear();
                n = NLINES;
                afl::sys::LogListener::Message msg;
                while (n > 0 && m_collector.readOlderMessage(nr, &msg, nr)) {
                    --n;

                    gfx::HorizontalAlignment align = gfx::LeftAlign;
                    int bold = 0;
                    util::SkinColor::Color color = util::SkinColor::Static;
                    String_t result;
                    if (m_format.match(msg, result)) {
                        util::StringParser p(result);
                        String_t s;
                        while (!p.parseEnd()) {
                            p.parseDelim(",", s);
                            if (s == "left") {
                                align = gfx::LeftAlign;
                            } else if (s == "right") {
                                align = gfx::RightAlign;
                            } else if (s == "center") {
                                align = gfx::CenterAlign;
                            } else if (s == "bold") {
                                ++bold;
                            } else if (s == "static") {
                                color = util::SkinColor::Static;
                            } else if (s == "green") {
                                color = util::SkinColor::Green;
                            } else if (s == "yellow") {
                                color = util::SkinColor::Yellow;
                            } else if (s == "red") {
                                color = util::SkinColor::Red;
                            } else if (s == "white") {
                                color = util::SkinColor::White;
                            } else if (s == "blue") {
                                color = util::SkinColor::Blue;
                            } else if (s == "dim") {
                                color = util::SkinColor::Faded;
                            } else {
                                // else what?
                            }

                            if (p.parseEnd()) {
                                break;
                            }
                            p.parseCharacter(',');
                        }
                    }

                    m_view.addLine(n, msg.m_message, align, bold, color);
                }
                m_view.setScrollbackIndicator(m_scrollback);
            }

        virtual bool handleKey(util::Key_t key, int prefix)
            {
                // WConsoleDialog::handleEvent
                switch (key) {
                 case util::KeyMod_Shift + util::Key_PgUp:
                    requestActive();
                    m_scrollback += 5;
                    onUpdate();
                    return true;
                 case util::KeyMod_Shift + util::Key_Up:
                    requestActive();
                    m_scrollback += 1;
                    onUpdate();
                    return true;
                 case util::KeyMod_Shift + util::Key_PgDn:
                    requestActive();
                    m_scrollback -= std::min(m_scrollback, 5);
                    onUpdate();
                    return true;
                 case util::KeyMod_Shift + util::Key_Down:
                    requestActive();
                    m_scrollback -= std::min(m_scrollback, 1);
                    onUpdate();
                    return true;
                 case util::KeyMod_Shift + util::Key_End:
                    requestActive();
                    m_scrollback = 0;
                    onUpdate();
                    return true;
                 case util::Key_Up:
                    requestActive();
                    doRecall(-1);
                    return true;
                 case util::Key_Down:
                    requestActive();
                    doRecall(+1);
                    return true;
                 case util::Key_Tab:
                    doCompletion();
                    return true;
                 case util::Key_Insert:
                    doInsert();
                    return true;
                 case util::KeyMod_Alt + 'v':
                    doListVariables();
                    return true;
                 case util::Key_F1:
                 case util::KeyMod_Alt + 'h':
                    client::dialogs::doHelpDialog(m_root, m_translator, m_user.gameSender(), "pcc2:console");
                    return true;
                 default:
                    return defaultHandleKey(key, prefix);
                }
            }

        void insertCompletion(const String_t& stem, const String_t& completion)
            {
                if (completion.size() > stem.size()) {
                    m_input.setFlag(ui::widgets::InputLine::TypeErase, false);
                    m_input.insertText(completion.substr(stem.size()));
                }
            }

        void doCompletion()
            {
                client::Downlink link(m_root, m_translator);
                game::interface::CompletionList result;
                game::proxy::ScriptEditorProxy(m_user.gameSender())
                    .buildCompletionList(link, result,
                                         afl::charset::Utf8().substr(m_input.getText(), 0, m_input.getCursorIndex()),
                                         false,
                                         std::auto_ptr<game::interface::ContextProvider>(m_user.createContextProvider()));

                String_t stem = result.getStem();
                String_t immediate = result.getImmediateCompletion();
                if (immediate.size() > stem.size()) {
                    insertCompletion(stem, immediate);
                } else if (!result.isEmpty()) {
                    ui::widgets::StringListbox list(m_root.provider(), m_root.colorScheme());
                    int32_t i = 0;
                    for (game::interface::CompletionList::Iterator_t it = result.begin(), e = result.end(); it != e; ++it) {
                        list.addItem(i, *it);
                        ++i;
                    }
                    list.sortItemsAlphabetically();
                    if (ui::widgets::doStandardDialog(m_translator("Completions"), String_t(), list, true, m_root, m_translator)
                        && list.getCurrentKey(i))
                    {
                        game::interface::CompletionList::Iterator_t it = result.begin(), e = result.end();
                        while (it != e && i != 0) {
                            ++it, --i;
                        }
                        if (it != e) {
                            insertCompletion(stem, *it);
                        }
                    } else {
                    }
                } else {
                    // no completions available
                }

                // No matter what happened, should still clear TypeErase to avoid new input overwriting old one.
                m_input.setFlag(ui::widgets::InputLine::TypeErase, false);
            }

        void doRecall(int direction)
            {
                // ex WConsoleDialog::doRecall, console.pas:ScrollInput

                // Find current position.
                // If at end, initialize.
                util::MessageCollector::MessageNumber_t pos;
                if (!m_recallPosition.get(pos)) {
                    pos = m_collector.getNewestPosition();
                    m_recallLastInput = m_input.getText();
                }

                while (1) {
                    afl::sys::LogListener::Message msg;
                    bool ok = (direction < 0
                               ? m_collector.readOlderMessage(pos, &msg, pos)
                               : m_collector.readNewerMessage(pos, &msg, pos));
                    if (!ok) {
                        // End reached
                        if (direction < 0) {
                            // Cannot go further
                            break;
                        } else {
                            // End reached
                            m_recallPosition = afl::base::Nothing;
                            m_input.setText(m_recallLastInput);
                            break;
                        }
                    } else {
                        // Found a line. Recallable?
                        String_t mode;
                        if (msg.m_message != m_input.getText() && m_recall.match(msg, mode) && (mode == "input" || mode == "result")) {
                            m_recallPosition = pos;
                            m_input.setText(msg.m_message);
                            break;
                        }
                    }
                }
            }

        void doInsert()
            {
                util::MessageCollector::MessageNumber_t pos = m_collector.getNewestPosition();

                afl::sys::LogListener::Message msg;
                while (m_collector.readOlderMessage(pos, &msg, pos)) {
                    String_t mode;
                    if (m_recall.match(msg, mode)) {
                        if (mode == "result") {
                            m_input.insertText(msg.m_message);
                        }
                        break;
                    }
                }
            }

        void doListVariables()
            {
                // ex WConsoleDialog::doListVariables
                client::Downlink link(m_root, m_translator);
                PropertyList result;
                game::proxy::ScriptEditorProxy(m_user.gameSender())
                    .buildPropertyList(link, result, std::auto_ptr<game::interface::ContextProvider>(m_user.createContextProvider()));

                if (!result.infos.empty()) {
                    PropertyListbox box(m_root, result);
                    if (ui::widgets::doStandardDialog(result.title, String_t(), box, true, m_root, m_translator)) {
                        size_t index = box.getCurrentItem();
                        if (index < result.infos.size()) {
                            // Must manually reset TypeErase which gets set by the focus change
                            m_input.setFlag(ui::widgets::InputLine::TypeErase, false);
                            m_input.insertText(result.infos[index].name);
                        }
                    }
                }
            }

        void resetRecall()
            {
                m_recallPosition = afl::base::Nothing;
                m_scrollback = 0;
                onUpdate();
            }

     private:
        int m_scrollback;
        client::widgets::ConsoleView& m_view;
        ui::widgets::InputLine& m_input;
        client::si::UserSide& m_user;
        ui::Root& m_root;
        afl::string::Translator& m_translator;
        util::MessageCollector& m_collector;
        util::MessageNotifier m_notifier;
        util::MessageMatcher m_format;
        util::MessageMatcher m_recall;
        afl::base::Optional<util::MessageCollector::MessageNumber_t> m_recallPosition;
        String_t m_recallLastInput;
    };


    class ConsoleDialog : private client::si::Control {
     public:
        ConsoleDialog(client::si::UserSide& iface, client::si::Control& parentControl, client::si::OutputState& outputState)
            : Control(iface),
              m_parentControl(parentControl),
              m_loop(parentControl.root()),
              m_window(translator()("Console"), parentControl.root().provider(), parentControl.root().colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5),
              m_input(1000, 30, parentControl.root()),
              m_group(ui::layout::HBox::instance5),
              m_spacer(),
              m_okButton(translator()("OK"), util::Key_Return, parentControl.root()),
              m_cancelButton(translator()("Cancel"), util::Key_Escape, parentControl.root()),
              m_consoleView(parentControl.root().provider(), gfx::Point(35, 15)),
              m_consoleController(m_consoleView, m_input, iface, parentControl.root(), parentControl.translator()),
              m_outputState(outputState)
            {
                m_input.setFont(gfx::FontRequest().addSize(1));
                m_okButton.sig_fire.add(this, &ConsoleDialog::onOK);
                m_cancelButton.sig_fire.addNewClosure(m_loop.makeStop(0));
                m_window.add(m_consoleView);
                m_window.add(m_consoleController);
                m_window.add(m_input);
                m_window.add(m_group);
                m_group.add(m_spacer);
                m_group.add(m_okButton);
                m_group.add(m_cancelButton);
                m_input.requestFocus();

                // Greeting
                // FIXME: this is in WConsoleDialog::processPreIdle and thus should not appear if we are blocked by a script that just did UI.PopupConsole
                String_t greeting = translator()("Enter command or expression:");
                afl::sys::LogListener::Message msg;
                util::MessageCollector::MessageNumber_t nr = iface.console().getNewestPosition();
                if (!iface.console().readOlderMessage(nr, &msg, nr) || msg.m_message != greeting) {
                    iface.mainLog().write(afl::sys::LogListener::Info, "console", greeting);
                }
            }

        void onOK()
            {
                // ex WConsoleDialog::onEnter
                // Reset
                m_consoleController.resetRecall();

                // Logging happens in CommandTask (verbose=true)
                String_t command = afl::string::strTrim(m_input.getText());
                if (!command.empty()) {
                    executeCommandWait(command, true, afl::string::Format(translator()("Console: %s").c_str(), command));
                }
                m_input.setText(String_t());
            }

        bool run(client::si::InputState& inputState)
            {
                m_window.pack();
                root().centerWidget(m_window);
                root().add(m_window);
                continueProcessWait(inputState.getProcess());
                return m_loop.run() != 0;
            }


        // Control virtuals:
        virtual void handleStateChange(client::si::RequestLink2 link, client::si::OutputState::Target target)
            {
                // ex WConsoleDialog::processEvent
                dialogHandleStateChange(link, target, m_outputState, m_loop, 1);
            }
        virtual void handlePopupConsole(client::si::RequestLink2 link)
            { interface().continueProcess(link); }
        virtual void handleScanKeyboardMode(client::si::RequestLink2 link)
            { defaultHandleScanKeyboardMode(link); }
        virtual void handleEndDialog(client::si::RequestLink2 link, int /*code*/)
            {
                // Console does not count as a dialog
                interface().continueProcess(link);
            }

        virtual void handleSetView(client::si::RequestLink2 link, String_t name, bool withKeymap)
            { m_parentControl.handleSetView(link, name, withKeymap); }
        virtual void handleUseKeymap(client::si::RequestLink2 link, String_t name, int prefix)
            { defaultHandleUseKeymap(link, name, prefix); }
        virtual void handleOverlayMessage(client::si::RequestLink2 link, String_t text)
            { defaultHandleOverlayMessage(link, text); }
        virtual game::interface::ContextProvider* createContextProvider()
            {
                return m_parentControl.createContextProvider();
            }

     private:
        client::si::Control& m_parentControl;
        ui::EventLoop m_loop;
        ui::Window m_window;
        ui::widgets::InputLine m_input;
        ui::Group m_group;
        ui::Spacer m_spacer;
        ui::widgets::Button m_okButton;
        ui::widgets::Button m_cancelButton;
        client::widgets::ConsoleView m_consoleView;
        ConsoleController m_consoleController;
        client::si::OutputState& m_outputState;
    };
}

bool
client::dialogs::doConsoleDialog(client::si::UserSide& iface,
                                 client::si::Control& parentControl,
                                 client::si::InputState& inputState,
                                 client::si::OutputState& outputState)
{
    // ex client/dialogs/consoledlg.cc:doConsole
    ConsoleDialog dialog(iface, parentControl, outputState);
    return dialog.run(inputState);
}
