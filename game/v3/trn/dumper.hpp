/**
  *  \file game/v3/trn/dumper.hpp
  *  \class game::v3::trn::Dumper
  */
#ifndef C2NG_GAME_V3_TRN_DUMPER_HPP
#define C2NG_GAME_V3_TRN_DUMPER_HPP

#include "afl/base/types.hpp"
#include "afl/base/uncopyable.hpp"
#include "afl/io/textwriter.hpp"
#include "afl/string/string.hpp"
#include "game/v3/turnfile.hpp"

namespace game { namespace v3 { namespace trn {

    class Filter;

    /** Turn file dumper.
        This class converts a turn file into human-readable text format for debugging.
        This is the core of the c2untrn ("un-trn") utility. */
    class Dumper : afl::base::Uncopyable {
     public:
        /** Constructor.
            \param out Output file */
        explicit Dumper(afl::io::TextWriter& out);

        /** Set "show comments" option.
            When enabled (default), the report will contain comments.
            When disabled, the commands will not contain comments, allowing easier comparison of reports from different files.
            \param flag Flag */
        void setShowComments(bool flag);

        /** Set "show header" option.
            When enabled (default), the report will contain the turn file header.
            \param flag Flag */
        void setShowHeader(bool flag);

        /** Set "show trailer" option.
            When enabled (default), the report will contain the turn file trailer.
            \param flag Flag */
        void setShowTrailer(bool flag);

        /** Set "verify trailer checksum" option.
            When enabled (default), the trailer checksum will be verified against the computed value.
            When disabled, the checksum will not be verified. Use this if the turn was modified before dumping.
            \param flag Flag */
        void setVerifyTrailerChecksum(bool flag);

        /** Set command filter.
            If a non-null filter is set, only matching commands will be shown.
            \param f Filter. Can be null. If non-null, must have a lifetime greater than that of the Dumper. */
        void setFilter(const Filter* f);

        /** Main entry point: dump a turn file.
            \param trn Turn file
            \retval true Some commands were output
            \retval false No commands were output, either because the turn was empty or because the filter matched none */
        bool dump(const TurnFile& trn) const;

     private:
        class CommandReader;

        void showLine(const String_t& name, const String_t& value, const String_t& comment) const;
        void showValue(const char* name, int32_t value) const;
        void showValue(const String_t& name, int32_t value) const;
        void showValue(const String_t& name, int32_t value, const String_t& comment) const;
        void showValue(const char* name, String_t value) const;
        void showValue(const String_t& name, String_t value) const;
        void showValue(const String_t& name, String_t value, const String_t& comment) const;
        void showValueArray(const String_t& name, CommandReader& rdr, size_t n) const;
        void showMessage(const TurnFile& trn, CommandReader& rdr, size_t size) const;
        void showUtilData(const TurnFile& trn, CommandReader& rdr, uint16_t type, uint16_t size) const;
        void showHex(CommandReader& rdr, size_t size) const;
        void showTaccom(const TurnFile& trn) const;

        void showHeader(const TurnFile& trn) const;
        void showTrailer(const TurnFile& trn) const;
        void showCommand(const TurnFile& trn, size_t index) const;

        afl::io::TextWriter& m_output;

        bool m_showComments;
        bool m_showHeader;
        bool m_showTrailer;
        bool m_verifyTrailerChecksum;
        const Filter* m_filter;
    };

} } }

#endif
