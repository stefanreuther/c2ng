/**
  *  \file server/file/ca/referencecounter.hpp
  *  \brief Interface server::file::ca::ReferenceCounter
  */
#ifndef C2NG_SERVER_FILE_CA_REFERENCECOUNTER_HPP
#define C2NG_SERVER_FILE_CA_REFERENCECOUNTER_HPP

#include "afl/base/deletable.hpp"
#include "server/file/ca/objectid.hpp"

namespace server { namespace file { namespace ca {

    /** Reference counter base class.
        Files in a content-addressable pool have a reference count.
        This interface implements access to reference count storage.

        <b>Basic principles:</b>

        A reference count may get lost.
        In this case, for safety, we do not further modify it, and do not delete the referenced object.

        If we create an object anew, we can set the reference counter safely.
        Because many objects are very short-lived intermediate states, tracking those is the "90%" usecase. */
    class ReferenceCounter : public afl::base::Deletable {
     public:
        /** Set reference counter.
            \param id Object Id
            \param value New value */
        virtual void set(const ObjectId& id, int32_t value) = 0;

        /** Modify reference counter.
            Adds \c delta to the reference counter and returns the new value in \c result.
            \param id [in] Object Id
            \param delta [in] Value to add to reference counter
            \param result [out] New value of reference counter
            \retval true Operation succeeded, \c result has been set
            \retval false Operation failed, \c result has not been set */
        virtual bool modify(const ObjectId& id, int32_t delta, int32_t& result) = 0;
    };

} } }

#endif
