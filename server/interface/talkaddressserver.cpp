/**
  *  \file server/interface/talkaddressserver.cpp
  */

#include "server/interface/talkaddressserver.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/vectorvalue.hpp"
#include "server/interface/talkaddress.hpp"
#include "interpreter/values.hpp"
#include "server/types.hpp"

namespace {
    void toStringList(afl::data::StringList_t& list, interpreter::Arguments& args)
    {
        while (args.getNumArgs() > 0) {
            list.push_back(server::toString(args.getNext()));
        }
    }

    afl::data::Value* toVector(afl::base::Memory<const String_t> list)
    {
        afl::data::Vector::Ref_t vec = afl::data::Vector::create();
        while (const String_t* p = list.eat()) {
            vec->pushBackString(*p);
        }
        return new afl::data::VectorValue(vec);
    }
}


server::interface::TalkAddressServer::TalkAddressServer(TalkAddress& impl)
    : m_implementation(impl)
{ }

server::interface::TalkAddressServer::~TalkAddressServer()
{ }

bool
server::interface::TalkAddressServer::handleCommand(const String_t& upcasedCommand, interpreter::Arguments& args, std::auto_ptr<Value_t>& result)
{
    /*
     *  These commands are called ADDRMPARSE, ADDRMRENDER because they operate on multiple items at once.
     *  This allows adding future ADDRPARSE, ADDRRENDER commands although as of 20190330 none such are planned.
     */
    if (upcasedCommand == "ADDRMPARSE") {
        /* @q ADDRMPARSE receiver:Str... (Talk Command)
           Parse receivers.
           Given a list of human-readable receivers (i.e. user names), produces a list of machine-readable receivers.
           Each parameter corresponds to an element in the result list.
           If a receiver cannot be parsed, its element will be empty.
           To obtain a TalkAddr for {PMNEW}, join the non-empty result elements using ','.

           Permissions: none

           @retval TalkAddr[] Machine-readable receivers
           @uses uid:$USERNAME */
        afl::data::StringList_t in;
        toStringList(in, args);

        afl::data::StringList_t out;
        m_implementation.parse(in, out);

        result.reset(toVector(out));
        return true;
    } else if (upcasedCommand == "ADDRMRENDER") {
        /* @q ADDRMRENDER receiver:TalkAddr... (Talk Command)
           Render receivers.
           Given a list of machine-readable receivers, renders these according to RENDEROPTION.
           Each parameter corresponds to an element in the result list.
           If a receiver cannot be parsed, its element will be empty.

           Formats are configured with RENDEROPTION and have mostly the same meaning:
           - html, mail, news, text, forum<em>LS</em>: as given for display
           - raw: produce suitable input to ADDRMPARSE
           - modifiers (quote:, noquote:, break:, abstract:, force:) are accepted but mostly pointless

           Permissions: none

           @retval Str[] Rendered receivers
           @uses uid:$USERNAME */
        afl::data::StringList_t in;
        toStringList(in, args);

        afl::data::StringList_t out;
        m_implementation.render(in, out);

        result.reset(toVector(out));
        return true;
    } else {
        return false;
    }
}
