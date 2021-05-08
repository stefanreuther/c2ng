/**
  *  \file client/picturenamer.hpp
  *  \brief Class client::PictureNamer
  */
#ifndef C2NG_CLIENT_PICTURENAMER_HPP
#define C2NG_CLIENT_PICTURENAMER_HPP

#include "game/spec/info/picturenamer.hpp"

namespace client {

    /** PictureNamer implementation.
        Implements the resource naming scheme for the PCC2ng client. */
    class PictureNamer : public game::spec::info::PictureNamer {
     public:
        virtual String_t getHullPicture(const game::spec::Hull& h) const;
        virtual String_t getEnginePicture(const game::spec::Engine& e) const;
        virtual String_t getBeamPicture(const game::spec::Beam& b) const;
        virtual String_t getLauncherPicture(const game::spec::TorpedoLauncher& tl) const;
        virtual String_t getAbilityPicture(const String_t& abilityName) const;
        virtual String_t getPlayerPicture(const game::Player& pl) const;
        virtual String_t getFighterPicture(int raceNr, int playerNr) const;
        virtual String_t getVcrObjectPicture(bool isPlanet, int pictureNumber) const;
    };

} 

#endif
