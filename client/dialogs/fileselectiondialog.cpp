/**
  *  \file client/dialogs/fileselectiondialog.cpp
  *  \brief Class client::dialogs::FileSelectionDialog
  */

#include "client/dialogs/fileselectiondialog.hpp"
#include "afl/base/deleter.hpp"
#include "afl/base/optional.hpp"
#include "ui/dialogs/messagebox.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/widgets/focusiterator.hpp"
#include "ui/widgets/framegroup.hpp"
#include "ui/widgets/keydispatcher.hpp"
#include "ui/widgets/quit.hpp"
#include "ui/widgets/standarddialogbuttons.hpp"
#include "ui/widgets/statictext.hpp"
#include "ui/window.hpp"
#include "util/filenamepattern.hpp"
#include "util/io.hpp"

using client::widgets::FileListbox;
using client::dialogs::FileSelectionDialog;
using util::DirectoryBrowser;
using ui::widgets::SimpleIconBox;

struct client::dialogs::FileSelectionDialog::State {
    String_t thisDirectoryName;
    size_t contentOffset;
    FileListbox::Items_t contentItems;
    DirectoryBrowser::OptionalIndex_t selectedItem;
    SimpleIconBox::Items_t crumbItems;

    State()
        : thisDirectoryName(), contentOffset(0), contentItems(), selectedItem(), crumbItems()
        { }
};

struct client::dialogs::FileSelectionDialog::Result {
    bool hasNewContent;
    afl::base::Optional<String_t> error;
    afl::base::Optional<String_t> newWildcard;
    afl::base::Optional<String_t> result;

    Result()
        : hasNewContent(false), error(), newWildcard(), result()
        { }
};

namespace {
    void setState(FileSelectionDialog::State& out, afl::string::Translator& tx, const DirectoryBrowser& in)
    {
        // Directory name
        out.thisDirectoryName = in.getCurrentDirectory()->getDirectoryName();

        // Content
        if (!in.path().empty()) {
            out.contentItems.push_back(FileListbox::Item(tx("[Go up one level]"), 0, true, FileListbox::iUp));
        }
        out.contentOffset = out.contentItems.size();

        const std::vector<DirectoryBrowser::DirectoryItem>& dirs = in.directories();
        for (size_t i = 0, n = dirs.size(); i < n; ++i) {
            out.contentItems.push_back(FileListbox::Item(dirs[i].title, 0, true, FileListbox::iFolder));
        }

        const std::vector<DirectoryBrowser::DirectoryEntryPtr_t>& files = in.files();
        for (size_t i = 0, n = files.size(); i < n; ++i) {
            out.contentItems.push_back(FileListbox::Item(files[i]->getTitle(), 0, false, FileListbox::iFile));
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
        InitTask(std::auto_ptr<DirectoryBrowser>& result, String_t folderName, String_t pattern, FileSelectionDialog::State& state, afl::string::Translator& tx)
            : m_result(result),
              m_folderName(folderName),
              m_pattern(pattern),
              m_state(state),
              m_translator(tx)
            { }
        virtual void handle(afl::io::FileSystem& fs)
            {
                m_result.reset(new DirectoryBrowser(fs));
                m_result->clearFileNamePatterns();
                if (!m_pattern.empty()) {
                    m_result->addFileNamePattern(m_pattern);
                }
                if (m_folderName.empty()) {
                    m_result->openRoot();
                    m_result->loadContent(); // FIXME: this is required because openRoot() is ignored on a default-initialized DirectoryBrowser
                } else {
                    m_result->openDirectory(m_folderName);
                }
                setState(m_state, m_translator, *m_result);
            }
     private:
        std::auto_ptr<DirectoryBrowser>& m_result;
        String_t m_folderName;
        String_t m_pattern;
        FileSelectionDialog::State& m_state;
        afl::string::Translator& m_translator;
    };

    class UpTask : public util::Request<afl::io::FileSystem> {
     public:
        UpTask(std::auto_ptr<DirectoryBrowser>& browser, size_t n, FileSelectionDialog::State& state, afl::string::Translator& tx)
            : m_browser(browser),
              m_count(n),
              m_state(state),
              m_translator(tx)
            { }
        virtual void handle(afl::io::FileSystem& /*fs*/)
            {
                if (DirectoryBrowser* b = m_browser.get()) {
                    for (size_t i = 0; i < m_count; ++i) {
                        b->openParent();
                    }
                    setState(m_state, m_translator, *b);
                }
            }
     private:
        std::auto_ptr<DirectoryBrowser>& m_browser;
        size_t m_count;
        FileSelectionDialog::State& m_state;
        afl::string::Translator& m_translator;
    };

    class DownTask : public util::Request<afl::io::FileSystem> {
     public:
        DownTask(std::auto_ptr<DirectoryBrowser>& browser, size_t index, FileSelectionDialog::State& state, afl::string::Translator& tx)
            : m_browser(browser),
              m_index(index),
              m_state(state),
              m_translator(tx)
            { }
        virtual void handle(afl::io::FileSystem& /*fs*/)
            {
                if (DirectoryBrowser* b = m_browser.get()) {
                    b->openChild(m_index);
                    setState(m_state, m_translator, *b);
                }
            }
     private:
        std::auto_ptr<DirectoryBrowser>& m_browser;
        size_t m_index;
        FileSelectionDialog::State& m_state;
        afl::string::Translator& m_translator;
    };

    class InputTask : public util::Request<afl::io::FileSystem> {
     public:
        InputTask(std::auto_ptr<DirectoryBrowser>& browser,
                  const String_t& input, bool allowPattern,
                  const String_t& defaultExtension,
                  FileSelectionDialog::State& state,
                  FileSelectionDialog::Result& result,
                  afl::string::Translator& tx)
            : m_browser(browser),
              m_input(input),
              m_allowPattern(allowPattern),
              m_defaultExtension(defaultExtension),
              m_state(state),
              m_result(result),
              m_translator(tx)
            { }
        virtual void handle(afl::io::FileSystem& fs)
            {
                // ex UIFileChooser::handleUserInput (sort-of)
                if (DirectoryBrowser* b = m_browser.get()) {
                    // Split into file and directory part
                    String_t dir  = fs.getDirectoryName(m_input);
                    String_t file = fs.getFileName(m_input);
                    util::FileNamePattern npat(file);

                    // Empty file name? Do nothing. Happens when users press Enter
                    // on the input line.
                    if (file.empty()) {
                        setState(m_state, m_translator, *b);
                        m_result.hasNewContent = true;
                        return;
                    }

                    // If this is just a pattern, re-list the directory, no matter which
                    // (this allows changing the pattern while on the "roots" view)
                    if (file == m_input && m_allowPattern && npat.hasWildcard()) {
                        b->clearFileNamePatterns();
                        b->addFileNamePattern(npat);
                        b->loadContent();
                        setState(m_state, m_translator, *b);
                        m_result.hasNewContent = true;
                        m_result.newWildcard = file;
                        return;
                    }
                    
                    // Handle relative file names
                    if (!fs.isAbsolutePathName(dir)) {
                        // Relative file names cannot be used in the "roots" view
                        afl::base::Ref<afl::io::Directory> current = b->getCurrentDirectory();
                        if (current->getDirectoryName().empty()) {
                            m_result.error = m_translator("You cannot create files here. Please choose a place (drive, partition) first.");
                            return;
                        }
                        dir = fs.makePathName(current->getDirectoryName(), dir);
                    }

                    // Remove ".." (getAbsoluteName is required to expand paths like "c:foo")
                    dir = fs.getAbsolutePathName(dir);

                    // Is this actually a directory?
                    // FIXME: exceptions!
                    afl::base::Ref<afl::io::Directory> ndir = fs.openDirectory(dir);
                    // FIXME:
                    //     ndir->getEntries();
                    //     messageBox(format(m_translator("Unable to read directory %s: %s"), e.getFileName(), e.what()),

                    // Now, examine the file part
                    if (m_allowPattern && npat.hasWildcard()) {
                        // Directory and wildcard given
                        b->clearFileNamePatterns();
                        b->addFileNamePattern(npat);
                        b->openDirectory(dir);
                        setState(m_state, m_translator, *b);
                        m_result.hasNewContent = true;
                        m_result.newWildcard = file;
                    } else {
                        // No wildcard: could be file or directory name
                        afl::base::Ref<afl::io::DirectoryEntry> e = ndir->getDirectoryEntryByName(file);
                        if (e->getFileType() == afl::io::DirectoryEntry::tDirectory) {
                            // It's a directory
                            b->openDirectory(e->getPathName());
                            setState(m_state, m_translator, *b);
                            m_result.hasNewContent = true;
                        } else {
                            // Assume it's a file
                            if (!m_defaultExtension.empty()) {
                                m_result.result = util::appendFileNameExtension(fs, e->getPathName(), m_defaultExtension, false);
                            } else {
                                m_result.result = e->getPathName();
                            }
                        }
                    }
                }
            }
     private:
        std::auto_ptr<DirectoryBrowser>& m_browser;
        String_t m_input;
        bool m_allowPattern;
        String_t m_defaultExtension;
        FileSelectionDialog::State& m_state;
        FileSelectionDialog::Result& m_result;
        afl::string::Translator& m_translator;
    };
}


client::dialogs::FileSelectionDialog::FileSelectionDialog(ui::Root& root, afl::string::Translator& tx, util::RequestSender<afl::io::FileSystem> fs, String_t title)
    : m_root(root),
      m_translator(tx),
      m_fileSystem(fs),
      m_title(title),
      m_folderName(),
      m_pattern(util::FileNamePattern::getAllFilesPattern()),
      m_defaultExtension(),
      m_contentOffset(0),
      m_pHelpWidget(0),
      m_input(500, 20, root),
      m_crumbTrail(root.provider().getFont(gfx::FontRequest())->getCellSize().scaledBy(20, 1), root),
      m_fileList(2, 15, root),
      m_loop(root),
      m_link(root, tx),
      m_result(),
      m_browser()
{ }

client::dialogs::FileSelectionDialog::~FileSelectionDialog()
{ }

void
client::dialogs::FileSelectionDialog::setFolder(const String_t& folderName)
{
    m_folderName = folderName;
}

String_t
client::dialogs::FileSelectionDialog::getFolder() const
{
    // ex UIFileChooserDialog::getCurrentDirectory (sort-of)
    return m_folderName;
}

void
client::dialogs::FileSelectionDialog::setPattern(const String_t& pat)
{
    m_pattern = pat;
}

void
client::dialogs::FileSelectionDialog::setDefaultExtension(const String_t& defaultExtension)
{
    m_defaultExtension = defaultExtension;
}

void
client::dialogs::FileSelectionDialog::setHelpWidget(ui::Widget& helpWidget)
{
    m_pHelpWidget = &helpWidget;
}

String_t
client::dialogs::FileSelectionDialog::getResult() const
{
    // ex UIFileChooserDialog::getResultFile
    return m_result;
}

bool
client::dialogs::FileSelectionDialog::run()
{
    // ex UIFileChooserDialog::init (part)

    // VBox
    //   HBox
    //     "File:"
    //     InputLine
    //   FrameGroup
    //     FileListbox (+scrollbar)
    //   SimpleIconBox (directory crumbs)
    //   HBox (default dialog buttons)
    afl::base::Deleter del;
    ui::Window& win = del.addNew(new ui::Window(m_title, m_root.provider(), m_root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5));

    ui::Group& g1 = del.addNew(new ui::Group(ui::layout::HBox::instance5));
    g1.add(del.addNew(new ui::widgets::StaticText(m_translator("File:"),
                                                  util::SkinColor::Static,
                                                  gfx::FontRequest().addSize(1),
                                                  m_root.provider())));
    g1.add(m_input);
    win.add(g1);
    
    // FIXME: scrollbar
    win.add(ui::widgets::FrameGroup::wrapWidget(del, m_root.colorScheme(), ui::LoweredFrame, m_fileList));
    win.add(m_crumbTrail);

    ui::widgets::StandardDialogButtons& btns = del.addNew(new ui::widgets::StandardDialogButtons(m_root, m_translator));
    if (m_pHelpWidget != 0) {
        btns.addHelp(*m_pHelpWidget);
    }
    win.add(btns);

    // Focus
    ui::widgets::FocusIterator& fi = del.addNew(new ui::widgets::FocusIterator(ui::widgets::FocusIterator::Tab));
    win.add(fi);
    fi.add(m_input);
    fi.add(m_fileList);

    // Keys
    ui::widgets::KeyDispatcher& disp = del.addNew(new ui::widgets::KeyDispatcher());
    win.add(disp);
    disp.add(util::KeyMod_Alt + 'f', &m_input, &ui::Widget::requestFocus);
    disp.add(util::Key_Down, &m_fileList, &ui::Widget::requestFocus);
    disp.add(util::Key_Backspace, this, &FileSelectionDialog::onKeyBackspace);

    win.add(del.addNew(new ui::widgets::Quit(m_root, m_loop)));

    // Configure
    m_input.setFont(gfx::FontRequest().addSize(1));

    // Events
    btns.cancel().sig_fire.addNewClosure(m_loop.makeStop(0));
    btns.ok().sig_fire.add(this, &FileSelectionDialog::onEnter);
    m_fileList.sig_itemDoubleClick.add(this, &FileSelectionDialog::onItemDoubleClick);
    m_crumbTrail.sig_change.add(this, &FileSelectionDialog::onCrumbClick);

    win.pack();
    m_root.centerWidget(win);
    m_root.add(win);

    init();

    return m_loop.run();
}

void
client::dialogs::FileSelectionDialog::init()
{
    State state;
    InitTask t(m_browser, m_folderName, m_pattern, state, m_translator);
    m_link.call(m_fileSystem, t);
    loadState(state);
}

void
client::dialogs::FileSelectionDialog::loadState(State& state)
{
    m_fileList.swapItems(state.contentItems);
    size_t index = 0;
    if (state.selectedItem.get(index)) {
        index += state.contentOffset;
    }
    m_fileList.setCurrentIndex(index);
    m_crumbTrail.swapContent(state.crumbItems, state.crumbItems.size()-1);
    m_folderName = state.thisDirectoryName;
    m_contentOffset = state.contentOffset;
    m_input.setText(String_t());
    m_fileList.requestFocus();
}

void
client::dialogs::FileSelectionDialog::onEnter()
{
    // ex UIFileChooserDialog::onEnter
    if (m_fileList.hasState(ui::Widget::FocusedState)) {
        // Choose an item
        onItemDoubleClick(m_fileList.getCursorTop());
    } else {
        // User input
        handleUserInput(m_input.getText(), true);
    }
}

void
client::dialogs::FileSelectionDialog::onItemDoubleClick(size_t index)
{
    if (const FileListbox::Item* item = m_fileList.getItem(index)) {
        if (item->canEnter) {
            // It's a directory
            if (index < m_contentOffset) {
                handleUp(1);
            } else {
                handleChangeDirectory(index - m_contentOffset);
            }
        } else {
            // It's a file name
            handleUserInput(item->name, false);
        }
    }
}

void
client::dialogs::FileSelectionDialog::onCrumbClick(size_t index)
{
    // If last item has been clicked, do nothing, as this would mean no directory change.
    if (index != m_crumbTrail.getNumItems()-1) {
        handleUp(m_crumbTrail.getNumItems() - index - 1);
    }
}

void
client::dialogs::FileSelectionDialog::onKeyBackspace()
{
    handleUp(1);
}

void
client::dialogs::FileSelectionDialog::handleUserInput(String_t name, bool allowPattern)
{
    State state;
    Result result;
    InputTask t(m_browser, name, allowPattern, m_defaultExtension, state, result, m_translator);
    m_link.call(m_fileSystem, t);
    if (const String_t* err = result.error.get()) {
        ui::dialogs::MessageBox(*err, m_title, m_root).doOkDialog(m_translator);
    }
    if (result.hasNewContent) {
        loadState(state);
    }
    result.newWildcard.get(m_pattern);

    if (result.result.get(m_result)) {
        m_loop.stop(1);
    }
}

void
client::dialogs::FileSelectionDialog::handleUp(size_t levels)
{
    State state;
    UpTask t(m_browser, levels, state, m_translator);
    m_link.call(m_fileSystem, t);
    loadState(state);
}

void
client::dialogs::FileSelectionDialog::handleChangeDirectory(size_t index)
{
    State state;
    DownTask t(m_browser, index, state, m_translator);
    m_link.call(m_fileSystem, t);
    loadState(state);
}
