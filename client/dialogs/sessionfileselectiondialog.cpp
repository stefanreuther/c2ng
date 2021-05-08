/**
  *  \file client/dialogs/sessionfileselectiondialog.cpp
  *  \brief Class client::dialogs::SessionFileSelectionDialog
  */

#include "client/dialogs/sessionfileselectiondialog.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/values.hpp"

namespace {
    const char*const UI_DIRECTORY = "UI.DIRECTORY";

    class SessionFileSystem : public afl::base::Closure<afl::io::FileSystem& (game::Session&)> {
     public:
        virtual afl::io::FileSystem& call(game::Session& session)
            { return session.world().fileSystem(); }
        virtual SessionFileSystem* clone() const
            { return new SessionFileSystem(); }
    };
}

client::dialogs::SessionFileSelectionDialog::SessionFileSelectionDialog(ui::Root& root, afl::string::Translator& tx, util::RequestSender<game::Session> gameSender, String_t title)
    : FileSelectionDialog(root, tx, gameSender.convert(new SessionFileSystem()), title),
      m_gameSender(gameSender)
{ }

client::dialogs::SessionFileSelectionDialog::~SessionFileSelectionDialog()
{ }

void
client::dialogs::SessionFileSelectionDialog::setFolderFromSession(game::proxy::WaitIndicator& ind)
{
    class Task : public util::Request<game::Session> {
     public:
        Task()
            : m_result()
            { }
        virtual void handle(game::Session& session)
            { interpreter::checkStringArg(m_result, session.world().getGlobalValue(UI_DIRECTORY)); }
        const String_t& getResult() const
            { return m_result; }
     private:
        String_t m_result;
    };

    Task t;
    ind.call(m_gameSender, t);
    setFolder(t.getResult());
}

void
client::dialogs::SessionFileSelectionDialog::storeFolderInSession()
{
    class Task : public util::Request<game::Session> {
     public:
        Task(String_t folderName)
            : m_folderName(folderName)
            { }
        virtual void handle(game::Session& session)
            { session.world().setNewGlobalValue(UI_DIRECTORY, interpreter::makeStringValue(m_folderName)); }
     private:
        String_t m_folderName;
    };
    m_gameSender.postNewRequest(new Task(getFolder()));
}

bool
client::dialogs::SessionFileSelectionDialog::runDefault(game::proxy::WaitIndicator& ind)
{
    // PCC1 and PCC2 update the 'UI.DIRECTORY' variable on every folder change.
    // We therefore update it upon every exit from the dialog, successful or not.
    setFolderFromSession(ind);
    bool result = run();
    storeFolderInSession();
    return result;
}
