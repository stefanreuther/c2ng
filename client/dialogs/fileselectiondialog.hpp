/**
  *  \file client/dialogs/fileselectiondialog.hpp
  *  \brief Class client::dialogs::FileSelectionDialog
  */
#ifndef C2NG_CLIENT_DIALOGS_FILESELECTIONDIALOG_HPP
#define C2NG_CLIENT_DIALOGS_FILESELECTIONDIALOG_HPP

#include "afl/io/filesystem.hpp"
#include "afl/string/string.hpp"
#include "afl/string/translator.hpp"
#include "client/downlink.hpp"
#include "client/widgets/filelistbox.hpp"
#include "ui/eventloop.hpp"
#include "ui/root.hpp"
#include "ui/widgets/inputline.hpp"
#include "ui/widgets/simpleiconbox.hpp"
#include "util/directorybrowser.hpp"
#include "util/requestsender.hpp"

namespace client { namespace dialogs {

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
        FileSelectionDialog(ui::Root& root, afl::string::Translator& tx, util::RequestSender<afl::io::FileSystem> fs, String_t title);
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

        /** Set default extension.
            If this is nonempty and the user enters a file name without extension, that extension is appended.
            \param defaultExtension Default extension, not including leading dot
            \see util::appendFileNameExtension */
        void setDefaultExtension(const String_t& defaultExtension);

        /** Set help widget.
            If you use this call, the dialog will contain a "Help" button that dispatches to this widget.
            \param helpWidget Widget; must outlive the FileSelectionDialog instance
            \see ui::widgets::StandardDialogButtons::addHelp */
        void setHelpWidget(ui::Widget& helpWidget);

        /** Get selected path name.
            \return path name */
        String_t getResult() const;

        /** Run dialog.
            \retval true User confirmed
            \retval false User canceled */
        bool run();

     private:
        ui::Root& m_root;
        afl::string::Translator& m_translator;
        util::RequestSender<afl::io::FileSystem> m_fileSystem;
        String_t m_title;

        String_t m_folderName;
        String_t m_pattern;
        String_t m_defaultExtension;
        size_t m_contentOffset;

        ui::Widget* m_pHelpWidget;
        ui::widgets::InputLine m_input;
        ui::widgets::SimpleIconBox m_crumbTrail;
        client::widgets::FileListbox m_fileList;

        ui::EventLoop m_loop;
        Downlink m_link;

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
