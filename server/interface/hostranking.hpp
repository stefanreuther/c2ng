/**
  *  \file server/interface/hostranking.hpp
  *  \brief Interface server::interface::HostRanking
  */
#ifndef C2NG_SERVER_INTERFACE_HOSTRANKING_HPP
#define C2NG_SERVER_INTERFACE_HOSTRANKING_HPP

#include "server/types.hpp"
#include "afl/base/deletable.hpp"
#include "afl/base/optional.hpp"
#include "afl/data/stringlist.hpp"

namespace server { namespace interface {

    /** Host ranking interface.
        Access the user ranking list. */
    class HostRanking : public afl::base::Deletable {
     public:
        struct ListRequest {
            afl::base::Optional<String_t> sortField;
            bool sortReverse;
            afl::data::StringList_t fieldsToGet;

            ListRequest()
                : sortField(), sortReverse(false), fieldsToGet()
                { }
        };

        /** Get list of users (RANKLIST).
            \param req Request
            \return raw result; a list containing alternating user Ids and list-of-fields. */
        virtual Value_t* getUserList(const ListRequest& req) = 0;
    };

} }

#endif
