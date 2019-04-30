/**
  *  \file server/interface/userdataserver.cpp
  *  \brief Class server::interface::UserDataServer
  */

#include "server/interface/userdataserver.hpp"
#include "server/types.hpp"
#include "server/interface/userdata.hpp"

server::interface::UserDataServer::UserDataServer(UserData& impl)
    : m_implementation(impl)
{ }

bool
server::interface::UserDataServer::handleCommand(const String_t& upcasedCommand, interpreter::Arguments& args, std::auto_ptr<Value_t>& result)
{
    if (upcasedCommand == "UGET") {
        /* @q UGET uid:UID key:Str (User Command)
           Set application-data value.
           @retval Str value
           @err 400 Invalid key
           @since PCC2 2.40.6 */
        args.checkArgumentCount(2);
        String_t uid = toString(args.getNext());
        String_t key = toString(args.getNext());
        result.reset(makeStringValue(m_implementation.get(uid, key)));
        return true;
    } else if (upcasedCommand == "USET") {
        /* @q USET uid:UID key:Str value:Str (User Command)
           Set application-data value.
           @err 400 Invalid key or value
           @since PCC2 2.40.6 */
        args.checkArgumentCount(3);
        String_t uid = toString(args.getNext());
        String_t key = toString(args.getNext());
        String_t value = toString(args.getNext());
        m_implementation.set(uid, key, value);
        result.reset(makeStringValue("OK"));
        return true;
    } else {
        return false;
    }
}
