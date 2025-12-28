/**
  *  \file game/maint/dump/textoutput.hpp
  *  \brief Class game::maint::dump::TextOutput
  */
#ifndef C2NG_GAME_MAINT_DUMP_TEXTOUTPUT_HPP
#define C2NG_GAME_MAINT_DUMP_TEXTOUTPUT_HPP

#include "afl/io/textwriter.hpp"
#include "game/maint/dump/output.hpp"

namespace game { namespace maint { namespace dump {

    /** Output implementation for human-readable text. */
    class TextOutput : public Output {
     public:
        /** Constructor.
            @param out TextWriter instance */
        explicit TextOutput(afl::io::TextWriter& out);

        // Output:
        virtual void startRecord(String_t header);
        virtual void addField(String_t name, String_t formattedValue);
        virtual void addUnparsedData(String_t formattedValue);
        virtual void endRecord();

        using Output::addField;

     private:
        afl::io::TextWriter& m_output;
        int m_level;
    };

} } }

#endif
