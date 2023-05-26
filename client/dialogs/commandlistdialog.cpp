/**
  *  \file client/dialogs/commandlistdialog.cpp
  */

#include "client/dialogs/commandlistdialog.hpp"
#include "afl/base/deleter.hpp"
#include "client/downlink.hpp"
#include "client/widgets/helpwidget.hpp"
#include "game/proxy/commandlistproxy.hpp"
#include "ui/dialogs/messagebox.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/rich/documentview.hpp"
#include "ui/spacer.hpp"
#include "ui/widgets/abstractlistbox.hpp"
#include "ui/widgets/button.hpp"
#include "ui/widgets/framegroup.hpp"
#include "ui/widgets/inputline.hpp"
#include "ui/widgets/keydispatcher.hpp"
#include "ui/widgets/quit.hpp"
#include "ui/window.hpp"

namespace {
    class CommandListbox : public ui::widgets::AbstractListbox {
     public:
        CommandListbox(ui::Root& root)
            : m_root(root)
            {
                // ex WCommandList::WCommandList
            }

        void setContent(game::proxy::CommandListProxy::Infos_t& content, size_t newIndex)
            {
                m_content.swap(content);
                setCurrentItem(newIndex);
                handleModelChange();
            }

        const game::proxy::CommandListProxy::Info* getCurrentCommand()
            {
                // ex WCommandList::getCurrentCommand
                size_t index = getCurrentItem();
                if (index < m_content.size()) {
                    return &m_content[index];
                } else {
                    return 0;
                }
            }

        virtual size_t getNumItems() const
            { return m_content.size(); }
        virtual bool isItemAccessible(size_t /*n*/) const
            { return true; }
        virtual int getItemHeight(size_t /*n*/) const
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
                // ex WCommandList::drawPart
                afl::base::Deleter del;
                gfx::Context<util::SkinColor::Color> ctx(can, getColorScheme());
                prepareColorListItem(ctx, area, state, m_root.colorScheme(), del);

                if (item < m_content.size()) {
                    afl::base::Ref<gfx::Font> normalFont = m_root.provider().getFont(gfx::FontRequest());
                    const game::proxy::CommandListProxy::Info& e = m_content[item];
                    ctx.setTextAlign(gfx::LeftAlign, gfx::TopAlign);
                    ctx.useFont(*normalFont);
                    ctx.setColor(util::SkinColor::Static);
                    area.consumeX(5);
                    outTextF(ctx, area, e.text);
                }
            }
        virtual void handlePositionChange()
            { }
        virtual ui::layout::Info getLayoutInfo() const
            {
                int lines = 20;
                int width = 30;
                gfx::Point size = m_root.provider().getFont(gfx::FontRequest())->getCellSize()
                    .scaledBy(width, lines);
                return ui::layout::Info(size, ui::layout::Info::GrowBoth);
            }
        virtual bool handleKey(util::Key_t key, int prefix)
            { return defaultHandleKey(key, prefix); }

     private:
        int getItemHeight() const
            { return m_root.provider().getFont(gfx::FontRequest())->getLineHeight(); }

        ui::Root& m_root;
        game::proxy::CommandListProxy::Infos_t m_content;
    };

    class CommandListDialog : public client::si::Control {
     public:
        CommandListDialog(client::si::UserSide& side, ui::Root& root, afl::string::Translator& tx, client::si::OutputState& outputState)
            : Control(side),
              m_loop(root),
              m_outputState(outputState),
              m_proxy(side.gameSender()),
              m_listbox(root),
              m_gotoButton(tx("Go to"), 'g', root),
              m_delButton(tx("Del"), util::Key_Delete, root),
              m_infoView(root.provider().getFont(gfx::FontRequest())->getCellSize().scaledBy(30, 2), 0, root.provider())
            {
                m_gotoButton.sig_fire.add(this, &CommandListDialog::onGoto);
                m_delButton.sig_fire.add(this, &CommandListDialog::onDelete);
                m_listbox.sig_change.add(this, &CommandListDialog::updateDialog);
            }

        bool init()
            {
                client::Downlink link(root(), translator());
                game::proxy::CommandListProxy::Infos_t list;
                if (m_proxy.init(link, list)) {
                    m_listbox.setContent(list, 0);
                    return true;
                } else {
                    return false;
                }
            }

        void run()
            {
                // VBox
                //   Frame
                //     WCommandList
                //     UIScrollbar
                //   HBox
                //     WCommandInfo
                //     VBox
                //       UIButton "G"
                //       UISpacer
                //   HBox
                //     UIButton "Close"
                //     UIButton "Del"
                //     UIButton "Ins"
                //     UISpacer
                //     UIButton "Help"
                afl::string::Translator& tx = translator();
                afl::base::Deleter del;
                ui::Window& win = del.addNew(new ui::Window(tx("Auxiliary Commands"), root().provider(), root().colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5));

                win.add(ui::widgets::FrameGroup::wrapWidget(del, root().colorScheme(), ui::LoweredFrame, m_listbox));

                ui::Group& g1 = del.addNew(new ui::Group(ui::layout::HBox::instance5));
                ui::Group& g11 = del.addNew(new ui::Group(ui::layout::VBox::instance0));
                g1.add(m_infoView);
                g11.add(m_gotoButton);
                g11.add(del.addNew(new ui::Spacer()));
                g1.add(g11);
                win.add(g1);

                ui::Widget& helper = del.addNew(new client::widgets::HelpWidget(root(), tx, interface().gameSender(), "pcc2:auxcmds"));
                ui::Group& g2 = del.addNew(new ui::Group(ui::layout::HBox::instance5));
                ui::widgets::Button& btnOK  = del.addNew(new ui::widgets::Button(tx("Close"), util::Key_Escape, root()));
                ui::widgets::Button& btnAdd = del.addNew(new ui::widgets::Button(tx("Ins"), util::Key_Insert, root()));
                ui::widgets::Button& btnHelp = del.addNew(new ui::widgets::Button(tx("Help"), 'h', root()));
                btnOK.sig_fire.addNewClosure(m_loop.makeStop(0));
                btnAdd.sig_fire.add(this, &CommandListDialog::onInsert);
                btnHelp.dispatchKeyTo(helper);
                g2.add(btnOK);
                g2.add(m_delButton);
                g2.add(btnAdd);
                g2.add(del.addNew(new ui::Spacer()));
                g2.add(btnHelp);
                win.add(g2);
                win.add(del.addNew(new ui::widgets::Quit(root(), m_loop)));
                win.add(helper);

                ui::widgets::KeyDispatcher& disp = del.addNew(new ui::widgets::KeyDispatcher());
                disp.add(' ', this, &CommandListDialog::onEdit);
                disp.add(util::Key_Return, this, &CommandListDialog::onGoto);
                win.add(disp);

                win.pack();

                updateDialog();      // after pack() so formatting uses correct dimensions

                root().centerWidget(win);
                root().add(win);
                m_loop.run();
            }

        void onGoto()
            {
                // ex WCommandEditDialog::onGoto
                if (const game::proxy::CommandListProxy::Info* p = m_listbox.getCurrentCommand()) {
                    executeGoToReferenceWait("Auxiliary Commands", p->ref);
                }
            }

        void onDelete()
            {
                // ex WCommandList::handleEvent (part), WCommandList::deleteCurrent
                afl::string::Translator& tx = translator();
                if (const game::proxy::CommandListProxy::Info* p = m_listbox.getCurrentCommand()) {
                    if (ui::dialogs::MessageBox(tx("Delete this command?"), tx("Auxiliary Commands"), root()).doYesNoDialog(tx)) {
                        client::Downlink link(root(), tx);
                        game::proxy::CommandListProxy::Infos_t newList;
                        m_proxy.removeCommand(link, p->text, newList);
                        m_listbox.setContent(newList, m_listbox.getCurrentItem());
                        m_listbox.requestActive();
                    }
                }
            }

        void onInsert()
            {
                // ex WCommandList::editNew
                edit(String_t());
            }

        void onEdit()
            {
                // ex WCommandList::editCurrent
                if (const game::proxy::CommandListProxy::Info* p = m_listbox.getCurrentCommand()) {
                    edit(p->text);
                }
            }

        void updateDialog()
            {
                const game::proxy::CommandListProxy::Info* p = m_listbox.getCurrentCommand();

                // ex WCommandEditDialog::onScroll
                m_gotoButton.setState(ui::widgets::Button::DisabledState, !p || !p->ref.isSet());
                m_delButton.setState(ui::widgets::Button::DisabledState, !p);

                // ex WCommandInfo::drawContent (very far relative; different style)
                ui::rich::Document& doc = m_infoView.getDocument();
                doc.clear();
                if (p) {
                    doc.add(util::rich::Text(String_t(p->text + ": ")).withStyle(util::rich::StyleAttribute::Bold));
                    doc.add(p->info);
                }
                doc.finish();
                m_infoView.handleDocumentUpdate();
            }

        void edit(String_t s)
            {
                // ex phost.pas:CCommandEditor.EditCommand
                afl::string::Translator& tx = translator();
                ui::widgets::InputLine input(30, root());
                input.setText(s);
                input.setFlag(ui::widgets::InputLine::GameChars, true);
                input.setFont("+");
                if (input.doStandardDialog(tx("Auxiliary Commands"), tx("Edit command:"), tx)) {
                    client::Downlink link(root(), tx);
                    game::proxy::CommandListProxy::Infos_t newList;
                    size_t newPos;
                    if (m_proxy.addCommand(link, input.getText(), newList, newPos)) {
                        m_listbox.setContent(newList, newPos);
                    } else {
                        ui::dialogs::MessageBox(tx("This command was not recognized."), tx("Auxiliary Commands"), root()).doOkDialog(tx);
                    }
                    m_listbox.requestActive();
                }
            }


        virtual void handleStateChange(client::si::RequestLink2 link, client::si::OutputState::Target target)
            { dialogHandleStateChange(link, target, m_outputState, m_loop, 0); }
        virtual void handlePopupConsole(client::si::RequestLink2 link)
            { defaultHandlePopupConsole(link); }
        virtual void handleScanKeyboardMode(client::si::RequestLink2 link)
            { defaultHandleScanKeyboardMode(link); }
        virtual void handleEndDialog(client::si::RequestLink2 link, int code)
            { dialogHandleEndDialog(link, code, m_outputState, m_loop, 0); }
        virtual void handleSetView(client::si::RequestLink2 link, String_t name, bool withKeymap)
            { defaultHandleSetView(link, name, withKeymap); }
        virtual void handleUseKeymap(client::si::RequestLink2 link, String_t name, int prefix)
            { defaultHandleUseKeymap(link, name, prefix); }
        virtual void handleOverlayMessage(client::si::RequestLink2 link, String_t text)
            { defaultHandleOverlayMessage(link, text); }
        virtual afl::base::Optional<game::Id_t> getFocusedObjectId(game::Reference::Type type) const
            { return defaultGetFocusedObjectId(type); }
        virtual game::interface::ContextProvider* createContextProvider()
            { return 0; }

     private:
        ui::EventLoop m_loop;
        client::si::OutputState& m_outputState;
        game::proxy::CommandListProxy m_proxy;
        CommandListbox m_listbox;
        ui::widgets::Button m_gotoButton;
        ui::widgets::Button m_delButton;
        ui::rich::DocumentView m_infoView;
    };

}

void
client::dialogs::editCommands(ui::Root& root,
                              client::si::UserSide& iface,
                              client::si::OutputState& outputState,
                              afl::string::Translator& tx)
{
    // ex client/dialogs/commandedit.cc:editCommands, phost.pas:EditCommands
    // FIXME: ObscureFeature warning:
    // IF NOT ObscureFeature(obs_Commands,
    //                       'This window will list auxiliary commands. Those are used with '+
    //                       'the PHost command processor, for example.'#13+
    //                       'You can review, edit and delete these commands here. Please read '+
    //                       'the help screen before changing anything here.',
    //                       hcEditCommands)
    //   THEN Exit;
    CommandListDialog dialog(iface, root, tx, outputState);
    if (!dialog.init()) {
        ui::dialogs::MessageBox(tx("Auxiliary commands are not supported for this host."),
                                tx("Auxiliary Commands"),
                                root).doOkDialog(tx);
    } else {
        dialog.run();
    }
}
