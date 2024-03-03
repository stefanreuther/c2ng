/**
  *  \file game/map/info/mission.cpp
  *  \brief Formatting Mission-Related Information
  */

#include "game/map/info/mission.hpp"
#include "afl/io/xml/textnode.hpp"
#include "afl/string/format.hpp"
#include "game/map/chunnelmission.hpp"

using afl::io::xml::TagNode;
using afl::io::xml::TextNode;
using afl::string::Format;

namespace {
    /*
     *  XML Utils
     */

    TagNode& makeTag(TagNode& out, const String_t& tagName)
    {
        TagNode* p = new TagNode(tagName);
        out.addNewChild(p);
        return *p;
    }

    void makeText(TagNode& out, const String_t& text)
    {
        out.addNewChild(new TextNode(text));
    }

    TagNode& makeListItem(TagNode& list)
    {
        return makeTag(list, "li");
    }

    TagNode& makeDetail(TagNode& listItem)
    {
        makeTag(listItem, "br");

        TagNode& content = makeTag(listItem, "font");
        content.setAttribute("color", "dim");
        return content;
    }

    TagNode& makeBold(TagNode& t)
    {
        return makeTag(t, "b");
    }
}

void
game::map::info::renderChunnelFailureReasons(TagNode& list, int failures, afl::string::Translator& tx)
{
    afl::data::StringList_t failureList = formatChunnelFailureReasons(failures, tx);
    for (size_t i = 0, n = failureList.size(); i < n; ++i) {
        makeText(makeListItem(list), failureList[i]);
    }
}

void
game::map::info::renderShipPredictorUsedProperties(TagNode& list, const ShipPredictor& pred, const String_t& missionName, const PlayerList& playerList, afl::string::Translator& tx)
{
    // ex explainPrediction (part)
    makeText(makeListItem(list),
             Format(tx("Movement (%d turn%!1{s%})"), pred.getNumTurns()));

    const ShipPredictor::UsedProperties_t used = pred.getUsedProperties();

    // Mission
    if (used.contains(ShipPredictor::UsedMission)) {
        TagNode& item = makeListItem(list);
        makeText(item, tx("Ship mission"));
        if (!missionName.empty()) {
            makeText(makeDetail(item), missionName);
        } else {
            if (const game::spec::Mission* msn = pred.shipList().missions().findMissionByNumber(pred.getMission(), PlayerSet_t(pred.getRealOwner()))) {
                makeText(makeDetail(item), msn->getName());
            }
        }
    }

    // FCode
    if (used.contains(ShipPredictor::UsedFCode)) {
        TagNode& item = makeListItem(list);
        makeText(item, tx("Ship friendly code"));

        String_t shipFC = pred.getFriendlyCode();
        const game::spec::FriendlyCodeList& fcl = pred.shipList().friendlyCodes();
        game::spec::FriendlyCodeList::Iterator_t it = fcl.findCodeByName(shipFC);
        if (it != fcl.end()) {
            TagNode& detail = makeDetail(item);
            makeText(makeBold(detail), shipFC);
            makeText(detail, ": " + (*it)->getDescription(playerList, tx));
        }
    }

    // Alchemy
    if (used.contains(ShipPredictor::UsedAlchemy)) {
        makeText(makeListItem(list), tx("Alchemy function"));
    }

    // Shipyard
    if (used.contains(ShipPredictor::UsedShipyard)) {
        makeText(makeListItem(list), tx("Starbase shipyard order"));
    }

    // Self repair
    if (used.contains(ShipPredictor::UsedRepair)) {
        makeText(makeListItem(list), tx("Supply repair"));
    }

    // Cloak
    if (used.contains(ShipPredictor::UsedCloak)) {
        makeText(makeListItem(list), tx("Cloaking"));
    }

    // Damage limit
    if (used.contains(ShipPredictor::UsedDamageLimit)) {
        makeText(makeListItem(list), tx("Damage speed limit"));
    }

    // Fighter building
    if (used.contains(ShipPredictor::UsedBuildFighters)) {
        makeText(makeListItem(list), tx("Built fighters"));
    }

    // Towee
    if (used.contains(ShipPredictor::UsedTowee)) {
        TagNode& item = makeListItem(list);
        makeText(item, tx("Towed ship's prediction"));

        String_t toweeName = pred.getTowedShipName();
        if (!toweeName.empty()) {
            makeText(makeDetail(item), toweeName);
        }
    }
}
