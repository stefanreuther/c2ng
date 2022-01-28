/**
  *  \file client/dialogs/directoryselectiondialog.cpp
  */

#include <memory>
#include "client/dialogs/directoryselectiondialog.hpp"
#include "afl/string/format.hpp"
#include "client/downlink.hpp"
#include "client/widgets/folderlistbox.hpp"
#include "game/browser/browser.hpp"
#include "ui/dialogs/messagebox.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/spacer.hpp"
#include "ui/widgets/inputline.hpp"
#include "ui/widgets/keydispatcher.hpp"
#include "ui/widgets/simpleiconbox.hpp"
#include "ui/widgets/standarddialogbuttons.hpp"
#include "ui/window.hpp"
#include "util/directorybrowser.hpp"
#include "util/translation.hpp"

namespace {
    using client::widgets::FolderListbox;
    using util::DirectoryBrowser;
    using ui::widgets::SimpleIconBox;

    struct State {
        String_t thisDirectoryName;
        size_t contentOffset;
        FolderListbox::Items_t contentItems;
        DirectoryBrowser::OptionalIndex_t selectedItem;
        SimpleIconBox::Items_t crumbItems;
    };


    void setState(State& out, const DirectoryBrowser& in, afl::string::Translator& tx)
    {
        // Directory name
        out.thisDirectoryName = in.getCurrentDirectory()->getDirectoryName();

        // Content
        if (!out.thisDirectoryName.empty()) {
            out.contentItems.push_back(FolderListbox::Item(tx("[Choose this directory]"), 0, true, FolderListbox::iNone));
            out.contentOffset = 1;
        } else {
            out.contentOffset = 0;
        }
        const std::vector<DirectoryBrowser::DirectoryItem>& dirs = in.directories();
        for (size_t i = 0, n = dirs.size(); i < n; ++i) {
            out.contentItems.push_back(FolderListbox::Item(dirs[i].title, 0, true, FolderListbox::iFolder));
        }

        // Focused index
        out.selectedItem = in.getSelectedChild();

        // Crumb list
        out.crumbItems.push_back(SimpleIconBox::Item(tx("[Places]")));
        const std::vector<DirectoryBrowser::DirectoryPtr_t>& path = in.path();
        for (size_t i = 0, n = path.size(); i < n; ++i) {
            if (path[i].get() != 0) {
                String_t title = path[i]->getTitle();
                if (title.empty()) {
                    title = path[i]->getDirectoryName();
                }
                out.crumbItems.push_back(SimpleIconBox::Item(title));
            }
        }

        // FIXME: do something with error reports
    }

    
    class InitTask : public util::Request<game::browser::Session> {
     public:
        InitTask(std::auto_ptr<DirectoryBrowser>& result, String_t folderName, State& state)
            : m_result(result),
              m_folderName(folderName),
              m_state(state)
            { }
        virtual void handle(game::browser::Session& session)
            {
                game::browser::Browser& b = session.browser();
                m_result.reset(new DirectoryBrowser(b.fileSystem()));
                m_result->openDirectory(m_folderName);
                if (m_result->getErrorText().empty()) {
                    m_result->openParent();
                } else {
                    m_result->openRoot();
                }
                setState(m_state, *m_result, session.translator());
            }
     private:
        std::auto_ptr<DirectoryBrowser>& m_result;
        String_t m_folderName;
        State& m_state;
    };

    class UpTask : public util::Request<game::browser::Session> {
     public:
        UpTask(std::auto_ptr<DirectoryBrowser>& browser, size_t n, State& state)
            : m_browser(browser),
              m_count(n),
              m_state(state)
            { }
        virtual void handle(game::browser::Session& session)
            {
                if (DirectoryBrowser* b = m_browser.get()) {
                    for (size_t i = 0; i < m_count; ++i) {
                        b->openParent();
                    }
                    setState(m_state, *b, session.translator());
                }
            }
     private:
        std::auto_ptr<DirectoryBrowser>& m_browser;
        size_t m_count;
        State& m_state;
    };

    class DownTask : public util::Request<game::browser::Session> {
     public:
        DownTask(std::auto_ptr<DirectoryBrowser>& browser, size_t index, State& state)
            : m_browser(browser),
              m_index(index),
              m_state(state)
            { }
        virtual void handle(game::browser::Session& session)
            {
                if (DirectoryBrowser* b = m_browser.get()) {
                    b->openChild(m_index);
                    setState(m_state, *b, session.translator());
                }
            }
     private:
        std::auto_ptr<DirectoryBrowser>& m_browser;
        size_t m_index;
        State& m_state;
    };

    class NewTask : public util::Request<game::browser::Session> {
     public:
        NewTask(std::auto_ptr<DirectoryBrowser>& browser, String_t name, State& state)
            : m_browser(browser),
              m_name(name),
              m_state(state)
            { }
        virtual void handle(game::browser::Session& session)
            {
                if (DirectoryBrowser* b = m_browser.get()) {
                    m_result = b->createDirectory(m_name);
                    setState(m_state, *b, session.translator());
                }
            }
        const String_t& getResult() const
            { return m_result; }
     private:
        std::auto_ptr<DirectoryBrowser>& m_browser;
        String_t m_name;
        String_t m_result;
        State& m_state;
    };


    class Dialog {
     public:
        Dialog(ui::Root& root, afl::string::Translator& tx, util::RequestSender<game::browser::Session> session)
            : m_root(root),
              m_translator(tx),
              m_sender(session),
              m_link(root, tx),
              m_list(gfx::Point(20, 15), root),
              m_crumbs(root.provider().getFont(gfx::FontRequest())->getCellSize().scaledBy(30, 1), root),
              m_buttons(root, tx),
              m_insButton(tx("Ins - New directory..."), util::Key_Insert, root),
              m_loop(root),
              m_browser(),
              m_thisDirectoryName(),
              m_contentOffset(0)
            { }

        bool init(String_t folderName)
            {
                State state;
                InitTask t(m_browser, folderName, state);
                m_link.call(m_sender, t);
                if (m_browser.get() != 0) {
                    loadState(state);
                    return true;
                } else {
                    return false;
                }
            }

        void loadState(State& state)
            {
                m_list.swapItems(state.contentItems);
                size_t index = 0;
                if (state.selectedItem.get(index)) {
                    index += state.contentOffset;
                }
                m_list.setCurrentItem(index);
                m_crumbs.swapContent(state.crumbItems, state.crumbItems.size()-1);
                m_thisDirectoryName = state.thisDirectoryName;
                m_contentOffset = state.contentOffset;
            }

        bool run()
            {
                afl::base::Deleter del;
                ui::Window& window = del.addNew(new ui::Window(m_translator("Choose directory"), m_root.provider(), m_root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5));
                window.add(m_crumbs);
                window.add(m_list);

                ui::widgets::KeyDispatcher keys;
                keys.add(util::Key_Left,  this, &Dialog::onKeyLeft);
                keys.add(util::Key_Right, this, &Dialog::onEnter);

                m_list.sig_itemDoubleClick.add(this, &Dialog::onEnter);
                m_crumbs.sig_change.add(this, &Dialog::onCrumbClick);

                ui::Group& g = del.addNew(new ui::Group(ui::layout::HBox::instance5));
                g.add(m_insButton);
                g.add(del.addNew(new ui::Spacer()));
                window.add(g);
                window.add(m_buttons);
                window.add(keys);
                window.pack();

                m_buttons.ok().sig_fire.add(this, &Dialog::onEnter);
                m_buttons.cancel().sig_fire.addNewClosure(m_loop.makeStop(0));
                m_insButton.sig_fire.add(this, &Dialog::onNewDirectory);

                m_root.centerWidget(window);
                m_root.add(window);
                m_list.requestFocus();
                return (m_loop.run() != 0);
            }

        void onKeyLeft()
            {
                State state;
                UpTask t(m_browser, 1, state);
                m_link.call(m_sender, t);
                loadState(state);
            }

        void onEnter()
            {
                size_t index = m_list.getCurrentItem();
                if (!m_thisDirectoryName.empty() && index == 0) {
                    m_loop.stop(1);
                } else {
                    State state;
                    DownTask t(m_browser, index - m_contentOffset, state);
                    m_link.call(m_sender, t);
                    loadState(state);
                }
            }

        void onNewDirectory()
            {
                ui::widgets::InputLine input(1000, 40, m_root);
                if (!input.doStandardDialog(m_translator("New directory"), m_translator("Directory name"), m_translator)) {
                    return;
                }

                State state;
                NewTask t(m_browser, input.getText(), state);
                m_link.call(m_sender, t);
                if (!t.getResult().empty()) {
                    ui::dialogs::MessageBox(afl::string::Format(m_translator("Creation of directory \"%s\" failed: %s").c_str(),
                                                                input.getText(), t.getResult()),
                                            m_translator("New directory"),
                                            m_root).doOkDialog(m_translator);
                    return;
                }

                loadState(state);
            }

        void onCrumbClick(size_t n)
            {
                size_t numItems = m_crumbs.getNumItems();
                if (n+1 < numItems) {
                    State state;
                    UpTask t(m_browser, numItems-n-1, state);
                    m_link.call(m_sender, t);
                    loadState(state);
                }
            }

        const String_t& getDirectoryName() const
            { return m_thisDirectoryName; }

     private:
        ui::Root& m_root;
        afl::string::Translator& m_translator;
        util::RequestSender<game::browser::Session> m_sender;
        client::Downlink m_link;
        client::widgets::FolderListbox m_list;
        ui::widgets::SimpleIconBox m_crumbs;
        ui::widgets::StandardDialogButtons m_buttons;
        ui::widgets::Button m_insButton;
        ui::EventLoop m_loop;

        std::auto_ptr<DirectoryBrowser> m_browser;
        String_t m_thisDirectoryName;
        size_t m_contentOffset;
    };
}


// FIXME: this is currently tied to the browser session due to lack of ideas how to make it more general
bool
client::dialogs::doDirectorySelectionDialog(ui::Root& root, afl::string::Translator& tx, util::RequestSender<game::browser::Session> session, String_t& folderName)
{
    Dialog dlg(root, tx, session);
    if (!dlg.init(folderName)) {
        return false;
    }
    if (dlg.run() != 0 && !dlg.getDirectoryName().empty()) {
        folderName = dlg.getDirectoryName();
        return true;
    } else {
        return false;
    }
}
