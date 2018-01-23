/**
  *  \file server/nntp/session.hpp
  */
#ifndef C2NG_SERVER_NNTP_SESSION_HPP
#define C2NG_SERVER_NNTP_SESSION_HPP

#include <map>
#include <memory>
#include "afl/string/string.hpp"
#include "afl/base/types.hpp"
#include "afl/data/value.hpp"
#include "afl/container/ptrvector.hpp"
#include "server/interface/talknntp.hpp"

namespace server { namespace nntp {

    class Session {
     public:
        Session()
            : auth_status(NeedUser),
              auth_user(),
              auth_uid(),
              m_groupListCache(),
              current_group(),
              current_forum(0),
              current_seq(0),
              current_seq_map()
            { }

        /* Authentification */
        enum AuthStatus {
            NeedUser,
            NeedPass,
            Authenticated
        };
        AuthStatus auth_status;                     /**< Status. */
        String_t auth_user;                         /**< User name. */
        String_t auth_uid;                          /**< User Id. @change This is an integer in -classic. */

        /* Group list cache. Caches the list of newsgroups.
           Since this is not expected to change, it is cached during the whole lifetime of the connection. */
        afl::container::PtrVector<server::interface::TalkNNTP::Info> m_groupListCache;

        /* Group status. We cache the sequence->message mappings.
           Since these can change often, we update these whenever a newsgroup is selected,
           and do not try to optimize. */
        String_t current_group;                     /**< Newsgroup name. */
        int32_t current_forum;                      /**< Forum number (fid). */
        int32_t current_seq;                        /**< Current sequence number. */
        std::map<int32_t,int32_t> current_seq_map;  /**< Maps sequence numbers to message numbers (mid). */

    };

} }

#endif
