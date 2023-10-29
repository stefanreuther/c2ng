/**
  *  \file server/host/spec/publisherimpl.cpp
  *  \brief Class server::host::spec::PublisherImpl
  */

#include "server/host/spec/publisherimpl.hpp"
#include "afl/charset/codepage.hpp"
#include "afl/charset/codepagecharset.hpp"
#include "afl/data/hash.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/vectorvalue.hpp"
#include "afl/io/multidirectory.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/root.hpp"
#include "game/task.hpp"
#include "game/v3/loader.hpp"
#include "game/v3/registrationkey.hpp"
#include "game/v3/specificationloader.hpp"
#include "game/v3/stringverifier.hpp"
#include "game/v3/utils.hpp"
#include "server/errors.hpp"
#include "server/host/spec/directory.hpp"
#include "server/interface/baseclient.hpp"
#include "server/interface/filebaseclient.hpp"
#include "server/play/basichullfunctionpacker.hpp"
#include "server/play/beampacker.hpp"
#include "server/play/configurationpacker.hpp"
#include "server/play/enginepacker.hpp"
#include "server/play/flakconfigurationpacker.hpp"
#include "server/play/friendlycodepacker.hpp"
#include "server/play/hullpacker.hpp"
#include "server/play/torpedopacker.hpp"
#include "server/play/truehullpacker.hpp"
#include "util/stringparser.hpp"

namespace {
    const size_t MAX_CACHE_SIZE = 10;

    /*
     *  Helpers
     */

    /* Reference to a Directory that automatically disables it.
       We do not want a non-disabled directory to hang around, even if exiting nonlocally. */
    class DirectoryRef {
     public:
        DirectoryRef(afl::base::Ref<server::host::spec::Directory> ref)
            : m_ref(ref)
            { }
        ~DirectoryRef()
            { m_ref->setEnabled(false); }

        const afl::base::Ref<server::host::spec::Directory>& get() const
            { return m_ref; }

     private:
        afl::base::Ref<server::host::spec::Directory> m_ref;
    };


    /*
     *  Constructors
     */

    /* Make character set.
       Our shiplists do not use international characters, but we need a Charset implementation anyway.
       If international characters exist, treat them as 437 because they most likely originate on DOS. */
    std::auto_ptr<afl::charset::Charset> makeCharset()
    {
        return std::auto_ptr<afl::charset::Charset>(new afl::charset::CodepageCharset(afl::charset::g_codepage437));
    }

    /* Make a registration key.
       Not directly needed but might affect some hullfuncs someday? */
    std::auto_ptr<game::RegistrationKey> makeKey()
    {
        std::auto_ptr<game::v3::RegistrationKey> key(new game::v3::RegistrationKey(makeCharset()));
        key->initFromValues("c2host", "Specification Publisher");
        return std::auto_ptr<game::RegistrationKey>(key);
    }


    /*
     *  Value builder
     */

    /* Build value for hullspec */
    server::Value_t* buildHullSpecification(game::spec::ShipList& sl, game::Root& root)
    {
        afl::data::Vector::Ref_t v = afl::data::Vector::create();
        for (int i = 1, n = sl.hulls().size(); i <= n; ++i) {
            if (sl.hulls().get(i) != 0) {
                v->pushBackNew(server::play::HullPacker(sl, root, i).buildValue());
            } else {
                v->pushBackNew(0);
            }
        }
        return new afl::data::VectorValue(v);
    }

    /* Build value for a single key */
    server::Value_t* buildValue(game::spec::ShipList& sl, game::Root& root, util::StringParser& key, afl::string::Translator& tx)
    {
        int n;
        if (key.parseString("beamspec")) {
            return server::play::BeamPacker(sl, root, 1).buildValue();
        } else if (key.parseString("config")) {
            return server::play::ConfigurationPacker(root, 0).buildValue();
        } else if (key.parseString("engspec")) {
            return server::play::EnginePacker(sl, 1).buildValue();
        } else if (key.parseString("fcodes")) {
            return server::play::FriendlyCodePacker(sl, root, tx).buildValue();
        } else if (key.parseString("flakconfig")) {
            return server::play::FlakConfigurationPacker(root).buildValue();
        } else if (key.parseString("torpspec")) {
            return server::play::TorpedoPacker(sl, root, 1).buildValue();
        } else if (key.parseString("truehull")) {
            return server::play::TruehullPacker(sl, root, 1).buildValue();
        } else if (key.parseString("hullfunc")) {
            return server::play::BasicHullFunctionPacker(sl).buildValue();
        } else if (key.parseString("hullspec")) {
            return buildHullSpecification(sl, root);
        } else if (key.parseString("hull") && key.parseInt(n)) {
            if (sl.hulls().get(n) != 0) {
                return server::play::HullPacker(sl, root, n).buildValue();
            } else {
                throw std::runtime_error(server::INVALID_KEY);
            }
        } else if (key.parseString("result")) {
            // Our API frontend wants to produce a field "result".
            // Adding it here means it doesn't have to dissect the JSON and can just pass it on.
            return server::makeIntegerValue(1);
        } else {
            throw std::runtime_error(server::INVALID_KEY);
        }
    }
}


/*
 *  PublisherImpl
 */

server::host::spec::PublisherImpl::PublisherImpl(afl::base::Ref<afl::io::Directory> defaultSpecificationDirectory,
                                                 afl::net::CommandHandler& hostFile,
                                                 afl::sys::LogListener& log)
    : m_defaultSpecificationDirectory(defaultSpecificationDirectory),
      m_hostFile(hostFile),
      m_log(log)
{ }

void
server::host::spec::PublisherImpl::invalidateCache()
{
    m_cache.clear();
}

afl::data::Hash::Ref_t
server::host::spec::PublisherImpl::getSpecificationData(String_t pathName, String_t flakPath, const afl::data::StringList_t& keys)
{
    CacheList_t::iterator it = find(pathName, flakPath);
    if (it == m_cache.end()) {
        // Disable access checking. Caller checks (and once we start caching, user permission checks at this place might interact badly with it).
        server::interface::BaseClient(m_hostFile).setUserContext(String_t());

        // Create directory implementation
        server::interface::FileBaseClient filer(m_hostFile);
        DirectoryRef gameDir(Directory::create(filer, pathName));

        // FLAK
        std::auto_ptr<DirectoryRef> flakDir;
        if (!flakPath.empty()) {
            flakDir.reset(new DirectoryRef(Directory::create(filer, flakPath)));
        }

        // Create SpecificationLoader
        afl::base::Ref<afl::io::MultiDirectory> specDir = afl::io::MultiDirectory::create();
        specDir->addDirectory(gameDir.get());
        specDir->addDirectory(m_defaultSpecificationDirectory);
        if (flakDir.get() != 0) {
            specDir->addDirectory(flakDir->get());
        }
        afl::base::Ref<game::v3::SpecificationLoader> specLoader(*new game::v3::SpecificationLoader(specDir, makeCharset(), m_translator, m_log));

        // Create Root
        // TODO: for now, version is hardcoded as PHost. When loading a game, maybe take from that.
        afl::base::Ref<game::Root> root(*new game::Root(gameDir.get(), specLoader, game::HostVersion(game::HostVersion::PHost, MKVERSION(4,2,0)),
                                                        makeKey(), std::auto_ptr<game::StringVerifier>(new game::v3::StringVerifier(makeCharset())),
                                                        makeCharset(), game::Root::Actions_t()));
        game::v3::loadRaceNames(root->playerList(), *specDir, root->charset());
        game::v3::Loader(root->charset(), m_translator, m_log).loadConfiguration(*root, *specDir);

        // Load
        afl::base::Ref<game::spec::ShipList> shipList(*new game::spec::ShipList());
        bool loadResult = false;
        specLoader->loadShipList(*shipList, *root, game::makeResultTask(loadResult))->call();

        // Save in cache
        while (m_cache.size() > MAX_CACHE_SIZE) {
            m_cache.pop_back();
        }
        m_cache.push_front(CacheNode(pathName, flakPath, root, shipList));
        it = m_cache.begin();
    } else {
        // Move to front
        m_cache.splice(m_cache.begin(), m_cache, it);
    }

    // Build result
    afl::data::Hash::Ref_t result = afl::data::Hash::create();
    for (size_t i = 0; i < keys.size(); ++i) {
        util::StringParser p(keys[i]);
        result->setNew(keys[i], buildValue(*it->shipList, *it->root, p, m_translator));
        if (!p.parseEnd()) {
            throw std::runtime_error(INVALID_KEY);
        }
    }
    return result;
}

server::host::spec::PublisherImpl::CacheList_t::iterator
server::host::spec::PublisherImpl::find(const String_t& pathName, const String_t& flakPath)
{
    for (CacheList_t::iterator it = m_cache.begin(); it != m_cache.end(); ++it) {
        if (it->pathName == pathName && it->flakPath == flakPath) {
            return it;
        }
    }
    return m_cache.end();
}
