/**
  *  \file server/host/spec/publisherimpl.hpp
  *  \brief Class server::host::spec::PublisherImpl
  */
#ifndef C2NG_SERVER_HOST_SPEC_PUBLISHERIMPL_HPP
#define C2NG_SERVER_HOST_SPEC_PUBLISHERIMPL_HPP

#include <list>
#include "afl/io/directory.hpp"
#include "afl/net/commandhandler.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/loglistener.hpp"
#include "game/root.hpp"
#include "game/spec/shiplist.hpp"
#include "server/host/spec/publisher.hpp"

namespace server { namespace host { namespace spec {

    /** Production implementation of Publisher.
        This is a potentially long-lived object to implement retrieval of specification data.
        When given a path name, it will load the ship list data available under that path
        (completing it with default specifications), and produce data. */
    class PublisherImpl : public Publisher {
     public:
        /** Constructor.
            @param defaultSpecificationDirectory Directory containing default specification files
            @param hostFile                      Host file server
            @param log                           Logger. Will receive errors/warnings from loading spec data. */
        PublisherImpl(afl::base::Ref<afl::io::Directory> defaultSpecificationDirectory,
                      afl::net::CommandHandler& hostFile,
                      afl::sys::LogListener& log);

        /** Invalidate cache.
            The original plan was to selectively invalidate individual cache elements.
            For now, we just invalidate everything.
            This means we need not worry about interdependencies (change to FLAK tool invalidating a game),
            and invalidation errors have a higher chance to heal themselves. */
        void invalidateCache();

        // Publisher:
        afl::data::Hash::Ref_t getSpecificationData(String_t pathName, String_t flakPath, const afl::data::StringList_t& keys);

     private:
        afl::base::Ref<afl::io::Directory> m_defaultSpecificationDirectory;
        afl::net::CommandHandler& m_hostFile;
        afl::sys::LogListener& m_log;

        /** Null translator. This will affect interpretation of multilingual files (but also error messages).
            Must be long-lived because cache entries can keep a reference to it. */
        afl::string::NullTranslator m_translator;

        /*
         *  Cache
         *
         *  Test case host/p27_spec.pl reports ~10400 us per 'specshiplist' command without caching, ~470 us with caching.
         *  This makes it worthwhile to implement.
         *  In particular, one intended usecase is having individual pages for each ship type,
         *  which means pages for a single ship list take >1 s without caching, ~50 ms with caching.
         */
        struct CacheNode {
            String_t pathName;
            String_t flakPath;
            afl::base::Ref<game::Root> root;
            afl::base::Ref<game::spec::ShipList> shipList;

            CacheNode(const String_t& pathName,
                      const String_t& flakPath,
                      const afl::base::Ref<game::Root>& root,
                      const afl::base::Ref<game::spec::ShipList>& shipList)
                : pathName(pathName), flakPath(flakPath), root(root), shipList(shipList)
                { }
        };
        typedef std::list<CacheNode> CacheList_t;
        CacheList_t m_cache;

        CacheList_t::iterator find(const String_t& pathName, const String_t& flakPath);
    };

} } }

#endif
