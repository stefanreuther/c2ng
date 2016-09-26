/**
  *  \file game/v3/resultfile.hpp
  *  \brief Class game::v3::ResultFile
  */
#ifndef C2NG_GAME_V3_RESULTFILE_HPP
#define C2NG_GAME_V3_RESULTFILE_HPP

#include "afl/base/types.hpp"
#include "afl/io/stream.hpp"
#include "afl/string/translator.hpp"

namespace game { namespace v3 {

    /** This class parses result files.
        It provides the user with a means of querying header information, such as section positions. */
    class ResultFile {
     public:
        /** RST Section numbers.
            The first 8 correspond to the pointers in the header. */
        enum Section {
            ShipSection,               ///< Player's ships.
            TargetSection,             ///< Visual contacts.
            PlanetSection,             ///< Player's planets.
            BaseSection,               ///< Player's bases.
            MessageSection,            ///< Incoming messages.
            ShipXYSection,             ///< Ship X/Y, nonvisual contacts.
            GenSection,                ///< Red tape.
            VcrSection,                ///< Combat recordings.
            KoreSection,               ///< "KORE" (minefields, race names, storms, contacts, UFOs).
            LeechSection,              ///< "LEECH.DAT".
            SkoreSection               ///< "SKORE" (more UFOs).
        };
        static const size_t NUM_SECTIONS = SkoreSection+1;

        /** Constructor.
            \param file result file. Must be seekable.
            \param tx Translator for error messages.

            \throws FileFormatException if file is too short or contains an invalid pointer.
            Note that the file is not completely validated, just the pointers are checked to be not too badly out of range.
            User code must handle the case that the file was truncated. */
        ResultFile(afl::io::Stream& file, afl::string::Translator& tx);

        /** Destructor. */
        ~ResultFile();

        /** Get result file version.
            This can be -1 (old-style turn, 3.0), 0 (3.500) or 1 (3.501).
            No known host generates higher versions, but we would support them in theory.
            This value is only for information purposes.
            Use getSectionOffset()/hasSection() to check for section presence. */
        int getVersion() const;

        
        /** Get offset of a RST section.
            \param section [in] Section number to query
            \param offset [out] Section offset, suitable for Stream::setPos().
            \retval true section is present, \c offset has been set
            \retval false section is not present */
        bool getSectionOffset(Section section, afl::io::Stream::FileSize_t& offset) const;

        /** Check whether section is present.
            \returns true iff given section is present */
        bool hasSection(Section section) const;

        /** Get underlying file.
            \return underlying file */
        afl::io::Stream& getFile() const;

     private:
        /** Underlying file. */
        afl::io::Stream& m_file;

        /** Result file version. */
        int m_version;

        /** File offsets.
            Each of these is nonzero if the section is present, zero if it is not.
            These are real file offsets (0-based), but a result file cannot have a section at offset 0. */
        uint32_t m_offset[NUM_SECTIONS];

        void loadHeader(afl::string::Translator& tx);
        void setSectionAddress(Section section, int32_t addressFromFile, afl::io::Stream::FileSize_t fileSize, afl::string::Translator& tx);
    };
} }

#endif
