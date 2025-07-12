/**
  *  \file server/file/clientdirectory.hpp
  *  \brief Class server::file::ClientDirectory
  */
#ifndef C2NG_SERVER_FILE_CLIENTDIRECTORY_HPP
#define C2NG_SERVER_FILE_CLIENTDIRECTORY_HPP

#include "afl/io/directory.hpp"
#include "afl/io/directoryentry.hpp"
#include "afl/net/commandhandler.hpp"
#include "server/interface/filebase.hpp"
#include "afl/base/ref.hpp"

namespace server { namespace file {

    /** Access a FileBase service as a afl::io::Directory.
        Use this to call code that needs a Directory when you have a (afl::net::CommandHandler that points to a) FileBase.
        This implements read-only access, and does not attempt to meaningfully handle parallel modifications to the file space. */
    class ClientDirectory : public afl::io::Directory {
     public:
        /** Constructor.
            \param commandHandler CommandHandler to talk to a FileBase instance
            \param basePath Base path. Should not be empty; the commandHandler must be able to answer a "LS basePath". */
        static afl::base::Ref<ClientDirectory> create(afl::net::CommandHandler& commandHandler, String_t basePath);

        /** Destructor. */
        ~ClientDirectory();

        // Directory:
        virtual afl::base::Ref<afl::io::DirectoryEntry> getDirectoryEntryByName(String_t name);
        virtual afl::base::Ref<afl::base::Enumerator<afl::base::Ptr<afl::io::DirectoryEntry> > > getDirectoryEntries();
        virtual afl::base::Ptr<afl::io::Directory> getParentDirectory();
        virtual String_t getDirectoryName();
        virtual String_t getTitle();
        virtual void flush();

     private:
        /** Constructor.
            Same as create(). */
        ClientDirectory(afl::net::CommandHandler& commandHandler, String_t basePath);

        /** Constructor for child directory.
            Same as create(), but sets the value for getParentDirectory(). */
        ClientDirectory(afl::net::CommandHandler& commandHandler, String_t basePath, afl::base::Ptr<ClientDirectory> parent);

        class Entry;
        class Enum;

        afl::net::CommandHandler& m_commandHandler;
        String_t m_basePath;
        afl::base::Ptr<ClientDirectory> m_parent;
    };

} }

#endif
