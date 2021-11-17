/**
  *  \file u/t_util_doc_blobstore.cpp
  *  \brief Test for util::doc::BlobStore
  */

#include "util/doc/blobstore.hpp"

#include "t_util_doc.hpp"

/** Interface test. */
void
TestUtilDocBlobStore::testInterface()
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

