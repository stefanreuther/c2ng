/**
  *  \file game/browser/usercallback.hpp
  */
#ifndef C2NG_GAME_BROWSER_USERCALLBACK_HPP
#define C2NG_GAME_BROWSER_USERCALLBACK_HPP

#include "afl/base/deletable.hpp"
#include "afl/string/string.hpp"
#include "afl/data/segment.hpp"

namespace game { namespace browser {

    // FIXME: this class is not yet final
    class UserCallback : public afl::base::Deletable {
     public:
        enum Type {
            AskString,            // String input field; associated with a single String value
            AskPassword,          // Password input field; associated with a single String value
            ShowInfo              // Information; no associated value
        };
        struct Element {
            Type type;
            String_t prompt;
        };

        // Data entry dialog
        virtual bool askInput(String_t title,
                              const std::vector<Element>& question,
                              afl::data::Segment& values) = 0;
    };

} }

#endif
