/**
  *  \file game/spec/info/nullpicturenamer.cpp
  *  \brief Class game::spec::info::NullPictureNamer
  */

#include "game/spec/info/nullpicturenamer.hpp"

String_t
game::spec::info::NullPictureNamer::getHullPicture(const Hull& /*h*/) const
{
    return String_t();
}

String_t
game::spec::info::NullPictureNamer::getEnginePicture(const Engine& /*e*/) const
{
    return String_t();
}

String_t
game::spec::info::NullPictureNamer::getBeamPicture(const Beam& /*b*/) const
{
    return String_t();
}

String_t
game::spec::info::NullPictureNamer::getLauncherPicture(const TorpedoLauncher& /*tl*/) const
{
    return String_t();
}

String_t
game::spec::info::NullPictureNamer::getAbilityPicture(const String_t& /*abilityName*/) const
{
    return String_t();
}

String_t
game::spec::info::NullPictureNamer::getPlayerPicture(const Player& /*pl*/) const
{
    return String_t();
}

String_t
game::spec::info::NullPictureNamer::getFighterPicture(int /*raceNr*/, int /*playerNr*/) const
{
    return String_t();
}

String_t
game::spec::info::NullPictureNamer::getVcrObjectPicture(bool /*isPlanet*/, int /*pictureNumber*/) const
{
    return String_t();
}
