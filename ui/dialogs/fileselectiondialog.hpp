/**
  *  \file ui/dialogs/fileselectiondialog.hpp
  *  \brief Class ui::dialogs::FileSelectionDialog
  */
#ifndef C2NG_UI_DIALOGS_FILESELECTIONDIALOG_HPP
#define C2NG_UI_DIALOGS_FILESELECTIONDIALOG_HPP

#include "afl/data/stringlist.hpp"
#include "afl/io/filesystem.hpp"
#include "afl/string/string.hpp"
#include "afl/string/translator.hpp"
#include "ui/eventloop.hpp"
#include "ui/root.hpp"
#include "ui/waitindicator.hpp"
#include "ui/widgets/filelistbox.hpp"
#include "ui/widgets/inputline.hpp"
#include "ui/widgets/simpleiconbox.hpp"
#include "util/directorybrowser.hpp"
#include "util/requestsender.hpp"

namespace ui { namespace dialogs {

    /** File selection dialog.
        Asks the user to choose a file.

        To use,
        - construct
        - call setters to define parameters
        - call run()
        - if run() returns true, call getResult() to obtain the selected file name

        This class is pretty generic by only requiring access to a afl::io::FileSystem.
        However, the Downlink component currently is part of client/, therefore this class needs to live in client/. */
    class FileSelectionDialog {
     public:
        struct State;
        struct Result;

        /** Constructor.
            \param root   Root
            \param tx     Translator
            \param fs     Access to afl::io::FileSystem
            \param title  Window title */
        FileSelectionDialog(Root& root, afl::string::Translator& tx, util::RequestSender<afl::io::FileSystem> fs, String_t title);
        ~FileSelectionDialog();

        /** Set initial folder name.
            Set an empty string to start with the super-root (list of roots, afl::io::FileSystem::openRootDirectory / util::DirectoryBrowser::openRoot)
            \param folderName Folder name */
        void setFolder(const String_t& folderName);

        /** Get current folder name.
            \return folder name */
        String_t getFolder() const;

        /** Set pattern (wildcard).
            \param pat Pattern
            \see util::FileNamePattern */
        void setPattern(const String_t& pat);

        /** Add additional pattern (wildcard).
            \param pat Pattern
            \see util::FileNamePattern */
        void addPattern(const String_t& pat);

        /** Set default extension.
            If this is nonempty and the user enters a file name without extension, that extension is appended.
            \param defaultExtension Default extension, not including leading dot
            \see util::appendFileNameExtension */
        void setDefaultExtension(const String_t& defaultExtension);

        /** Set help widget.
            If you use this call, the dialog will contain a "Help" button that dispatches to this widget.
            \param helpWidget Widget; must outlive the FileSelectionDialog instance
            \see ui::widgets::StandardDialogButtons::addHelp */
        void setHelpWidget(Widget& helpWidget);

        /** Get selected path name.
            \return path name */
        String_t getResult() const;

        /** Run dialog.
            \retval true User confirmed
            \retval false User canceled */
        bool run();

     private:
        Root& m_root;
        afl::string::Translator& m_translator;
        util::RequestSender<afl::io::FileSystem> m_fileSystem;
        String_t m_title;

        String_t m_folderName;
        afl::data::StringList_t m_patterns;
        String_t m_defaultExtension;
        size_t m_contentOffset;

        Widget* m_pHelpWidget;
        ui::widgets::InputLine m_input;
        ui::widgets::SimpleIconBox m_crumbTrail;
        ui::widgets::FileListbox m_fileList;

        EventLoop m_loop;
        WaitIndicator m_link;

        String_t m_result;

        std::auto_ptr<util::DirectoryBrowser> m_browser;

        void init();
        void loadState(State& state);

        void onEnter();
        void onItemDoubleClick(size_t index);
        void onCrumbClick(size_t index);
        void onKeyBackspace();

        void handleUserInput(String_t name, bool allowPattern);
        void handleUp(size_t levels);
        void handleChangeDirectory(size_t index);
    };

} }

#endif
