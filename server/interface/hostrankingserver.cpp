/**
  *  \file server/interface/hostrankingserver.cpp
  *  \brief Class server::interface::HostRankingServer
  */

#include <stdexcept>
#include "server/interface/hostrankingserver.hpp"
#include "server/interface/hostranking.hpp"
#include "afl/string/string.hpp"
#include "server/errors.hpp"

server::interface::HostRankingServer::HostRankingServer(HostRanking& impl)
    : m_implementation(impl)
{ }

server::interface::HostRankingServer::~HostRankingServer()
{ }

bool
server::interface::HostRankingServer::handleCommand(const String_t& upcasedCommand, interpreter::Arguments& args, std::auto_ptr<Value_t>& result)
{
    if (upcasedCommand == "RANKLIST") {
        /* @q RANKLIST [SORT f:HostRankField] [REVERSE] [FIELDS fs:HostRankField ...] (Host Command)
           Get list of users.

           Returns a list of alternating user Ids (UID) and list-of-associated-fields,
           for example, ["uid1",["field1a","field1b"],"uid2",["field2a","field2b"]].
           This allows the consumer to treat the result as an associative array, but also preserves sort order.

           If a field is given with SORT, the result is sorted (optionally, reverse sort).

           If the FIELDS clause is given, it must be last.
           All following parameters are fields to return in the list-of-associated-fields.
           The list elements will have the same order as the fields in the FIELDS clause.

           @since PCC2 2.40.6
           @rettype UID */

        /* @type HostRankField
           Field in a {RANKLIST} query.

           Possible values:
           - name
           - screenname
           - rank
           - rankpoints
           - turnreliability
           - turnsplayed
           - turnsmissed

           See also: {RANKLIST}, {SET (User Command)} */

        // Parse
        HostRanking::ListRequest req;
        while (args.getNumArgs() > 0) {
            String_t keyword = afl::string::strUCase(toString(args.getNext()));
            if (keyword == "SORT") {
                args.checkArgumentCountAtLeast(1);
                req.sortField = toString(args.getNext());
            } else if (keyword == "REVERSE") {
                req.sortReverse = true;
            } else if (keyword == "FIELDS") {
                while (args.getNumArgs() > 0) {
                    req.fieldsToGet.push_back(toString(args.getNext()));
                }
            } else {
                throw std::runtime_error(SYNTAX_ERROR);
            }
        }

        // Produce result
        result.reset(m_implementation.getUserList(req));
        return true;
    } else {
        return false;
    }
}
