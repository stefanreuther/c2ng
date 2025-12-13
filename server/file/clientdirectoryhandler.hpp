/**
  *  \file server/file/clientdirectoryhandler.hpp
  *  \brief Class server::file::ClientDirectoryHandler
  */
#ifndef C2NG_SERVER_FILE_CLIENTDIRECTORYHANDLER_HPP
#define C2NG_SERVER_FILE_CLIENTDIRECTORYHANDLER_HPP

#include "server/file/directoryhandler.hpp"
#include "afl/net/commandhandler.hpp"

namespace server { namespace file {

    /** Implementation of DirectoryHandler using a (remote) server::interface::FileBase implementation as a back-end.
        This uses a FileBaseClient to talk to the remote side via a CommandHandler.
        The CommandHandler typically is a network transport.

        This is mainly intended for short-lived operations uses with c2fileclient.
        Although it can also be used to make a c2file instance proxy another, that is not very reliable:
        - ClientDirectoryHandler cannot sensibly deal with connection loss.
        - there is no meaningful way to notify a client of changes in the underlying file store.

        <b>Note:</b> FileBase cannot normally list the nameless directory ("LS ''").
        Therefore, this DirectoryHandler cannot make a complete c2file instance accessible, only a subdirectory tree of it. */
    class ClientDirectoryHandler : public DirectoryHandler {
     public:
        /** Constructor.
            \param commandHandler CommandHandler to talk to a FileBase instance
            \param basePath Base path. Should not be empty; the commandHandler must be able to answer a "LS basePath". */
        ClientDirectoryHandler(afl::net::CommandHandler& commandHandler, String_t basePath);

        // DirectoryHandler implementation:
        virtual String_t getName();
        virtual afl::base::Ref<afl::io::FileMapping> getFile(const Info& info);
        virtual afl::base::Ref<afl::io::FileMapping> getFileByName(String_t name);
        virtual Info createFile(String_t name, afl::base::ConstBytes_t content);
        virtual void removeFile(String_t name);
        virtual void readContent(Callback& callback);
        virtual DirectoryHandler* getDirectory(const Info& info);
        virtual Info createDirectory(String_t name);
        virtual void removeDirectory(String_t name);
        virtual afl::base::Optional<Info> copyFile(ReadOnlyDirectoryHandler& source, const Info& sourceInfo, String_t name);
        virtual SnapshotHandler* getSnapshotHandler();
        virtual afl::base::Ptr<afl::io::Directory> getDirectory();

     private:
        String_t makePath(String_t userPath);

        afl::net::CommandHandler& m_commandHandler;
        const String_t m_basePath;
    };

} }

#endif
