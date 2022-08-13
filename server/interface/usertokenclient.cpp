/**
  *  \file server/interface/usertokenclient.cpp
  *  \brief Class server::interface::UserTokenClient
  */

#include "server/interface/usertokenclient.hpp"
#include "afl/data/access.hpp"
#include "afl/data/segment.hpp"
#include "server/types.hpp"

using afl::data::Segment;

server::interface::UserTokenClient::UserTokenClient(afl::net::CommandHandler& commandHandler)
    : m_commandHandler(commandHandler)
{ }

String_t
server::interface::UserTokenClient::getToken(String_t userId, String_t tokenType)
{
    return m_commandHandler.callString(Segment()
                                       .pushBackString("MAKETOKEN")
                                       .pushBackString(userId)
                                       .pushBackString(tokenType));
}

server::interface::UserToken::Info
server::interface::UserTokenClient::checkToken(String_t token, afl::base::Optional<String_t> requiredType, bool autoRenew)
{
    Segment seg;
    seg.pushBackString("CHECKTOKEN");
    seg.pushBackString(token);
    if (const String_t* p = requiredType.get()) {
        seg.pushBackString("TYPE");
        seg.pushBackString(*p);
    }
    if (autoRenew) {
        seg.pushBackString("RENEW");
    }

    std::auto_ptr<Value_t> result(m_commandHandler.call(seg));
    afl::data::Access a(result);

    Info i;
    i.userId = a("user").toString();
    i.tokenType = a("type").toString();
    i.newToken = toOptionalString(a("new").getValue());
    return i;
}

void
server::interface::UserTokenClient::clearToken(String_t userId, afl::base::Memory<const String_t> tokenTypes)
{
    Segment seg;
    seg.pushBackString("RESETTOKEN");
    seg.pushBackString(userId);
    while (const String_t* p = tokenTypes.eat()) {
        seg.pushBackString(*p);
    }

    m_commandHandler.callVoid(seg);
}
