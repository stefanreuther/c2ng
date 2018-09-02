/**
  *  \file game/v3/fizzfile.hpp
  *  \brief Class game::v3::FizzFile
  */
#ifndef C2NG_GAME_V3_FIZZFILE_HPP
#define C2NG_GAME_V3_FIZZFILE_HPP

#include "afl/base/types.hpp"
#include "afl/io/directory.hpp"
#include "game/v3/structures.hpp"

namespace game { namespace v3 {

    class FizzFile {
     public:
        typedef game::v3::structures::Section Section;
        
        FizzFile();

        /** Reset.
            Resets this object to empty, unconfigured. */
        void clear();

        /** Load data from directory.
            Checks for presence of a fizz.bin file and loads that.
            \param dir Directory */
        void load(afl::io::Directory& dir);

        /** Save data to directory.
            If the file content is valid, saves the file.
            (Never creates a file that did not exist before.)
            \param dir Directory */
        void save(afl::io::Directory& dir);

        /** Set checksum.
            \param section Section in file (object type)
            \param player Player number. Out-of-range values are ignored.
            \param checksum Checksum */
        void set(Section section, int player, uint32_t checksum);

        /** Check validity.
            \return true if file was loaded successfully */
        bool isValid() const;

     private:
        static const int NUM_PLAYERS = game::v3::structures::NUM_PLAYERS;

        uint32_t m_data[NUM_PLAYERS * 3];

        bool m_valid;
    };

} }

#endif
