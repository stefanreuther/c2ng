/**
  *  \file test/server/file/ca/objectcachetest.cpp
  *  \brief Test for server::file::ca::ObjectCache
  */

#include "server/file/ca/objectcache.hpp"
#include "afl/test/testrunner.hpp"

/** Interface test. */
AFL_TEST_NOARG("server.file.ca.ObjectCache")
{
    class Tester : public server::file::ca::ObjectCache {
     public:
        virtual void addObject(const server::file::ca::ObjectId& /*id*/, server::file::ca::ObjectStore::Type /*type*/, afl::base::Ref<afl::io::FileMapping> /*content*/)
            { }
        virtual void addObjectSize(const server::file::ca::ObjectId& /*id*/, server::file::ca::ObjectStore::Type /*type*/, size_t /*size*/)
            { }
        virtual void removeObject(const server::file::ca::ObjectId& /*id*/)
            { }
        virtual afl::base::Ptr<afl::io::FileMapping> getObject(const server::file::ca::ObjectId& /*id*/, server::file::ca::ObjectStore::Type /*type*/)
            { return 0; }
        virtual afl::base::Optional<size_t> getObjectSize(const server::file::ca::ObjectId& /*id*/, server::file::ca::ObjectStore::Type /*type*/)
            { return afl::base::Optional<size_t>(); }
    };
    Tester t;
}
