/**
  *  \file client/picturenamer.cpp
  *  \brief Class client::PictureNamer
  */

#include "client/picturenamer.hpp"
#include "ui/res/resid.hpp"
#include "afl/string/format.hpp"

String_t
client::PictureNamer::getHullPicture(const game::spec::Hull& h) const
{
    // Hull: 'ship.<pic>.<id>'
    // Rationale:
    //   generalizes to 'ship.<pic>' which is the abstraction used by resource.pln, cc.res, etc.
    //   still allows distinction between STF, Merlin, NRS which share the same picture.
    return ui::res::makeResourceId(ui::res::SHIP, h.getInternalPictureNumber(), h.getId());
}

String_t
client::PictureNamer::getEnginePicture(const game::spec::Engine& e) const
{
    // Engine: 'engine.<id>.<fuel usages separated by pipe>
    // Rationale:
    //   generalizes to 'engine.<id>' allowing individual images
    //   allows synthetic creation of engine-fuel-usage charts
    String_t result = ui::res::makeResourceId(RESOURCE_ID("engine"), e.getId());

    int32_t factor;
    for (int i = 1; i <= game::spec::Engine::MAX_WARP && e.getFuelFactor(i, factor); ++i) {
        result += afl::string::Format("%c%d",
                                      i == 1 ? '.' : '|',
                                      factor / (i*i));
    }

    return result;
}

String_t
client::PictureNamer::getBeamPicture(const game::spec::Beam& b) const
{
    // Beam: 'beam.<id>'
    return ui::res::makeResourceId(RESOURCE_ID("beam"), b.getId());
}

String_t
client::PictureNamer::getLauncherPicture(const game::spec::TorpedoLauncher& tl) const
{
    // Torpedo launcher: 'launcher.<id>'
    return ui::res::makeResourceId(RESOURCE_ID("launcher"), tl.getId());
}

String_t
client::PictureNamer::getAbilityPicture(const String_t& abilityName) const
{
    // Ability: 'ability.<name>'
    if (abilityName.empty()) {
        return String_t();
    } else {
        return RESOURCE_ID("ability.") + abilityName;
    }
}

String_t
client::PictureNamer::getPlayerPicture(const game::Player& /*pl*/) const
{
    // As of 20200516, no player pictures
    return String_t();
}
