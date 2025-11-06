/**
  *  \file client/dialogs/directoryselectiondialog.cpp
  */

#include <memory>
#include "client/dialogs/directoryselectiondialog.hpp"
#include "afl/string/format.hpp"
#include "client/downlink.hpp"
#include "client/widgets/folderlistbox.hpp"
#include "ui/dialogs/messagebox.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/spacer.hpp"
#include "ui/widgets/inputline.hpp"
#include "ui/widgets/keydispatcher.hpp"
#include "ui/widgets/quit.hpp"
#include "ui/widgets/scrollbarcontainer.hpp"
#include "ui/widgets/simpleiconbox.hpp"
#include "ui/widgets/standarddialogbuttons.hpp"
#include "ui/window.hpp"
#include "util/directorybrowser.hpp"

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


    class InitTask : public util::Request<afl::io::FileSystem> {
     public:
        InitTask(std::auto_ptr<DirectoryBrowser>& result, String_t folderName, afl::string::Translator& tx, State& state)
            : m_result(result),
              m_folderName(folderName),
              m_translator(tx),
              m_state(state)
            { }
        virtual void handle(afl::io::FileSystem& fs)
            {
                m_result.reset(new DirectoryBrowser(fs));
                m_result->openDirectory(m_folderName);
                if (m_result->getErrorText().empty()) {
                    m_result->openParent();
                } else {
                    m_result->openRoot();
                }
                setState(m_state, *m_result, m_translator);
            }
     private:
        std::auto_ptr<DirectoryBrowser>& m_result;
        String_t m_folderName;
        afl::string::Translator& m_translator;
        State& m_state;
    };

    class UpTask : public util::Request<afl::io::FileSystem> {
     public:
        UpTask(std::auto_ptr<DirectoryBrowser>& browser, size_t n, afl::string::Translator& tx, State& state)
            : m_browser(browser),
              m_count(n),
              m_translator(tx),
              m_state(state)
            { }
        virtual void handle(afl::io::FileSystem& /*fs*/)
            {
                if (DirectoryBrowser* b = m_browser.get()) {
                    for (size_t i = 0; i < m_count; ++i) {
                        b->openParent();
                    }
                    setState(m_state, *b, m_translator);
                }
            }
     private:
        std::auto_ptr<DirectoryBrowser>& m_browser;
        size_t m_count;
        afl::string::Translator& m_translator;
        State& m_state;
    };

    class DownTask : public util::Request<afl::io::FileSystem> {
     public:
        DownTask(std::auto_ptr<DirectoryBrowser>& browser, size_t index, afl::string::Translator& tx, State& state)
            : m_browser(browser),
              m_index(index),
              m_translator(tx),
              m_state(state)
            { }
        virtual void handle(afl::io::FileSystem& /*fs*/)
            {
                if (DirectoryBrowser* b = m_browser.get()) {
                    b->openChild(m_index);
                    setState(m_state, *b, m_translator);
                }
            }
     private:
        std::auto_ptr<DirectoryBrowser>& m_browser;
        size_t m_index;
        afl::string::Translator& m_translator;
        State& m_state;
    };

    class NewTask : public util::Request<afl::io::FileSystem> {
     public:
        NewTask(std::auto_ptr<DirectoryBrowser>& browser, String_t name, afl::string::Translator& tx, State& state)
            : m_browser(browser),
              m_name(name),
              m_translator(tx),
              m_state(state)
            { }
        virtual void handle(afl::io::FileSystem& /*fs*/)
            {
                if (DirectoryBrowser* b = m_browser.get()) {
                    m_result = b->createDirectory(m_name, m_translator);
                    setState(m_state, *b, m_translator);
                }
            }
        const String_t& getResult() const
            { return m_result; }
     private:
        std::auto_ptr<DirectoryBrowser>& m_browser;
        String_t m_name;
        String_t m_result;
        afl::string::Translator& m_translator;
        State& m_state;
    };


    class Dialog {
     public:
        Dialog(ui::Root& root, afl::string::Translator& tx, util::RequestSender<afl::io::FileSystem> session)
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
                InitTask t(m_browser, folderName, m_translator, state);
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
                window.add(del.addNew(new ui::widgets::ScrollbarContainer(m_list, m_root)));

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
                window.add(del.addNew(new ui::widgets::Quit(m_root, m_loop)));
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
                UpTask t(m_browser, 1, m_translator, state);
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
                    DownTask t(m_browser, index - m_contentOffset, m_translator, state);
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
                NewTask t(m_browser, input.getText(), m_translator, state);
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
                    UpTask t(m_browser, numItems-n-1, m_translator, state);
                    m_link.call(m_sender, t);
                    loadState(state);
                }
            }

        const String_t& getDirectoryName() const
            { return m_thisDirectoryName; }

     private:
        ui::Root& m_root;
        afl::string::Translator& m_translator;
        util::RequestSender<afl::io::FileSystem> m_sender;
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

bool
client::dialogs::doDirectorySelectionDialog(ui::Root& root, afl::string::Translator& tx, util::RequestSender<afl::io::FileSystem> fs, String_t& folderName)
{
    Dialog dlg(root, tx, fs);
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
