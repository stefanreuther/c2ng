/**
  *  \file server/interface/talkrender.hpp
  */
#ifndef C2NG_SERVER_INTERFACE_TALKRENDER_HPP
#define C2NG_SERVER_INTERFACE_TALKRENDER_HPP

#include "afl/base/deletable.hpp"
#include "afl/base/optional.hpp"
#include "afl/string/string.hpp"

namespace server { namespace interface {

    class TalkRender : public afl::base::Deletable {
     public:
        struct Options {
            afl::base::Optional<String_t> baseUrl;
            afl::base::Optional<String_t> format;
        };

        virtual void setOptions(const Options& opts) = 0;
        virtual String_t render(const String_t& text, const Options& opts) = 0;
    };

} }

#endif
