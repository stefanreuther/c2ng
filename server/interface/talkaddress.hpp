/**
  *  \file server/interface/talkaddress.hpp
  */
#ifndef C2NG_SERVER_INTERFACE_TALKADDRESS_HPP
#define C2NG_SERVER_INTERFACE_TALKADDRESS_HPP

#include "afl/base/deleter.hpp"
#include "afl/data/stringlist.hpp"
#include "afl/base/memory.hpp"
#include "afl/string/string.hpp"

namespace server { namespace interface {

    class TalkAddress : public afl::base::Deleter {
     public:
        virtual void parse(afl::base::Memory<const String_t> in, afl::data::StringList_t& out) = 0;

        virtual void render(afl::base::Memory<const String_t> in, afl::data::StringList_t& out) = 0;
    };

} }

#endif
