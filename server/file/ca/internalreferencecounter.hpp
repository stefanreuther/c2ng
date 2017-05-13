/**
  *  \file server/file/ca/internalreferencecounter.hpp
  *  \brief Class server::file::ca::InternalReferenceCounter
  */
#ifndef C2NG_SERVER_FILE_CA_INTERNALREFERENCECOUNTER_HPP
#define C2NG_SERVER_FILE_CA_INTERNALREFERENCECOUNTER_HPP

#include <map>
#include "server/file/ca/referencecounter.hpp"

namespace server { namespace file { namespace ca {

    /** Internal (in-memory) reference counter.
        This only tracks objects created in this lifetime of the service.
        It makes no attempt at persisting reference counts across runs.

        This is the minimum reference counting implementation that shall be used with an ObjectStore
        because it gets rid of short-lived temporary objects.
        Each file update creates many of these.

        <b>Usage statistic 20170314:</b>

        Test case "import hostfile data" ("c2fileclient cp -r INPUT ca:OUTPUT")
        - 291568 user files / 20170224-hostdata
        - 7367168k user data (du -sk)
        - Without reference counting
          - 47 minutes conversion time on 'rocket'
          - 1865827 objects (=write amplification of 6.4)
          - 8340792k object data (du -sk)
          - 1947441k effective object data (file size)
        - <b>With reference counting:</b>
          - 27 minutes conversion time on 'rocket' (~6:30 CPU)
          - 113772 objects (55% savings)
          - 925968k object data (du -sk)
          - 615114k effective object data (file size)
          - 9725501440 bytes written, 7566229504 cancelled (=short-lived objects)

        For reference, git produces 113767 objects (fewer because it does not store empty directories),
        and 982452k object data (because it happens to compress differently) when the original 7.3 GB
        are imported in a single commit. */
    class InternalReferenceCounter : public ReferenceCounter {
     public:
        InternalReferenceCounter();
        virtual void set(const ObjectId& id, int32_t value);
        virtual bool modify(const ObjectId& id, int32_t delta, int32_t& result);

     private:
        typedef std::map<ObjectId, int32_t> Map_t;
        Map_t m_data;
    };

} } }

#endif
