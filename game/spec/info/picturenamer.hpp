/**
  *  \file game/spec/info/picturenamer.hpp
  *  \brief Interface game::spec::info::PictureNamer
  */
#ifndef C2NG_GAME_SPEC_INFO_PICTURENAMER_HPP
#define C2NG_GAME_SPEC_INFO_PICTURENAMER_HPP

#include "afl/base/deletable.hpp"
#include "game/player.hpp"
#include "game/spec/beam.hpp"
#include "game/spec/engine.hpp"
#include "game/spec/hull.hpp"
#include "game/spec/info/types.hpp"
#include "game/spec/torpedolauncher.hpp"

namespace game { namespace spec { namespace info {

    /** Picture Name Generator.
        Browser will produce picture names (e.g. in PageContent::pictureName).
        The format of these picture names is specified by the application (e.g. ui::res::makeResourceId).
        This interface allows the application to provide appropriate rules.

        These names are passed through unmodified; the only restriction is that they need to be strings.
        Use NullPictureNamer if you don't care. */
    class PictureNamer : public afl::base::Deletable {
     public:
        /** Get name of picture to represent a hull.
            \param h Hull
            \return picture name */
        virtual String_t getHullPicture(const Hull& h) const = 0;

        /** Get name of picture to represent an engine.
            \param e Engine
            \return picture name */
        virtual String_t getEnginePicture(const Engine& e) const = 0;

        /** Get name of picture to represent a beam.
            \param b Beam
            \return picture name */
        virtual String_t getBeamPicture(const Beam& b) const = 0;

        /** Get name of picture to represent a torpedo launcher.
            \param tl Torpedo launcher
            \return picture name */
        virtual String_t getLauncherPicture(const TorpedoLauncher& tl) const = 0;

        /** Get name of picture to represent a ship/racial ability.
            \param abilityName Name of ability (e.g. BasicHullFunction::getPictureName())
            \param flags Flags (to request picture variant)
            \return picture name */
        virtual String_t getAbilityPicture(const String_t& abilityName, AbilityFlags_t flags) const = 0;

        /** Get name of picture to represent a player.
            \param pl Player
            \return picture name */
        virtual String_t getPlayerPicture(const Player& pl) const = 0;

        /** Get name of picture for a player's fighter.
            \param raceNr Race number (PlayerRace)
            \param playerNr Player number
            \return picture name */
        virtual String_t getFighterPicture(int raceNr, int playerNr) const = 0;

        /** Get name of picture to represent VCR unit.
            Note that if the unit has already been resolved to a hull, getHullPicture is used instead.
            \param isPlanet true if this is a planet
            \param pictureNumber Picture number provided by host
            \return picture name */
        virtual String_t getVcrObjectPicture(bool isPlanet, int pictureNumber) const = 0;
    };

} } }

#endif
