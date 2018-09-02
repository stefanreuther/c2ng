/**
  *  \file game/v3/trn/turnprocessor.hpp
  *  \brief Interface game::v3::trn::TurnProcessor
  */
#ifndef C2NG_GAME_V3_TRN_TURNPROCESSOR_HPP
#define C2NG_GAME_V3_TRN_TURNPROCESSOR_HPP

#include "game/v3/structures.hpp"
#include "game/v3/turnfile.hpp"
#include "afl/charset/charset.hpp"

namespace game { namespace v3 { namespace trn {

    class TurnProcessor {
     public:
        typedef game::v3::structures::Ship   Ship_t;
        typedef game::v3::structures::Planet Planet_t;
        typedef game::v3::structures::Base   Base_t;
        typedef uint8_t NewPassword_t[10];

        TurnProcessor();
        virtual ~TurnProcessor();

        void handleTurnFile(TurnFile& f, afl::charset::Charset& charset);

        virtual void handleInvalidCommand(int code) = 0;
        virtual void validateShip(int id) = 0;
        virtual void validatePlanet(int id) = 0;
        virtual void validateBase(int id) = 0;

        virtual void getShipData(int id, Ship_t& out, afl::charset::Charset& charset) = 0;
        virtual void getPlanetData(int id, Planet_t& out, afl::charset::Charset& charset) = 0;
        virtual void getBaseData(int id, Base_t& out, afl::charset::Charset& charset) = 0;

        virtual void storeShipData(int id, const Ship_t& in, afl::charset::Charset& charset) = 0;
        virtual void storePlanetData(int id, const Planet_t& in, afl::charset::Charset& charset) = 0;
        virtual void storeBaseData(int id, const Base_t& in, afl::charset::Charset& charset) = 0;

        virtual void addMessage(int to, String_t text) = 0;
        virtual void addNewPassword(const NewPassword_t& pass) = 0;
        virtual void addAllianceCommand(String_t text) = 0;
    };

} } }

#endif
