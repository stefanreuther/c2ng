/**
  *  \file server/interface/talkrender.hpp
  *  \brief Interface server::interface::TalkRender
  */
#ifndef C2NG_SERVER_INTERFACE_TALKRENDER_HPP
#define C2NG_SERVER_INTERFACE_TALKRENDER_HPP

#include "afl/base/deletable.hpp"
#include "afl/base/optional.hpp"
#include "afl/string/string.hpp"

namespace server { namespace interface {

    /** Interface to message rendering. */
    class TalkRender : public afl::base::Deletable {
     public:
        /** Render options. */
        struct Options {
            afl::base::Optional<String_t> baseUrl;  ///< Base URL (prefix) for links, e.g.\ in HTML.
            afl::base::Optional<String_t> format;   ///< Format.
        };

        /** Warning message. */
        struct Warning {
            String_t type;                          ///< Type of warning.
            String_t token;                         ///< Token at which the warning was detected.
            String_t extra;                         ///< Extra information.
            int pos;                                ///< Position of token in text.
        };

        /** Set options for future renderings in other commands (RENDEROPTION).
            @param opts Options */
        virtual void setOptions(const Options& opts) = 0;

        /** Render text (RENDER).
            @param text Text
            @param opts Options
            @return rendered text */
        virtual String_t render(const String_t& text, const Options& opts) = 0;

        /** Check text for possible syntax problems (RENDERCHECK).
            @param [in]  text Text
            @param [out] out  Warnings */
        virtual void check(const String_t& text, std::vector<Warning>& out) = 0;
    };

} }

#endif
