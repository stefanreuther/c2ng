/**
  *  \file game/pcc/gamefolder.cpp
  */

#include "game/pcc/gamefolder.hpp"
#include "game/pcc/browserhandler.hpp"
#include "game/pcc/serverdirectory.hpp"

game::pcc::GameFolder::GameFolder(BrowserHandler& handler, game::browser::Account& acc, String_t path, size_t hint)
    : m_handler(handler),
      m_account(acc),
      m_path(path),
      m_hint(hint),
      m_nullFS(),
      m_v3Loader(handler.getDefaultSpecificationDirectory(),
                 handler.profile(),
                 handler.translator(),
                 handler.log(),
                 m_nullFS)
{ }

        // Folder:
void
game::pcc::GameFolder::loadContent(afl::container::PtrVector<Folder>& /*result*/)
{
    // Nothing to load, there are no subfolders
}

afl::base::Ptr<game::Root>
game::pcc::GameFolder::loadGameRoot()
{
#if 1
    // Quick and dirty solution: pretend this to be a local folder and work with that.
    // FIXME: this needs a lot of optimisation (and quite a number of protocol improvements on server side).
    return m_v3Loader.load(*new ServerDirectory(m_handler, m_account, m_path), false);
#else
    afl::base::Ptr<afl::io::Directory> gameDirectory(new ServerDirectory(m_handler, m_account, m_path));

    // FIXME: Charset
    afl::charset::CodepageCharset charset(afl::charset::g_codepageLatin1);

    // Specification directory
    afl::base::Ptr<afl::io::MultiDirectory> spec = afl::io::MultiDirectory::create();
    spec->addDirectory(gameDirectory);
    spec->addDirectory(m_handler.getDefaultSpecificationDirectory());
                
    // Registration key
    std::auto_ptr<game::v3::RegistrationKey> key(new game::v3::RegistrationKey(charset));
    key->initFromDirectory(*gameDirectory, m_handler.log());

    // Specification loader
    afl::base::Ptr<game::SpecificationLoader> specLoader(new game::v3::SpecificationLoader(charset, m_handler.translator(), m_handler.log()));

    // FIXME: host version should be determined by server
    game::HostVersion version(game::HostVersion::PHost, MKVERSION(4,0,0));

    // Result
    afl::base::Ptr<game::Root> result =
        new game::Root(spec, gameDirectory, specLoader, version,
                       std::auto_ptr<game::RegistrationKey>(key),
                       std::auto_ptr<game::StringVerifier>(new game::v3::StringVerifier(std::auto_ptr<afl::charset::Charset>(charset.clone()))));

    // FIXME: hostConfiguration
    // FIXME: userConfiguration
    // FIXME: playerList
    // FIXME: turnLoader

    return result;
#endif
}

String_t
game::pcc::GameFolder::getName() const
{
    afl::data::Access a = getGameListEntry();
    String_t name = a("name").toString();
    if (name.empty()) {
        return m_path;
    } else {
        return name;
    }
}

util::rich::Text
game::pcc::GameFolder::getDescription() const
{
    return m_handler.translator().translateString("Server-side game directory");
}

bool
game::pcc::GameFolder::isSame(const Folder& other) const
{
    const GameFolder* p = dynamic_cast<const GameFolder*>(&other);
    return p != 0
        && &p->m_account == &m_account
        && p->m_path == m_path;
}

bool
game::pcc::GameFolder::canEnter() const
{
    return false;
}
game::pcc::GameFolder::Kind
game::pcc::GameFolder::getKind() const
{
    return kGame;
}

afl::data::Access
game::pcc::GameFolder::getGameListEntry() const
{
    afl::data::Access a = m_handler.getGameList(m_account);

    // Try the hint
    {
        afl::data::Access guess = a("reply")[m_hint];
        if (guess("path").toString() == m_path) {
            return guess;
        }
    }

    // No, find it
    for (size_t i = 0, n = a("reply").getArraySize(); i < n; ++i) {
        afl::data::Access elem = a("reply")[i];
        if (elem("path").toString() == m_path) {
            m_hint = i;
            return elem;
        }
    }
    return afl::data::Access();
}
