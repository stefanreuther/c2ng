/**
  *  \file game/pcc/serverdirectory.cpp
  */

#include "game/pcc/serverdirectory.hpp"
#include "afl/except/fileproblemexception.hpp"
#include "afl/io/directoryentry.hpp"
#include "afl/io/internalstream.hpp"
#include "afl/net/http/downloadlistener.hpp"
#include "afl/net/http/simpledownloadlistener.hpp"
#include "afl/string/messages.hpp"
#include "afl/string/posixfilenames.hpp"
#include "game/pcc/browserhandler.hpp"
#include "afl/string/format.hpp"

/*
 *  DirectoryEntry implementation
 */
class game::pcc::ServerDirectory::Entry : public afl::io::DirectoryEntry {
 public:
    // Construct from container and JSON data (existing files)
    Entry(afl::base::Ref<ServerDirectory> container, afl::data::Access data)
        : m_container(container),
          m_title(data("name").toString()),
          m_url()
        {
            String_t type = data("type").toString();
            if (type == "file") {
                setFileType(tFile);
                setFileSize(data("size").toInteger());
                m_url = data("url").toString();
            } else if (type == "dir") {
                setFileType(tDirectory);
            } else {
                setFileType(tUnknown);
            }
        }

    // Construct from container and name (nonexistant files)
    Entry(afl::base::Ref<ServerDirectory> container, String_t title)
        : m_container(container),
          m_title(title),
          m_url()
        {
            setFileType(tUnknown);
        }

    // Get title (user-friendly name).
    String_t getTitle()
        { return m_title; }

    // Get path name. Empty for virtual files.
    String_t getPathName()
        { return String_t(); }

    // Open as file. For now, we can only open for reading, and download the file completely.
    afl::base::Ref<afl::io::Stream> openFile(afl::io::FileSystem::OpenMode mode)
        {
            if (getFileType() != tFile) {
                throw afl::except::FileProblemException(m_title, afl::string::Messages::fileNotFound());
            }
            if (mode != afl::io::FileSystem::OpenRead) {
                throw afl::except::FileProblemException(m_title, afl::string::Messages::cannotWrite());
            }

            // Download the file
            afl::net::http::SimpleDownloadListener listener;
            m_container->m_handler.getFile(m_container->m_account, m_url, listener);

            switch (listener.wait()) {
             case afl::net::http::SimpleDownloadListener::Succeeded:
                break;
             case afl::net::http::SimpleDownloadListener::Failed:
             case afl::net::http::SimpleDownloadListener::TimedOut:
             case afl::net::http::SimpleDownloadListener::LimitExceeded:
                throw afl::except::FileProblemException(m_title, afl::string::Messages::networkError());
            }

            // Create InternalStream object for user to work with
            afl::base::Ref<afl::io::InternalStream> s(*new afl::io::InternalStream());
            s->setName(m_title);
            s->write(listener.getResponseData());
            s->setPos(0);
            return s;
        }

    // Open as directory.
    afl::base::Ref<afl::io::Directory> openDirectory()
        {
            if (getFileType() == tDirectory) {
                return *new ServerDirectory(m_container->m_handler,
                                            m_container->m_account,
                                            afl::string::PosixFileNames().makePathName(m_container->m_name, m_title));
            } else {
                throw afl::except::FileProblemException(m_title, afl::string::Messages::fileNotFound());
            }
        }

    // Open container.
    afl::base::Ref<afl::io::Directory> openContainingDirectory()
        { return m_container; }

    // Update info. All available info has been provided in the constructor, so nothing to do here.
    void updateInfo(uint32_t /*requested*/)
        { }

    // rename; not supported yet.
    void doRename(String_t /*newName*/)
        { throw afl::except::FileProblemException(m_title, afl::string::Messages::cannotWrite()); }

    // erase; not supported yet.
    void doErase()
        { throw afl::except::FileProblemException(m_title, afl::string::Messages::cannotWrite()); }

    // mkdir; not supported yet.
    void doCreateAsDirectory()
        { throw afl::except::FileProblemException(m_title, afl::string::Messages::cannotWrite()); }

 private:
    afl::base::Ref<ServerDirectory> m_container;
    String_t m_title;
    String_t m_url;
};

/**************************** ServerDirectory ****************************/

game::pcc::ServerDirectory::ServerDirectory(BrowserHandler& handler,
                                            game::browser::Account& acc,
                                            String_t name)
    : m_handler(handler),
      m_account(acc),
      m_name(name),
      m_loaded(false),
      m_entries()
{ }

game::pcc::ServerDirectory::~ServerDirectory()
{ }

afl::base::Ref<afl::io::DirectoryEntry>
game::pcc::ServerDirectory::getDirectoryEntryByName(String_t name)
{
    // Load content
    load();

    // If there is a matching directory entry, use that
    for (size_t i = 0, n = m_entries->size(); i < n; ++i) {
        if ((*m_entries)[i]->getTitle() == name) {
            return *(*m_entries)[i];
        }
    }

    // Mone found; make empty one
    return *new Entry(*this, name);
}

afl::base::Ref<afl::base::Enumerator<afl::base::Ptr<afl::io::DirectoryEntry> > >
game::pcc::ServerDirectory::getDirectoryEntries()
{
    // Load the directory listing and provide it to user from memory.
    class Enum : public afl::base::Enumerator<afl::base::Ptr<afl::io::DirectoryEntry> > {
     public:
        Enum(afl::base::Ptr<ContentVector_t> p)
            : m_p(p),
              m_index(0)
            { }
        virtual bool getNextElement(afl::base::Ptr<afl::io::DirectoryEntry>& result)
            {
                if (m_index < m_p->size()) {
                    result = (*m_p)[m_index++];
                    return true;
                } else {
                    return false;
                }
            }
     private:
        afl::base::Ptr<ContentVector_t> m_p;
        size_t m_index;
    };

    load();
    return *new Enum(m_entries);
}

afl::base::Ptr<afl::io::Directory>
game::pcc::ServerDirectory::getParentDirectory()
{
    String_t parentName = afl::string::PosixFileNames().getDirectoryName(m_name);
    if (parentName != ".") {
        return new ServerDirectory(m_handler, m_account, parentName);
    } else {
        return 0;
    }
}

String_t
game::pcc::ServerDirectory::getDirectoryName()
{
    return String_t();
}

String_t
game::pcc::ServerDirectory::getTitle()
{
    return afl::string::PosixFileNames().getFileName(m_name);
}

void
game::pcc::ServerDirectory::load()
{
    // FIXME: validate TTL
    if (m_loaded) {
        return;
    }

    m_loaded = true;
    m_entries = new ContentVector_t();

    std::auto_ptr<afl::data::Value> content(m_handler.getDirectoryContent(m_account, m_name));
    afl::data::Access a(content);
    if (a("result").toInteger()) {
        for (size_t i = 0, n = a("reply").getArraySize(); i < n; ++i) {
            m_entries->push_back(new Entry(*this, a("reply")[i]));
        }
    } else {
        String_t error = a("error").toString();
        if (error.empty()) {
            throw afl::except::FileProblemException(m_name, afl::string::Messages::networkError());
        } else {
            throw afl::except::FileProblemException(m_name, afl::string::Format(m_handler.translator().translateString("The server reported an error: %s").c_str(), error));
        }
    }
}
