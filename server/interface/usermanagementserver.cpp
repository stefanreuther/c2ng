/**
  *  \file server/interface/usermanagementserver.cpp
  *  \brief Class server::interface::UserManagementServer
  */

#include "server/interface/usermanagementserver.hpp"
#include "server/types.hpp"
#include "server/interface/usermanagement.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/vectorvalue.hpp"

server::interface::UserManagementServer::UserManagementServer(UserManagement& impl)
    : m_implementation(impl)
{ }

server::interface::UserManagementServer::~UserManagementServer()
{ }

bool
server::interface::UserManagementServer::handleCommand(const String_t& upcasedCommand, interpreter::Arguments& args, std::auto_ptr<Value_t>& result)
{
    if (upcasedCommand == "ADDUSER") {
        /* @q ADDUSER name:Str pass:Str [key:Str value:Str, ...] (User Command)
           Create a new user.
           The given key/value pairs are placed in the user's profile, as if by the SET command.
           @retval UID User Id
           @err 401 Invalid user name
           @err 409 User already exists
           @since PCC2 2.40.6 */
        args.checkArgumentCountAtLeast(2);
        String_t userName = toString(args.getNext());
        String_t password = toString(args.getNext());

        afl::data::StringList_t config;
        while (args.getNumArgs() > 0) {
            config.push_back(toString(args.getNext()));
        }

        result.reset(makeStringValue(m_implementation.add(userName, password, config)));
        return true;
    } else if (upcasedCommand == "DELUSER") {
        /* @q DELUSER uid:UID (User Command)
           Delete a user.
           Postcondition is that the user does not exist, so this will not fail if the user Id does not exist.
           @since PCC2 2.40.7 */
        args.checkArgumentCount(1);
        m_implementation.remove(toString(args.getNext()));
        result.reset(makeStringValue("OK"));
        return true;
    } else if (upcasedCommand == "LOGIN") {
        /* @q LOGIN name:Str pass:Str (User Command)
           Check user password (log in).
           On success, returns the user id.
           @retval UID User Id
           @err 401 Username/password do not match
           @since PCC2 2.40.6 */
        args.checkArgumentCount(2);
        String_t userName = toString(args.getNext());
        String_t password = toString(args.getNext());
        result.reset(makeStringValue(m_implementation.login(userName, password)));
        return true;
    } else if (upcasedCommand == "LOOKUP") {
        /* @q LOOKUP name:Str (User Command)
           Given a user name, return the user Id.
           @retval UID User Id
           @err 404 User does not exist
           @since PCC2 2.40.6 */
        args.checkArgumentCount(1);
        result.reset(makeStringValue(m_implementation.getUserIdByName(toString(args.getNext()))));
        return true;
    } else if (upcasedCommand == "NAME") {
        /* @q NAME uid:UID (User Command)
           Given a user Id, return the user name.
           @retval Str User name
           @since PCC2 2.40.6 */
        args.checkArgumentCount(1);
        result.reset(makeStringValue(m_implementation.getNameByUserId(toString(args.getNext()))));
        return true;
    } else if (upcasedCommand == "MNAME") {
        /* @q MNAME uid:UID... (User Command)
           Given a multiple user Ids, return the user names.
           @retval Str[] User names
           @since PCC2 2.40.6 */
        afl::data::StringList_t userIds;
        while (args.getNumArgs() > 0) {
            userIds.push_back(toString(args.getNext()));
        }

        afl::data::StringList_t userNames;
        m_implementation.getNamesByUserId(userIds, userNames);

        afl::data::Vector::Ref_t vec = afl::data::Vector::create();
        vec->pushBackElements(userNames);

        result.reset(new afl::data::VectorValue(vec));
        return true;
    } else if (upcasedCommand == "GET") {
        /* @q GET uid:UID key:Str (User Command)
           Return value from the user's profile.
           @retval Any Value
           @since PCC2 2.40.6 */
        args.checkArgumentCount(2);
        String_t userId = toString(args.getNext());
        String_t key = toString(args.getNext());
        result.reset(m_implementation.getProfileRaw(userId, key));
        return true;
    } else if (upcasedCommand == "MGET") {
        /* @q MGET uid:UID [key:Str, ...] (User Command)
           Return values from the user's profile.
           @retval Any[] Values
           @since PCC2 2.40.6 */
        args.checkArgumentCountAtLeast(1);
        String_t userId = toString(args.getNext());

        afl::data::StringList_t keys;
        while (args.getNumArgs() > 0) {
            keys.push_back(toString(args.getNext()));
        }

        result.reset(m_implementation.getProfileRaw(userId, keys));
        return true;
    } else if (upcasedCommand == "SET") {
        /* @q SET uid:UID [key:Str value:Str, ...] (User Command)
           Set values in user's profile.
           @since PCC2 2.40.6 */
        args.checkArgumentCountAtLeast(1);
        String_t userId = toString(args.getNext());

        afl::data::StringList_t config;
        while (args.getNumArgs() > 0) {
            config.push_back(toString(args.getNext()));
        }

        m_implementation.setProfile(userId, config);
        result.reset(makeStringValue("OK"));
        return true;
    } else if (upcasedCommand == "PASSWD") {
        /* @q PASSWD uid:UID pass:Str (User Command)
           Change user's password.
           @since PCC2 2.40.6 */
        args.checkArgumentCount(2);
        String_t userId = toString(args.getNext());
        String_t password = toString(args.getNext());
        m_implementation.setPassword(userId, password);
        result.reset(makeStringValue("OK"));
        return true;
    } else {
        return false;
    }
}

