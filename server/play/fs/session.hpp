/**
  *  \file server/play/fs/session.hpp
  *  \brief Class server::play::fs::Session
  */
#ifndef C2NG_SERVER_PLAY_FS_SESSION_HPP
#define C2NG_SERVER_PLAY_FS_SESSION_HPP

#include "afl/base/ptr.hpp"
#include "afl/base/ref.hpp"
#include "afl/base/refcounted.hpp"
#include "afl/charset/charset.hpp"
#include "afl/io/directory.hpp"
#include "afl/io/filesystem.hpp"
#include "afl/net/name.hpp"
#include "afl/net/networkstack.hpp"
#include "afl/net/resp/client.hpp"
#include "afl/string/translator.hpp"
#include "afl/sys/loglistener.hpp"
#include "game/root.hpp"

namespace server { namespace play { namespace fs {

    /** File-server play session.
        Stores the state (network connections) for server-based play. */
    class Session : public afl::base::RefCounted {
     public:
        /** Create session.
            \param net       Network stack
            \param name      Network name of file server (host, port)
            \param userName  User Id */
        static afl::base::Ref<Session> create(afl::net::NetworkStack& net, afl::net::Name name, String_t userName);

        /** Access file client.
            \return file client */
        afl::net::resp::Client& fileClient()
            { return m_fileClient; }

        /** Create game root.
            \param pathName Path name on file server, starting with "/"
            \param tx       Translator
            \param log      Logger
            \param fs       File system for backup access (pass a NullFileSystem)
            \param rootDir  Root directory (specifications)
            \param gameCharset Game character set
            \return root; can be null */
        afl::base::Ptr<game::Root> createRoot(String_t pathName, afl::string::Translator& tx,
                                              afl::sys::LogListener& log, afl::io::FileSystem& fs,
                                              afl::base::Ref<afl::io::Directory> rootDir,
                                              afl::charset::Charset& gameCharset);


     private:
        Session(afl::net::NetworkStack& net, const afl::net::Name& name, const String_t& userName);

        afl::net::resp::Client m_fileClient;
        String_t m_userName;
    };

} } }

#endif
