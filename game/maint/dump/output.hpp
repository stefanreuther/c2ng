/**
  *  \file game/maint/dump/output.hpp
  *  \brief Class game::maint::dump::Output
  */
#ifndef C2NG_GAME_MAINT_DUMP_OUTPUT_HPP
#define C2NG_GAME_MAINT_DUMP_OUTPUT_HPP

#include "afl/base/deletable.hpp"
#include "afl/string/string.hpp"

namespace game { namespace maint { namespace dump {

    /** Interface for dump output. */
    class Output : public afl::base::Deletable {
     public:
        /** Start a new section (object).
            Each call to startRecord() shall eventually be followed by endRecord().
            @param header Name of object */
        virtual void startRecord(String_t header) = 0;

        /** Add a field.
            @param name Field name
            @param formattedValue Formatted value (e.g. "9" or "'Name'") */
        virtual void addField(String_t name, String_t formattedValue) = 0;

        /** Add unparsed data.
            @param formattedValue Formatted value (e.g. "14 25 57") */
        virtual void addUnparsedData(String_t formattedValue) = 0;

        /** End a section. */
        virtual void endRecord() = 0;

        // Additional signature for efficiency
        void addField(const char* name, String_t formattedValue);
    };

} } }

#endif
