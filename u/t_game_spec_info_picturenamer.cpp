/**
  *  \file u/t_game_spec_info_picturenamer.cpp
  *  \brief Test for game::spec::info::PictureNamer
  */

#include "game/spec/info/picturenamer.hpp"

#include "t_game_spec_info.hpp"

/** Interface test. */
void
TestGameSpecInfoPictureNamer::testInterface()
{
    class Tester : public game::spec::info::PictureNamer {
     public:
        virtual String_t getHullPicture(const game::spec::Hull& /*h*/) const
            { return String_t(); }
        virtual String_t getEnginePicture(const game::spec::Engine& /*e*/) const
            { return String_t(); }
        virtual String_t getBeamPicture(const game::spec::Beam& /*b*/) const
            { return String_t(); }
        virtual String_t getLauncherPicture(const game::spec::TorpedoLauncher& /*tl*/) const
            { return String_t(); }
        virtual String_t getAbilityPicture(const String_t& /*abilityName*/, game::spec::info::AbilityFlags_t /*flags*/) const
            { return String_t(); }
        virtual String_t getPlayerPicture(const game::Player& /*pl*/) const
            { return String_t(); }
        virtual String_t getFighterPicture(int /*raceNr*/, int /*playerNr*/) const
            { return String_t(); }
        virtual String_t getVcrObjectPicture(bool /*isPlanet*/, int /*pictureNumber*/) const
            { return String_t(); }
    };
    Tester t;
}

