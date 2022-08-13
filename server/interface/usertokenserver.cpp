/**
  *  \file server/interface/usertokenserver.cpp
  *  \brief Class server::interface::UserTokenServer
  */

#include <stdexcept>
#include "server/interface/usertokenserver.hpp"
#include "afl/data/hash.hpp"
#include "afl/data/hashvalue.hpp"
#include "afl/data/stringlist.hpp"
#include "server/errors.hpp"
#include "server/interface/usertoken.hpp"
#include "server/types.hpp"

server::interface::UserTokenServer::UserTokenServer(UserToken& impl)
    : m_implementation(impl)
{ }

server::interface::UserTokenServer::~UserTokenServer()
{ }

bool
server::interface::UserTokenServer::handleCommand(const String_t& upcasedCommand, interpreter::Arguments& args, std::auto_ptr<Value_t>& result)
{
    if (upcasedCommand == "MAKETOKEN") {
        /* @q MAKETOKEN uid:UID type:Str (User Command)
           Get a valid token of the given type for the given user.
           If a token with sufficient remaining lifetime exists, return that.
           Otherwise, creates a new one.

           @retval Str token
           @uses user:$UID:tokens:$TOKENTYPE
           @err 400 Bad request (bad type)
           @err 404 Not found (user does not exist)
           @since PCC2 2.40.6 */
        args.checkArgumentCount(2);
        String_t userId = toString(args.getNext());
        String_t tokenType = toString(args.getNext());
        result.reset(makeStringValue(m_implementation.getToken(userId, tokenType)));
        return true;
    } else if (upcasedCommand == "CHECKTOKEN") {
        /* @q CHECKTOKEN token:Str [TYPE type:Str] [RENEW] (User Command)
           Validate a token.
           If the token is valid,

           @retkey user:UID User
           @retkey type:Str Type
           @retkey new:Str New token (optional)

           @err 410 Expired (token does not exist or is expired)
           @since PCC2 2.40.6 */
        // Parse args
        args.checkArgumentCountAtLeast(1);
        String_t token = toString(args.getNext());
        afl::base::Optional<String_t> requiredType;
        bool autoRenew = false;
        while (args.getNumArgs() > 0) {
            String_t keyword = afl::string::strUCase(toString(args.getNext()));
            if (keyword == "RENEW") {
                autoRenew = true;
            } else if (keyword == "TYPE") {
                args.checkArgumentCountAtLeast(1);
                requiredType = toString(args.getNext());
            } else {
                throw std::runtime_error(INVALID_OPTION);
            }
        }

        // Do it
        UserToken::Info i = m_implementation.checkToken(token, requiredType, autoRenew);

        // Produce result
        afl::data::Hash::Ref_t h(afl::data::Hash::create());
        h->setNew("user", makeStringValue(i.userId));
        h->setNew("type", makeStringValue(i.tokenType));
        addOptionalStringKey(*h, "new", i.newToken);
        result.reset(new afl::data::HashValue(h));
        return true;
    } else if (upcasedCommand == "RESETTOKEN") {
        /* @q RESETTOKEN uid:UID type:Str... (User Command)
           Delete/invalidate tokens.
           @since PCC2 2.40.6 */
        args.checkArgumentCountAtLeast(1);
        String_t userId = toString(args.getNext());

        afl::data::StringList_t types;
        while (args.getNumArgs() > 0) {
            types.push_back(toString(args.getNext()));
        }

        m_implementation.clearToken(userId, types);

        result.reset(makeStringValue("OK"));
        return true;
    } else {
        return false;
    }
}
