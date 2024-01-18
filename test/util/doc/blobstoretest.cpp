/**
  *  \file test/util/doc/blobstoretest.cpp
  *  \brief Test for util::doc::BlobStore
  */

#include "util/doc/blobstore.hpp"

#include <stdexcept>
#include "afl/test/testrunner.hpp"

/** Interface test. */
AFL_TEST_NOARG("util.doc.BlobStore")
{
    class Tester : public util::doc::BlobStore {
     public:
        virtual ObjectId_t addObject(afl::base::ConstBytes_t /*data*/)
            { return ObjectId_t(); }
        virtual afl::base::Ref<afl::io::FileMapping> getObject(const ObjectId_t& /*id*/) const
            { throw std::runtime_error("no ref"); }
    };
    Tester t;
}
