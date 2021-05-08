/**
  *  \file game/spec/info/nullpicturenamer.hpp
  *  \brief Class game::spec::info::NullPictureNamer
  */
#ifndef C2NG_GAME_SPEC_INFO_NULLPICTURENAMER_HPP
#define C2NG_GAME_SPEC_INFO_NULLPICTURENAMER_HPP

#include "game/spec/info/picturenamer.hpp"

namespace game { namespace spec { namespace info {

    /** Null picture name generator.
        Implements PictureNamer by returning an empty string for everything. */
    class NullPictureNamer : public PictureNamer {
     public:
        virtual String_t getHullPicture(const Hull& h) const;
        virtual String_t getEnginePicture(const Engine& e) const;
        virtual String_t getBeamPicture(const Beam& b) const;
        virtual String_t getLauncherPicture(const TorpedoLauncher& tl) const;
        virtual String_t getAbilityPicture(const String_t& abilityName) const;
        virtual String_t getPlayerPicture(const Player& pl) const;
        virtual String_t getFighterPicture(int raceNr, int playerNr) const;
        virtual String_t getVcrObjectPicture(bool isPlanet, int pictureNumber) const;
    };

} } }

#endif
