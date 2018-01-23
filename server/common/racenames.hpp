/**
  *  \file server/common/racenames.hpp
  *  \brief Class server::common::RaceNames
  */
#ifndef C2NG_SERVER_COMMON_RACENAMES_HPP
#define C2NG_SERVER_COMMON_RACENAMES_HPP

#include "game/playerarray.hpp"
#include "afl/charset/charset.hpp"

namespace server { namespace common {

    /** Race name storage.
        This is a reduced version of the race.nm parser for code that needs just that single file.
        It does not handle extra race slots (unowned/aliens) nor extra attributes (user names, etc.). */
    class RaceNames {
     public:
        /** Default constructor.
            Initializes the object to all-blank. */
        RaceNames();

        /** Destructor. */
        ~RaceNames();

        /** Load from array-of-bytes.
            \param data Data loaded from file system
            \param cs   Character set
            \throw afl::except::FileProblemException if the data cannot be interpreted as race name file */
        void load(afl::base::ConstBytes_t data, afl::charset::Charset& cs);

        /** Access short names.
            \return Short names */
        const game::PlayerArray<String_t>& shortNames() const;

        /** Access long names.
            \return Long names */
        const game::PlayerArray<String_t>& longNames() const;

        /** Access adjectives.
            \return Adjectives */
        const game::PlayerArray<String_t>& adjectiveNames() const;

     private:
        game::PlayerArray<String_t> m_shortNames;
        game::PlayerArray<String_t> m_longNames;
        game::PlayerArray<String_t> m_adjectiveNames;
    };

} }

#endif
