/**
  *  \file game/interface/propertylist.cpp
  *  \brief Structure game::interface::PropertyList
  */

#include "game/interface/propertylist.hpp"
#include "afl/data/namemap.hpp"
#include "afl/data/segment.hpp"
#include "game/map/planet.hpp"
#include "game/map/ship.hpp"
#include "interpreter/values.hpp"
#include "util/string.hpp"

void
game::interface::buildPropertyList(PropertyList& out, const game::map::Object* obj, const interpreter::World& world, afl::string::Translator& tx)
{
    // ex console.pas:EnumProperties, CConsoleInput.DoPropertyList
    const afl::data::Segment* pValues = 0;
    const afl::data::NameMap* pNames = 0;
    if (dynamic_cast<const game::map::Ship*>(obj) != 0) {
        pValues = world.shipProperties().get(obj->getId());
        pNames = &world.shipPropertyNames();
        out.title = tx("Ship Properties");
    } else if (dynamic_cast<const game::map::Planet*>(obj) != 0) {
        pValues = world.planetProperties().get(obj->getId());
        pNames = &world.planetPropertyNames();
        out.title = tx("Planet Properties");
    } else {
        // nix
    }

    if (pNames != 0) {
        for (afl::data::NameMap::Index_t i = 0, n = pNames->getNumNames(); i < n; ++i) {
            const afl::data::Value* value = pValues ? pValues->get(i) : 0;
            String_t name = util::formatName(pNames->getNameByIndex(i));
            if (value == 0) {
                out.infos.push_back(PropertyList::Info(name, "Empty", util::SkinColor::Faded));
            } else {
                out.infos.push_back(PropertyList::Info(name, interpreter::toString(value, true), util::SkinColor::Static));
            }
        }
    }
}
