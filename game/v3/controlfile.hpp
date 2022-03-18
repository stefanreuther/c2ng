/**
  *  \file game/v3/controlfile.hpp
  *  \brief Class game::v3::ControlFile
  */
#ifndef C2NG_GAME_V3_CONTROLFILE_HPP
#define C2NG_GAME_V3_CONTROLFILE_HPP

#include "afl/io/directory.hpp"
#include "game/types.hpp"
#include "game/v3/structures.hpp"
#include "afl/string/translator.hpp"
#include "afl/sys/loglistener.hpp"

namespace game { namespace v3 {

    /** Control (checksum) file.

        In Dosplan, a file "control.dat" stores checksums over ship, pdata and bdata records for each directory.
        In Winplan, such a file is created for each player.
        This module manages these checksums.
        We maintain the checksums only to please Tim's maketurns, we don't check them ourselves.

        This class stores an in-memory copy of such a file, as well as a "file owner" attribute:
        - 0 = Dosplan (file valid for all players in this directory)
        - >0 = Winplan (file owned by this player)
        - <0 = no file exists */
    class ControlFile {
     public:
        typedef game::v3::structures::Section Section;

        /** Default constructor.
            Makes an empty, unconfigured file. */
        ControlFile();

        /** Reset.
            Resets this object to empty, unconfigured. */
        void clear();

        /** Load data from directory.
            Checks for presence of a checksum file, loads that, and sets the owner accordingly.
            \param dir Directory
            \param player Player number to look for
            \param tx Translator
            \param log Log listener */
        void load(afl::io::Directory& dir, int player, afl::string::Translator& tx, afl::sys::LogListener& log);

        /** Save data to directory.
            If the file owner is set to a valid value (>= 0), creates the file.
            \param dir Directory
            \param tx  Translator
            \param log Logger */
        void save(afl::io::Directory& dir, afl::string::Translator& tx, afl::sys::LogListener& log);

        /** Set checksum.
            \param section Section in file (object type)
            \param id Object Id. Out-of-range values are ignored.
            \param checksum Checksum */
        void set(Section section, Id_t id, uint32_t checksum);

        /** Set file owner.
            Defines what file will be written by save().
            Also see class description.
            \param owner New owner [-1,NUM_PLAYERS] */
        void setFileOwner(int owner);

     private:
        static const size_t CONTROL_MIN = 1501;
        static const size_t CONTROL_MAX = 2499;

        typedef uint32_t Value_t;
        Value_t m_data[CONTROL_MAX];

        int m_fileOwner;

        Value_t* getSlot(Section section, Id_t id);
    };

} }

#endif
