/**
  *  \file client/dialogs/fileselectiondialog.hpp
  */
#ifndef C2NG_CLIENT_DIALOGS_FILESELECTIONDIALOG_HPP
#define C2NG_CLIENT_DIALOGS_FILESELECTIONDIALOG_HPP

#include "ui/root.hpp"
#include "afl/string/string.hpp"
#include "util/requestsender.hpp"
#include "ui/widgets/inputline.hpp"
#include "ui/widgets/simpleiconbox.hpp"
#include "client/widgets/filelistbox.hpp"
#include "ui/eventloop.hpp"
#include "util/directorybrowser.hpp"
#include "client/downlink.hpp"
#include "afl/io/filesystem.hpp"

namespace client { namespace dialogs {

    class FileSelectionDialog {
     public:
        struct State;
        struct Result;

        FileSelectionDialog(ui::Root& root, util::RequestSender<afl::io::FileSystem> fs, String_t title);

        void setFolder(const String_t& folderName);
        String_t getFolder() const;
        void setPattern(const String_t& pat);

        String_t getResult() const;

        bool run();

     private:

        ui::Root& m_root;
        util::RequestSender<afl::io::FileSystem> m_fileSystem;
        String_t m_title;

        String_t m_folderName;
        String_t m_pattern;
        size_t m_contentOffset;

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
