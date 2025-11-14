/**
  *  \file server/play/fs/session.cpp
  *  \brief Class server::play::fs::Session
  */

#include "server/play/fs/session.hpp"
#include "game/root.hpp"
#include "game/v3/rootloader.hpp"
#include "server/interface/baseclient.hpp"
#include "server/play/fs/directory.hpp"

server::play::fs::Session::Session(afl::net::NetworkStack& net, const afl::net::Name& name, const String_t& userName)
    : m_fileClient(net, name),
      m_userName(userName)
{
    if (!m_userName.empty()) {
        server::interface::BaseClient(m_fileClient).setUserContext(m_userName);
    }
}


afl::base::Ref<server::play::fs::Session>
server::play::fs::Session::create(afl::net::NetworkStack& net, afl::net::Name name, String_t userName)
{
    return *new Session(net, name, userName);
}

afl::base::Ptr<game::Root>
server::play::fs::Session::createRoot(String_t pathName, afl::string::Translator& tx,
                                      afl::sys::LogListener& log, afl::io::FileSystem& fs,
                                      afl::base::Ref<afl::io::Directory> rootDir,
                                      afl::charset::Charset& gameCharset)
{
    /*
     *  For now, this is the minimum possible implementation.
     *
     *  This is the same code sequence as for local filesystem play.
     *  This is less efficient than it could be:
     *  - scans the directory for results although we can do that on server side
     *  - accesses spec files with a STAT/GET pair although just GET would be enough
     *  However, those are just local RPCs, so it's not that bad.
     *
     *  SAVE will directly write a TRN file; the front-end will upload that.
     *  I originally intended c2play-server to directly talk to c2host-server,
     *  but for now we can do without that. We actually don't yet have an
     *  interface to pass the result of HostTurn::submit() out of GameAccess::save().
     */
    afl::base::Ref<afl::io::Directory> gameDirectory(Directory::create(*this, pathName));
    game::v3::RootLoader loader(*rootDir, 0 /* profile */, 0 /* callback */, tx, log, fs);
    afl::base::Ref<const game::config::UserConfiguration> uc = game::config::UserConfiguration::create();
    return loader.load(gameDirectory, gameCharset, *uc, false);
}
