/**
  *  \file server/play/packer.hpp
  */
#ifndef C2NG_SERVER_PLAY_PACKER_HPP
#define C2NG_SERVER_PLAY_PACKER_HPP

#include "afl/base/deletable.hpp"
#include "afl/string/string.hpp"
#include "server/types.hpp"
#include "afl/data/hash.hpp"
#include "interpreter/context.hpp"

namespace server { namespace play {

    class Packer : public afl::base::Deletable {
        // ex ServerObjectWriter, ServerQueryWriter
     public:
        virtual Value_t* buildValue() const = 0;
        virtual String_t getName() const = 0;

        static void addValue(afl::data::Hash& hv, interpreter::Context& ctx, const char* scriptName, const char* jsonName);
        static void addValueNew(afl::data::Hash& hv, Value_t* value, const char* jsonName);
    };

} }

#endif
