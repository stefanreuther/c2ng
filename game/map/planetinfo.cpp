/**
  *  \file game/map/planetinfo.cpp
  *  \brief Functions to obtain information about planets
  */

#include "game/map/planetinfo.hpp"
#include "afl/io/xml/node.hpp"
#include "afl/io/xml/tagnode.hpp"
#include "afl/io/xml/textnode.hpp"
#include "afl/string/format.hpp"
#include "game/map/planet.hpp"
#include "game/map/planetformula.hpp"
#include "game/map/playedshiptype.hpp"
#include "game/map/universe.hpp"
#include "game/tables/happinesschangename.hpp"
#include "game/tables/industrylevel.hpp"
#include "game/tables/mineraldensityclassname.hpp"
#include "game/tables/mineralmassclassname.hpp"
#include "game/tables/nativegovernmentname.hpp"
#include "game/tables/nativeracename.hpp"
#include "game/tables/temperaturename.hpp"
#include "util/math.hpp"
#include "util/string.hpp"
#include "util/unicodechars.hpp"

namespace {
    using afl::io::xml::Node;
    using afl::io::xml::Nodes_t;
    using afl::io::xml::TagNode;
    using afl::io::xml::TextNode;
    using game::config::HostConfiguration;
    using game::Root;
    using game::map::Planet;
    using game::spec::Cost;
    using game::spec::TorpedoLauncher;
    using game::spec::Beam;
    using util::formatAge;

    const int MAX_BAY_LIMIT = 50;

    /*
     *  Misc - move elsewhere?
     */

    String_t getNativeRaceInfo(int race, afl::string::Translator& tx)
    {
        switch (race) {
         case game::HumanoidNatives:   return tx("Tech 10 Hulls on starbase");
         case game::BovinoidNatives:   return tx("Pay additional supplies");
         case game::ReptilianNatives:  return tx("Double mining rates");
         case game::AvianNatives:      return tx("Allow higher taxes");
         case game::AmorphousNatives:  return tx("Don't pay taxes; eat colonists");
         case game::InsectoidNatives:  return tx("Double tax collection");
         case game::AmphibianNatives:  return tx("Tech 10 Beams on starbase");
         case game::GhipsoldalNatives: return tx("Tech 10 Engines on starbase");
         case game::SiliconoidNatives: return tx("Tech 10 Torpedoes on starbase");
            // This and the following are proposed native races.
         case 10: return tx("Ships start with experience");
         case 11: return tx("Base fights with extra torpedo tubes");
         case 12: return tx("Can detect cloaked ships");
         case 13: return tx("Don't pay taxes; eat colonists");
         case 14: return tx("Drain fuel from orbiting ships");
         case 15: return tx("Don't pay taxes; extend sensor range");
         default: return String_t();
        }
    }

    int32_t hostSpecificDivide(int32_t a, int32_t b, const game::HostVersion& host)
    {
        // ex scr-env.cc:pdiv
        // ex envscan.pas:PDiv
        if (host.isPHost()) {
            return a/b;
        } else {
            return util::divideAndRoundToEven(a, b, 0);
        }
    }

    int computeNumStructures(int d, int k)
    {
        /* Solve d = k*n*(n-1) for n. Taken from PCC 1.x.
           Actually, this is n=Round(Sqrt(d/k)), but in integer only and thus
           with "perfect" precision.

           Note that this does not work for k>4; it returns one too much.
           The reason is that the actual formula is not
                 d = k*n*(n-1)
           but   d = k*n*(n-1) + 0.25*k,
           which is the inverse of
                 n = rountToInt(sqrt(d/k))
           or    n = trunc(sqrt(d/k) + 0.5)

           With  n = sqrt(d/k) + 0.5, we get

                 (n - 0.5)**2     = d/k
                 n*n - n - 0.25   = d/k
               k*n*(n-1) - 0.25*k = d

           Fixing this would mean starting with sum=1+(d-1)/4.
           (I still don't know where the 1 offset comes from...) */
        int n = 1;
        int sum = 1;
        while (sum < d) {
            sum += 2*k*n;
            ++n;
        }
        if (sum > d) {
            --n;
        }
        return n;
    }

    /*
     *  Building DOM nodes
     */

    /** Make a list.
        \param list Node list
        \return reference to newly-allocated "<ul>" added to node list */
    TagNode& makeList(Nodes_t& list)
    {
        TagNode* pNode = new TagNode("ul");
        list.pushBackNew(pNode);
        pNode->setAttribute("class", "compact");
        return *pNode;
    }

    /** Add item to list.
        \param listNode Result of makeList()
        \param text     Text of new list item (empty to add none)
        \return reference to newly-allocated "<li>" added to list */
    TagNode& addItem(TagNode& listNode, String_t text)
    {
        // ex addItem(RichDocument& doc, RichText text) (sort-of)
        TagNode* pNode = new TagNode("li");
        listNode.addNewChild(pNode);
        if (!text.empty()) {
            pNode->addNewChild(new TextNode(text));
        }
        return *pNode;
    }

    /** Add colored text.
        \param textNode List item node (result of addItem(), addDetail())
        \param color    Color name
        \param text     Text
        \return textNode */
    TagNode& addColoredText(TagNode& textNode, String_t color, String_t text)
    {
        TagNode* child = new TagNode("font");
        textNode.addNewChild(child);
        child->setAttribute("color", color);
        child->addNewChild(new TextNode(text));
        return textNode;
    }

    /** Add newline.
        \param textNode List item node (result of addItem(), addDetail())
        \return textNode */
    TagNode& addNewline(TagNode& textNode)
    {
        textNode.addNewChild(new TagNode("br"));
        return textNode;
    }

    /** Add detail info.
        Detail information is produced as a nested list.
        addDetail() should only be called after addItem() has been called at least once.

        \param listNode Result of makeList()
        \param bullet   Bullet to use
        \param text     Text (empty to add none)
        \return reference to newly-allocated "<li>" added to detail list */
    TagNode& addDetail(TagNode& listNode, String_t bullet, String_t text)
    {
        // ex addDetail(RichDocument& doc, const char* bullet, RichText text) (sort-of)
        // itemNode is the outer <ul>. Locate inner <li>.
        const Nodes_t& listChildren = listNode.getChildren();
        TagNode* outerItem = listChildren.empty() ? 0 : dynamic_cast<TagNode*>(listChildren.back());
        if (outerItem == 0) {
            // Failure. Cannot add detail here. Fall back to addItem to not lose the information, but losing the formatting.
            // This should not normally happen.
            return addItem(listNode, text);
        } else {
            // Check whether it already ends with an <ul>.
            const Nodes_t& itemChildren = outerItem->getChildren();
            TagNode* ul = itemChildren.empty() ? 0 : dynamic_cast<TagNode*>(itemChildren.back());
            if (ul == 0 || ul->getName() != "ul") {
                ul = new TagNode("ul");
                outerItem->addNewChild(ul);
                ul->setAttribute("class", "compact");
            }

            // Add list item.
            TagNode* pNode = new TagNode("li");
            ul->addNewChild(pNode);
            pNode->setAttribute("bullet", bullet);
            (void) bullet;
            if (!text.empty()) {
                pNode->addNewChild(new TextNode(text));
            }
            return *pNode;
        }
    }

    /*
     *  Document Fragments
     */

    void showSupport(TagNode& list, const Planet& pl, const Root& root, int player, afl::string::Translator& tx)
    {
        // ex WPlanetInfoBox::showSupport(RichDocument& doc, int player)
        // ex envscan.pas:ShowSupport
        util::NumberFormatter fmt(root.userConfiguration().getNumberFormatter());
        int32_t clans;
        if (game::map::getMaxSupportedColonists(pl, root.hostConfiguration(), root.hostVersion(), player).get(clans)) {
            addItem(list, afl::string::Format(tx("Supports %s %ss"),
                                              fmt.formatPopulation(clans),
                                              root.playerList().getPlayerName(player, game::Player::AdjectiveName)));

            /* Host-style CDR: according to PCC 1.x, this rule is active unless a special
               Rebel/Robot/Colony/Klingon rule kicks in. */
            game::HostVersion::Kind k = root.hostVersion().getKind();
            int temp;
            if ((k == game::HostVersion::Host || k == game::HostVersion::SRace) && pl.getTemperature().get(temp)) {
                /* CDR is only applicable for Climate <16 or >=86.
                   - Rebel rule: Climate > 80
                   - Kli/Reb/Rob/Col rule: Climate < 20, and main rule yields less than 60.
                   getMaxSupportedColonists() already increases those results to 60.
                   However, since we're doing Tim-Host only, 60 can only be produced by
                   this very rule. */
                int race = root.hostConfiguration().getPlayerRaceNumber(player);
                int cdr = root.hostConfiguration()[HostConfiguration::ClimateDeathRate](player);
                if ((temp > 84 || temp <= 14)
                    && cdr != 0
                    && race != 7
                    && !(race == 10 && temp < 20)
                    && !((race == 4 || race >= 9) && temp > 80 && clans <= 60))
                {
                    int32_t limit = clans * 100 / cdr;
                    if (limit > 250000) {
                        limit = 250000;
                    }
                    addDetail(list, UTF_HYPHEN, afl::string::Format(tx("won't die if less than %s"), fmt.formatPopulation(limit)));
                }
            }
        }
    }

    void addAge(TagNode& list, int currentTurn, int historyTurn, afl::string::Translator& tx)
    {
        // ex WPlanetInfoBox::showAge(RichDocument& doc, int turn)
        if (historyTurn > 0) {
            int age = currentTurn - historyTurn;
            String_t color = (age <= 0
                              ? "green"
                              : age >= 3
                              ? "red"
                              : "yellow");
            TagNode& detailNode = addDetail(list, UTF_STOPWATCH, String_t());
            addColoredText(detailNode, color, formatAge(currentTurn, historyTurn, tx));
        }
    }

    void addBaseTax(TagNode& list, const Planet& pl, int viewpointPlayer, String_t label, const Root& root, int happyTarget, afl::string::Translator& tx)
    {
        // ex formatBaseTax
        // ex envscan.pas:AddBaseTax
        int tax, race, gov;
        int32_t pop;
        if (getNativeBaseTax(pl, viewpointPlayer, root.hostConfiguration(), root.hostVersion(), happyTarget).get(tax)
            && pl.getNativeRace().get(race)
            && pl.getNativeGovernment().get(gov)
            && pl.getNatives().get(pop))
        {
            // How many colonists needed to collect that?
            int32_t due = game::map::getNativeDue(tax, race, gov, pop, viewpointPlayer, root.hostConfiguration(), root.hostVersion());
            int rate = root.hostConfiguration()[HostConfiguration::NativeTaxRate](viewpointPlayer);
            if (race == game::InsectoidNatives) {
                rate *= 2;
            }
            int32_t colonists = (rate > 0 ? due * 100 / rate : due);

            // Show info
            util::NumberFormatter fmt = root.userConfiguration().getNumberFormatter();
            if (colonists == due) {
                addItem(list, afl::string::Format(tx("%s: %d%% (%d mc)"), label, tax, fmt.formatNumber(due)));
            } else {
                addItem(list, afl::string::Format(tx("%s: %d%% (%d mc, with %d colonist clan%!1{s%})"), label, tax, fmt.formatNumber(due), fmt.formatNumber(colonists)));
            }
        }
    }

    void addAssimilation(TagNode& list, const Planet& pl, const Root& root, int viewpointPlayer, const game::map::UnloadInfo& unload, afl::string::Translator& tx)
    {
        // ex WPlanetInfoBox::showAssimilation
        // ex envscan.pas:ShowAssim
        int nrace;
        int32_t n;

        // Do we have natives we can talk about?
        if (!pl.getNativeRace().get(nrace) || nrace <= 0 || nrace == game::AmorphousNatives
            || !pl.getNatives().get(n))
        {
            return;
        }

        // Figure out a colonist count */
        bool by;
        int32_t clans;
        int race;
        if (pl.getOwner(race) && root.hostConfiguration().getPlayerRaceNumber(race) == 6 && pl.getCargo(game::Element::Colonists).get(clans)) {
            // Planet is Borg, use clans/race
            by = false;
        } else if (root.hostConfiguration().getPlayerRaceNumber(viewpointPlayer) == 6) {
            // We are Borg. Resistance is futile.
            clans = unload.hostileUnload;
            race = viewpointPlayer;
            by = true;
        } else {
            // No Borg in sight.
            return;
        }

        // Defaults
        if (clans <= 0) {
            clans = 10;
            by = true;
        }

        // Compute turns until natives are gone
        int turns = 0;
        int32_t c = clans;
        while (n > 0 && turns < 100) {
            int32_t ass = c * root.hostConfiguration()[HostConfiguration::BorgAssimilationRate](race) / 100;
            n -= ass;
            c += ass;
            ++turns;
        }

        String_t info = afl::string::Format(by
                                            ? (n > 0
                                               ? tx("Assimilated in >%d turn%!1{s%} by %d clan%!1{s%}")
                                               : tx("Assimilated in %d turn%!1{s%} by %d clan%!1{s%}"))
                                            : (n > 0
                                               ? tx("Assimilated in >%d turn%!1{s%}")
                                               : tx("Assimilated in %d turn%!1{s%}")),
                                            turns, root.userConfiguration().getNumberFormatter().formatNumber(clans));
        addItem(list, info);
    }

    void addPlanetAttack(TagNode& list, const HostConfiguration& config, int viewpointPlayer, int32_t clans, bool againstColonists, afl::string::Translator& tx)
    {
        // ex WPlanetInfoBox::showPlanetAttack
        // ex envscan.pas:ShowPillage
        int32_t extra;
        String_t fmt;
        switch (config.getPlayerMissionNumber(viewpointPlayer)) {
         case 4:
            /* Klingons - Pillage */
            fmt = tx("Pillage max. %d turn%!1{s%}");
            extra = againstColonists ? 20 : 120;
            break;
         case 10:
            /* Rebel Ground Attack */
            if (!againstColonists) {
                return;
            }
            fmt = tx("RGA max. %d turn%!1{s%}");
            extra = 0;
            break;
         default:
            /* No ground attack mission */
            return;
        }

        /* Stupid case */
        if (clans == 0) {
            return;
        }

        /* Compute time. This clears a planet in at most about 50 turns,
           so there is no need to limit it. */
        int turns = 0;
        while (clans > 0) {
            clans = (clans * 4 / 5) - extra;
            ++turns;
        }

        addItem(list, afl::string::Format(fmt, turns));
    }

    void addBeamDown(TagNode& list, const Planet& pl, const Root& root, const int viewpointPlayer, const game::map::UnloadInfo& unload, afl::string::Translator& tx)
    {
        // ex WPlanetInfoBox::showBeamDown
        // ex envscan.pas:ShowBeamDown
        util::NumberFormatter fmt(root.userConfiguration().getNumberFormatter());

        // What are we beaming down?
        addItem(list, afl::string::Format(unload.hostileUnloadIsAssumed
                                          ? tx("Assuming, we'd beam down %d clan%!1{s%}.")
                                          : tx("We are beaming down %d clan%!1{s%}."),
                                          fmt.formatNumber(unload.hostileUnload)));

        // Will that be a fight after all?
        int planetOwner;
        if (!pl.getOwner(planetOwner) || planetOwner == 0 || planetOwner == viewpointPlayer) {
            return;
        }

        // Do we know how many enemies are there?
        int32_t theirClans;
        if (!pl.getCargo(game::Element::Colonists).get(theirClans) || theirClans == 0) {
            return;
        }

        // Okay, there will be ground combat.
        int theirDefense = getMaxBuildings(pl, game::DefenseBuilding, root.hostConfiguration(), theirClans).orElse(0);

        // Minimum power: assuming 0 defense
        int32_t theirRatioLo = root.hostConfiguration()[HostConfiguration::GroundDefenseFactor](planetOwner);
        int32_t theirPowerLo = theirRatioLo * theirClans;

        // Maximum power: assuming full defense
        int32_t theirPowerHi;
        int32_t theirRatioHiX20;
        if (root.hostVersion().isPHost()) {
            theirRatioHiX20 = 20*theirRatioLo + theirDefense;
            theirPowerHi    = (theirClans * theirRatioHiX20) / 20;
        } else {
            theirRatioHiX20 = theirRatioLo * (theirDefense + 20);
            theirPowerHi    = util::divideAndRoundToEven(theirClans * theirRatioHiX20, 20, 0);
        }

        // Compute my power
        int32_t myRatio = root.hostConfiguration()[HostConfiguration::GroundKillFactor](viewpointPlayer);
        int32_t myPower = myRatio * unload.hostileUnload;

        // Avoid crash on bogus config
        if (myRatio == 0 || theirRatioLo == 0) {
            return;
        }

        if (myPower > theirPowerHi) {
            // Guaranteed win
            TagNode& item = addDetail(list, UTF_CHECK_MARK, String_t());
            addColoredText(item, "green", unload.hostileUnloadIsAssumed
                           ? tx("We'd win this ground combat.")
                           : tx("We'll win this ground combat."));
            addNewline(item);

            int32_t a = hostSpecificDivide(myPower - theirPowerLo, myRatio, root.hostVersion());
            int32_t b = hostSpecificDivide(myPower - theirPowerHi, myRatio, root.hostVersion());
            String_t text;
            if (a == b) {
                text += afl::string::Format(tx("%d clan%1{ survives%|s survive%}."), fmt.formatNumber(a));
            } else {
                text += afl::string::Format(tx("%d to %d clans survive."), fmt.formatNumber(a), fmt.formatNumber(b));
            }
            addColoredText(item, "green", text);
        } else if (myPower < theirPowerLo) {
            // Guaranteed loss
            TagNode& item = addDetail(list, UTF_BALLOT_CROSS, String_t());
            addColoredText(item, "red", unload.hostileUnloadIsAssumed
                           ? tx("We'd lose this ground combat.")
                           : tx("We'll lose this ground combat."));
            addNewline(item);

            int32_t a = hostSpecificDivide(theirPowerLo - myPower, theirRatioLo, root.hostVersion());
            int32_t b = hostSpecificDivide(20*(theirPowerHi - myPower), theirRatioHiX20, root.hostVersion());
            String_t text;
            if (a == b) {
                text = afl::string::Format(tx("%d of their clans survive%1{s%}."), fmt.formatNumber(a));
            } else {
                text = afl::string::Format(tx("%d to %d of their clans survive."), fmt.formatNumber(a), fmt.formatNumber(b));
            }
            addColoredText(item, "red", text);
        } else {
            // Depends on defense
            TagNode& item = addDetail(list, UTF_BALLOT_CROSS, String_t());

            int32_t a = hostSpecificDivide(myPower - theirPowerLo, myRatio, root.hostVersion());
            int32_t b = hostSpecificDivide(20*(theirPowerHi - myPower), theirRatioHiX20, root.hostVersion());
            if (a != 0 && b != 0) {
                addColoredText(item, "yellow", afl::string::Format(tx("Chance to win ground combat: %d%%"),
                                                                   (99*(myPower - theirPowerLo+1)) / (theirPowerHi - theirPowerLo+1)));
                addNewline(item);
            }
            addColoredText(item, "yellow", afl::string::Format(tx("Up to %d of our clans survive%1{s%}."), fmt.formatNumber(a)));
            addNewline(item);
            addColoredText(item, "yellow", afl::string::Format(tx("Up to %d of their clans survive%1{s%}."), fmt.formatNumber(b)));
        }
    }

    void addHappinessChange(TagNode& list, game::NegativeProperty_t change, afl::string::Translator& tx)
    {
        int n;
        if (change.get(n)) {
            addColoredText(addDetail(list, UTF_HYPHEN, String_t()),
                           n < 0 ? "red" : "green",
                           /* This format string means: display numerical 'change' only if it's nonzero */
                           afl::string::Format("%s%!d%!0{ (%1$+d)%}",
                                               game::tables::HappinessChangeName(tx)(n), n));
        }
    }

    /*
     *  DefenseEffectInfos_t
     */

    void addLine(game::map::DefenseEffectInfos_t& result, const String_t& name, int next, int have, int max, bool isDetail)
    {
        // ex WPlanetDefenseEffectWidget::showLine (part)
        result.push_back(game::map::DefenseEffectInfo(name, next, (next + have <= max), isDetail));
    }

    bool canDoFighterPrediction(const game::HostVersion& host, const HostConfiguration& config)
    {
        // We can do this for THost
        if (!host.isPHost()) {
            return true;
        }

        // We can do this for PHost and classic combat if all FighterBeamExplosive etc. are constant.
        // For 101% precision, we'd also have to check that the options are not experience-modified,
        // but that'd take away a useful feature. Note that Shield/DamageScaling can be variable, as
        // they're taken from the victim, which would be us.
        if (host.isPHost() && !config[HostConfiguration::AllowAlternativeCombat]()) {
            return config[HostConfiguration::FighterBeamExplosive].isAllTheSame()
                && config[HostConfiguration::FighterBeamKill].isAllTheSame();
        }

        return false;
    }

    void computeFighterEffects(int& shield, int& damage, int mass, int owner, const game::HostVersion& host, const HostConfiguration& config)
    {
        if (!host.isPHost()) {
            // THost formula
            shield = util::divideAndRoundToEven(80*2,      mass+1, 1);
            damage = util::divideAndRoundToEven(80*shield, mass+1, 1);
        } else {
            // PHost non-AC formula
            int boom = (config[HostConfiguration::ShieldDamageScaling](owner) * config[HostConfiguration::FighterBeamExplosive](1)
                        + config[HostConfiguration::ShieldKillScaling](owner) * config[HostConfiguration::FighterBeamKill](1));
            shield = util::divideAndRound(boom, mass+1) + 1;
            damage = util::divideAndRound(shield * config[HostConfiguration::CrewKillScaling](owner), mass+1) + 1;
        }
    }
}


// Retrieve information about minerals on a planet.
game::map::PlanetMineralInfo
game::map::packPlanetMineralInfo(const Planet& pl,
                                 Element::Type ele,
                                 int turnNr,
                                 const game::config::HostConfiguration& config, const HostVersion& host,
                                 IntegerProperty_t mineOverride,
                                 afl::string::Translator& tx)
{
    PlanetMineralInfo result;

    // Status
    const int mineralTime = pl.getHistoryTimestamp(Planet::MineralTime);
    result.status = (pl.hasFullPlanetData()
                     ? PlanetMineralInfo::Reliable
                     : mineralTime != 0
                     ? PlanetMineralInfo::Scanned
                     : PlanetMineralInfo::Unknown);

    // Age
    if (mineralTime != 0) {
        result.age = turnNr - mineralTime;
        result.ageLabel = formatAge(turnNr, mineralTime, tx);
    }

    // Amounts
    result.minedAmount = pl.getCargo(ele);
    result.groundAmount = pl.getOreGround(ele);
    result.density = pl.getOreDensity(ele);

    int32_t ground = 0;
    if (result.groundAmount.get(ground)) {
        result.groundSummary = game::tables::MineralMassClassName(tx)(ground);
    }
    int density;
    if (result.density.get(density)) {
        result.densitySummary = game::tables::MineralDensityClassName(tx)(density);
    }

    // Mining
    IntegerProperty_t numMines = mineOverride;
    if (!numMines.isValid()) {
        numMines = pl.getNumBuildings(MineBuilding);
    }
    int n;
    if (numMines.get(n)) {
        int capacity;
        if (getMiningCapacity(pl, config, host, ele, n).get(capacity)) {
            result.miningPerTurn = capacity;
            if (capacity > 0) {
                result.miningDuration = std::min((ground + (capacity-1)) / capacity, MAX_MINING_DURATION);
            }
        }
    }
    return result;
}

// Retrieve textual information about planet climate.
void
game::map::describePlanetClimate(afl::io::xml::Nodes_t& nodes,
                                 const Planet& pl,
                                 int /*turnNr*/,
                                 const Root& root,
                                 int viewpointPlayer,
                                 afl::string::Translator& tx)
{
    int temp;
    TagNode& list = makeList(nodes);
    if (pl.getTemperature().get(temp)) {
        addItem(list, afl::string::Format(tx("Climate type: %s"), game::tables::TemperatureName(tx)(temp)));
        addItem(list, afl::string::Format(tx("Average temperature: %d" "\xC2\xB0" "\x46"), temp));

        int planetOwner;
        if (pl.getOwner(planetOwner) && planetOwner > 0 && planetOwner != viewpointPlayer) {
            showSupport(list, pl, root, planetOwner, tx);
        }
        showSupport(list, pl, root, viewpointPlayer, tx);
    } else {
        addItem(list, tx("No information on climate available."));
    }
}

// Retrieve textual information about planet natives.
void
game::map::describePlanetNatives(afl::io::xml::Nodes_t& nodes,
                                 const Planet& pl,
                                 int turnNr,
                                 const Root& root,
                                 int viewpointPlayer,
                                 const UnloadInfo& unload,
                                 afl::string::Translator& tx)
{
    util::NumberFormatter fmt(root.userConfiguration().getNumberFormatter());
    TagNode& list = makeList(nodes);

    int race;
    if (!pl.getNativeRace().get(race)) {
        int32_t pop;
        if (pl.getNatives().get(pop)) {
            addItem(list, tx("Unknown native race."));
            addItem(list, afl::string::Format(tx("Population: %s"), fmt.formatPopulation(pop)));
        } else if (pl.isKnownToHaveNatives()) {
            addItem(list, tx("Planet has natives."));
        } else {
            addItem(list, tx("No information on natives available."));
        }
    } else if (race == 0) {
        addItem(list, tx("No native population."));
    } else {
        addItem(list, afl::string::Format(tx("Native race: %s"), game::tables::NativeRaceName(tx)(race)));

        String_t info = getNativeRaceInfo(race, tx);
        if (!info.empty()) {
            addDetail(list, UTF_HYPHEN, info);
        }

        int32_t pop = 0;
        bool popOK = pl.getNatives().get(pop);
        if (race == BovinoidNatives && popOK) {
            int32_t forMe = getBovinoidSupplyContribution(pop, viewpointPlayer, root.hostConfiguration(), root.hostVersion());
            addDetail(list, UTF_HYPHEN, afl::string::Format(tx("%d kt supplies per turn"), fmt.formatNumber(forMe)));

            int owner;
            if (pl.getOwner(owner) && owner != 0) {
                int32_t forThem = getBovinoidSupplyContribution(pop, owner, root.hostConfiguration(), root.hostVersion());
                if (forThem != forMe) {
                    addDetail(list, UTF_HYPHEN, afl::string::Format(tx("%d kt supplies per turn for %s"),
                                                                    fmt.formatNumber(forThem),
                                                                    root.playerList().getPlayerName(owner, Player::ShortName)));
                }
            }
        }

        if (popOK) {
            addItem(list, afl::string::Format(tx("Population: %s"), fmt.formatPopulation(pop)));
        }

        int gov;
        if (pl.getNativeGovernment().get(gov)) {
            addItem(list, afl::string::Format(tx("Government: %s (%d%%)"),
                                              game::tables::NativeGovernmentName(tx)(gov),
                                              20 * gov));
        }
    }

    if (!pl.hasFullPlanetData()) {
        addAge(list, turnNr, pl.getHistoryTimestamp(Planet::NativeTime), tx);
    }

    // Show taxes
    // NOTE: those are in the 'N' menu in PCC 1.x
    int32_t pop;
    int gov;
    if (pl.getNativeRace().get(race)
        && pl.getNatives().get(pop)
        && pl.getNativeGovernment().get(gov)
        && race != 0
        && race != AmorphousNatives)
    {
        // FIXME? If government is not known, PCC1 will assume feudalism. Does this happen?
        addBaseTax(list, pl, viewpointPlayer, tx("Base Tax Rate"), root,   0, tx);
        addBaseTax(list, pl, viewpointPlayer, tx("Max Tax Rate"),  root, -30, tx);
    }

    // Attacks
    addAssimilation(list, pl, root, viewpointPlayer, unload, tx);
    if (pl.getNatives().get(pop)) {
        addPlanetAttack(list, root.hostConfiguration(), viewpointPlayer, pop, false, tx);
    }
}

// Retrieve textual information about planet colony.
void
game::map::describePlanetColony(afl::io::xml::Nodes_t& nodes,
                                const Planet& pl,
                                int turnNr,
                                const Root& root,
                                int viewpointPlayer,
                                const UnloadInfo& unload,
                                afl::string::Translator& tx)
{
    util::NumberFormatter fmt(root.userConfiguration().getNumberFormatter());
    TagNode& list = makeList(nodes);

    // Colony
    int owner;
    if (!pl.getOwner(owner)) {
        addItem(list, tx("No information on colonists available."));
    } else if (owner == 0) {
        addItem(list, tx("No colonists."));
    } else {
        addItem(list, afl::string::Format(tx("Colonists: %s"), root.playerList().getPlayerName(owner, Player::ShortName)));

        int32_t pop;
        if (pl.getCargo(Element::Colonists).get(pop)) {
            addItem(list, afl::string::Format(tx("Population: %s"), fmt.formatPopulation(pop)));
            addPlanetAttack(list, root.hostConfiguration(), viewpointPlayer, pop, true, tx);
        }
    }

    // Industry
    {
        String_t industry;
        int factories = 0, mines = 0, defense = 0;
        bool factoriesOK = pl.getNumBuildings(FactoryBuilding).get(factories);
        bool minesOK     = pl.getNumBuildings(MineBuilding).get(mines);
        bool defenseOK   = pl.getNumBuildings(DefenseBuilding).get(defense);
        if (!factoriesOK && !minesOK) {
            int level;
            if (pl.getIndustryLevel(root.hostVersion()).get(level)) {
                util::addListItem(industry, ", ", afl::string::Format(tx("%s industrial activity"), game::tables::IndustryLevel(tx)(level)));
            }
        } else {
            if (factoriesOK && factories > 0) {
                util::addListItem(industry, ", ", afl::string::Format(tx("%d factor%!1{ies%|y%}"), fmt.formatNumber(factories)));
            }
            if (minesOK && mines > 0) {
                util::addListItem(industry, ", ", afl::string::Format(tx("%d mine%!1{s%}"), fmt.formatNumber(mines)));
            }
            if (defenseOK && defense > 0) {
                util::addListItem(industry, ", ", afl::string::Format(tx("%d DP%!1{s%}"), fmt.formatNumber(defense)));
            }
        }
        if (!industry.empty()) {
            addItem(list, industry);
        }
        if (!pl.hasFullPlanetData()) {
            addAge(list, turnNr, pl.getHistoryTimestamp(Planet::ColonistTime), tx);
        }
    }

    // Funds
    {
        String_t funds;
        int32_t n;
        if (pl.getCargo(Element::Money).get(n)) {
            util::addListItem(funds, ", ", afl::string::Format(tx("%d mc"), fmt.formatNumber(n)));
        }
        if (pl.getCargo(Element::Supplies).get(n)) {
            util::addListItem(funds, ", ", afl::string::Format(tx("%d suppl%1{y%|ies%}"), fmt.formatNumber(n)));
        }
        if (!funds.empty()) {
            addItem(list, funds);
        }
        if (!pl.hasFullPlanetData()) {
            addAge(list, turnNr, pl.getHistoryTimestamp(Planet::CashTime), tx);
        }
    }

    // FCode
    {
        String_t fc;
        if (pl.getFriendlyCode().get(fc)) {
            if (pl.hasFullPlanetData()) {
                addItem(list, afl::string::Format(tx("Friendly code: %s"), fc));
            } else {
                addItem(list, afl::string::Format(tx("Last known friendly code: %s"), fc));
            }
        }
    }

    // Base
    if (pl.hasBase()) {
        addItem(list, tx("Starbase present"));
    } else if (pl.isBuildingBase()) {
        addItem(list, tx("Starbase being built"));
    } else {
        // no base
    }

    // Unload information
    if (unload.friendlyUnload != 0) {
        addColoredText(addItem(list, String_t()), "yellow", afl::string::Format(tx("Friendly unload: %d clan%!1{s%}"), fmt.formatNumber(unload.friendlyUnload)));
    }
    if (unload.hostileUnload != 0) {
        if (unload.hostileUnloadIsAssault) {
            addColoredText(addItem(list, String_t()), "red", tx("Imperial Assault!"));
        }
        addBeamDown(list, pl, root, viewpointPlayer, unload, tx);
    }
}


// Retrieve textual information about planet building effects.
void
game::map::describePlanetBuildingEffects(afl::io::xml::Nodes_t& nodes,
                                         const Planet& pl,
                                         const Root& root,
                                         afl::string::Translator& tx)
{
    // ex WPlanetStructureEffectWidget::drawContent (part), pdata.pas:ShowHappinessChange, pdata.pas:ShowSensorRate
    util::NumberFormatter fmt(root.userConfiguration().getNumberFormatter());
    TagNode& list = makeList(nodes);

    // Sensor visibility
    {
        String_t vis;
        String_t color = "yellow";

        int level;
        if (getSensorVisibility(pl, root.hostConfiguration(), root.hostVersion()).get(level)) {
            util::addListItem(vis, ", ", fmt.formatNumber(level) + "%");
            if (level == 0) {
                color = "green";
            }
        }
        if (pl.getIndustryLevel(root.hostVersion()).get(level)) {
            util::addListItem(vis, ", ", game::tables::IndustryLevel(tx)(level));
        }

        if (!vis.empty()) {
            addColoredText(addItem(list, tx("Sensor visibility:") + " "), color, vis);
        }
    }

    // Colonist taxation
    int32_t rem_inc = 0;
    {
        int rate;
        int32_t income;
        if (pl.getColonistTax().get(rate)
            && getColonistDueLimited(pl, root.hostConfiguration(), root.hostVersion(), rate, rem_inc).get(income))
        {
            addColoredText(addItem(list, tx("Colonist Tax:") + " "), "green",
                           afl::string::Format(tx("%d%% (%d mc)"), rate, fmt.formatNumber(income)));
            addHappinessChange(list, getColonistChange(pl, root.hostConfiguration(), root.hostVersion()), tx);
        }
    }

    // Native taxation
    int race;
    if (pl.getNativeRace().get(race) && race != 0) {
        int rate;
        int32_t income;
        if (pl.getNativeTax().get(rate)
            && getNativeDueLimited(pl, root.hostConfiguration(), root.hostVersion(), rate, rem_inc).get(income))
        {
            addColoredText(addItem(list, tx("Native Tax:") + " "), "green",
                           afl::string::Format(tx("%d%% (%d mc)"), rate, fmt.formatNumber(income)));
            addHappinessChange(list, getNativeChange(pl, root.hostVersion()), tx);
        }
    }
}

// Retrieve textual information about planet defense effects.
void game::map::describePlanetDefenseEffects(DefenseEffectInfos_t& result,
                                             const Planet& pl,
                                             const Root& root,
                                             const game::spec::ShipList& shipList,
                                             const UnitScoreDefinitionList& planetScores,
                                             afl::string::Translator& tx)
{
    // ex WPlanetDefenseEffectWidget::drawContent, CDefenseStrengthTile.DrawData
    // FIXME: as of 20191227, we don't have a UI-independant table representation. When we have one, produce that instead of DefenseEffectInfos_t.
    // FIXME: for now, do not try too hard to deal with partial information

    // Quick exit if owner not known to simplify following code
    int planetOwner;
    if (!pl.getOwner(planetOwner)) {
        return;
    }

    const HostVersion& host = root.hostVersion();
    const game::config::HostConfiguration& config = root.hostConfiguration();

    const int defenseWant = pl.getNumBuildings(DefenseBuilding).orElse(0);
    const int sbdWant     = pl.getNumBuildings(BaseDefenseBuilding).orElse(0);
    const int totalWant   = defenseWant + sbdWant;

    const int defenseMax  = std::max(defenseWant, getMaxBuildings(pl, DefenseBuilding,     config).orElse(0));
    const int sbdMax      = std::max(sbdWant,     getMaxBuildings(pl, BaseDefenseBuilding, config).orElse(0));
    const int totalMax    = defenseMax + sbdMax;

    // Beam count
    {
        int n = computeNumStructures(totalWant, 3);
        int max = (host.isPHost() && config[HostConfiguration::AllowAlternativeCombat]() ? 20 : 10);
        int next;
        if (n >= max) {
            n    = max;
            next = 0;
        } else {
            next = n*(n+1)*3+1 - totalWant;
        }

        addLine(result, afl::string::Format(tx("%d beam%!1{s%}"), n), next, totalWant, totalMax, false);
    }

    // Beam type
    if (totalWant > 0) {
        int n = computeNumStructures(defenseWant, 2);
        if (pl.hasBase()) {
            n = std::max(n, pl.getBaseTechLevel(BeamTech).orElse(0));
        }
        if (n > 0) {
            int next;
            if (n >= 10) {
                n    = 10;
                next = 0;
            } else {
                next = n*(n+1)*2+1 - defenseWant;
            }

            if (const Beam* pBeam = shipList.beams().get(n)) {
                addLine(result, pBeam->getName(shipList.componentNamer()), next, defenseWant, defenseMax, true);
            }
        }
    }

    // Fighters
    {
        // Fighters
        int n = computeNumStructures(defenseWant, 1);
        int next = n*(n+1)+1 - defenseWant;
        int add = 0;
        if (pl.hasBase()) {
            pl.getCargo(Element::Fighters).get(add);
        }
        addLine(result, afl::string::Format(tx("%d fighter%!1{s%}"), n + add), next, defenseWant, defenseMax, false);

        // Bays
        if (pl.hasBase() && host.isPHost()) {
            n += 5;
        }
        if (n >= MAX_BAY_LIMIT) {
            n = MAX_BAY_LIMIT;
            next = 0;
        }
        addLine(result, afl::string::Format(tx("%d fighter bay%!1{s%}"), n), next, defenseWant, defenseMax, false);
    }

    // Torpedoes
    if (host.isPHost() && config[HostConfiguration::PlanetsHaveTubes]()) {
        // Launchers
        int n = computeNumStructures(totalWant, 4);
        int next;
        if (n >= 20) {
            n    = 20;        // FIXME: Non-AC (also in PCC2)
            next = 0;
        } else {
            next = n*(n+1)*4+1 - totalWant;
        }
        addLine(result, afl::string::Format(tx("%d torpedo launcher%!1{s%}"), n), next, totalWant, totalMax, false);

        if (n > 0) {
            // Type
            int tech = computeNumStructures(defenseWant, 2);
            if (pl.hasBase()) {
                tech = std::max(tech, pl.getBaseTechLevel(TorpedoTech).orElse(0));
            }
            if (tech >= 10) {
                tech = 10;
                next = 0;
            } else {
                next = tech*(tech+1)*2+1 - defenseWant;
            }

            if (const TorpedoLauncher* pTorp = shipList.launchers().get(tech)) {
                addLine(result, pTorp->getName(shipList.componentNamer()), next, defenseWant, defenseMax, true);
            }

            // Torpedoes
            int32_t total = 0;
            if (pl.hasBase() && config[HostConfiguration::UseBaseTorpsInCombat](planetOwner)) {
                int32_t totalCost = 0;
                for (const TorpedoLauncher* pTorp = shipList.launchers().findNext(0); pTorp != 0; pTorp = shipList.launchers().findNext(pTorp->getId())) {
                    totalCost += pTorp->torpedoCost().get(Cost::Money)
                        * pl.getCargo(Element::fromTorpedoType(pTorp->getId())).orElse(0);
                }
                if (const TorpedoLauncher* pTorp = shipList.launchers().get(tech)) {
                    if (int cost = pTorp->torpedoCost().get(Cost::Money)) {
                        total = totalCost / cost;
                    }
                }
            }

            int level = 0;
            {
                UnitScoreList::Index_t index;
                int16_t value, turn;
                if (planetScores.lookup(ScoreId_ExpLevel, index) && pl.unitScores().get(index, value, turn)) {
                    level = value;
                }
            }
            const int ppt = config[HostConfiguration::PlanetaryTorpsPerTube](planetOwner) + config.getExperienceBonus(HostConfiguration::EModPlanetaryTorpsPerTube, level);
            total += ppt * n;
            if (ppt > 0) {
                next = n*(n+1)*4+1 - totalWant;
            } else {
                next = 0;
            }

            addLine(result, afl::string::Format(tx("%d torpedo%!1{es%}"), total), next, totalWant, totalMax, false);
        }
    }

    // Fighter effects
    if (canDoFighterPrediction(host, config)) {
        // Compute current status
        int shield;
        int damage;
        computeFighterEffects(shield, damage, 100 + totalWant, planetOwner, host, config);

        // Try to find better amounts
        int shieldWant = -1;
        int damageWant = -1;
        int n = totalWant;
        if (shield <= 1) {
            shieldWant = 0;
        }
        if (damage <= 1) {
            damageWant = 0;
        }
        while (shieldWant == -1 || damageWant == -1) {
            int s, d;
            ++n;
            computeFighterEffects(s, d, 100 + n, planetOwner, host, config);
            if (shieldWant == -1 && s != shield) {
                shieldWant = n - totalWant;
            }
            if (damageWant == -1 && d != damage) {
                damageWant = n - totalWant;
            }
        }

        // Show it
        addLine(result, afl::string::Format(tx("%d%% shield loss from enemy fighter"), shield), shieldWant, totalWant, totalMax, false);
        addLine(result, afl::string::Format(tx("%d%% damage from enemy fighter"), damage), damageWant, totalWant, totalMax, false);
    }

    // Update MAX_DEFENSE_EFFECT_LINES when adding stuff.
}


// Prepare unload information for a planet.
game::map::UnloadInfo
game::map::prepareUnloadInfo(const Universe& univ,
                             Id_t pid,
                             int viewpointPlayer,
                             const UnitScoreDefinitionList& scoreDefinitions,
                             const game::spec::ShipList& shipList,
                             const game::config::HostConfiguration& config)
{
    // ex computeUnload
    UnloadInfo result;

    const Planet* pl = univ.planets().get(pid);
    Point planetPosition;
    int planetOwner;
    if (pl != 0
        && pl->getPosition(planetPosition)
        && pl->getOwner(planetOwner))
    {
        PlayedShipType ty(const_cast<Universe&>(univ));
        for (Id_t sid = ty.findNextObjectAt(planetPosition, 0, false); sid != 0; sid = ty.findNextObjectAt(planetPosition, sid, false)) {
            const Ship* sh = ty.getObjectByIndex(sid);
            int shipOwner;
            if (sh != 0
                && sh->isPlayable(Object::Playable)
                && sh->getOwner(shipOwner)
                && shipOwner == viewpointPlayer
                && sh->isTransporterActive(Ship::UnloadTransporter))
            {
                // ship exits, is at this planet, played by current player, and unloading
                int clansInTransporter = sh->getTransporterCargo(Ship::UnloadTransporter, Element::Colonists).orElse(0);
                if (sh->getRealOwner().get(shipOwner) && shipOwner == planetOwner) {
                    // ship actually belongs to planet owner, so there will not be a fight
                    result.friendlyUnload += clansInTransporter;
                } else {
                    // Hostile unload
                    result.hostileUnload += clansInTransporter;
                    if (clansInTransporter >= 10 && sh->hasSpecialFunction(game::spec::HullFunction::ImperialAssault, scoreDefinitions, shipList, config)) {
                        result.hostileUnloadIsAssault = true;
                    }
                }
            }
        }
    }

    return result;
}

game::map::PlanetEffectors
game::map::preparePlanetEffectors(const Universe& univ,
                                  Id_t pid,
                                  const UnitScoreDefinitionList& shipScores,
                                  const game::spec::ShipList& shipList,
                                  const game::config::HostConfiguration& config)
{
    // ex WTaxationDialog::init (part), pdata.pas:NumHissers
    PlanetEffectors result;

    const Planet* pl = univ.planets().get(pid);
    Point planetPosition;
    int planetOwner;
    if (pl != 0
        && pl->getPosition(planetPosition)
        && pl->getOwner(planetOwner))
    {
        PlayedShipType ty(const_cast<Universe&>(univ));
        for (Id_t sid = ty.findNextObjectAt(planetPosition, 0, false); sid != 0; sid = ty.findNextObjectAt(planetPosition, sid, false)) {
            const Ship* sh = ty.getObjectByIndex(sid);
            int shipOwner;
            if (sh != 0
                && sh->isPlayable(Object::ReadOnly)
                && sh->getOwner(shipOwner))
            {
                int shipMission;
                if (config.getPlayerMissionNumber(shipOwner) == 2
                    && sh->getMission().get(shipMission)
                    && (shipMission == game::spec::Mission::msn_Special || shipMission == config[HostConfiguration::ExtMissionsStartAt]() + game::spec::Mission::pmsn_Special)
                    && sh->getNumBeams().orElse(0) > 0)
                {
                    // Hiss
                    result.add(PlanetEffectors::Hiss, 1);
                }
                if (sh->getWaypointDX().orElse(0) == 0 && sh->getWaypointDY().orElse(0) == 0) {
                    // Terraforming is after movement, so only process it if ships have no waypoint.
                    if (sh->hasSpecialFunction(game::spec::HullFunction::HeatsTo50, shipScores, shipList, config)) {
                        result.add(PlanetEffectors::HeatsTo50, 1);
                    }
                    if (sh->hasSpecialFunction(game::spec::HullFunction::CoolsTo50, shipScores, shipList, config)) {
                        result.add(PlanetEffectors::CoolsTo50, 1);
                    }
                    if (sh->hasSpecialFunction(game::spec::HullFunction::HeatsTo100, shipScores, shipList, config)) {
                        result.add(PlanetEffectors::HeatsTo100, 1);
                    }
                }
            }
        }
    }

    return result;
}


// Retrieve information about ground defense.
game::map::GroundDefenseInfo
game::map::packGroundDefenseInfo(const Planet& pl,
                                 const Root& root)
{
    // ex doGroundCombatPrediction (part)
    // ex envscan.pas:CGroundCombatWindow.DrawInterior (part)
    const HostConfiguration& config = root.hostConfiguration();
    const HostVersion& host = root.hostVersion();
    const PlayerList& players = root.playerList();

    GroundDefenseInfo result;

    int planetOwner;
    int defense;
    int32_t planetColonists;
    if (pl.getOwner(planetOwner)
        && planetOwner != 0
        && pl.getCargo(Element::Colonists).get(planetColonists)
        && pl.getNumBuildings(DefenseBuilding).get(defense))
    {
        // Compute my strength
        int32_t myStrength;
        if (host.isPHost()) {
            // PS := Colonists * (20 * GDEFENSE + dp) \ 20
            // i.e. for 0 dp planet = Colonists * GDEFENSE
            //         20 dp planet = Colonists * (GDEFENSE+1)
            myStrength = (planetColonists *
                          (20 * config[HostConfiguration::GroundDefenseFactor](planetOwner) + defense))
                / 20;
        } else {
            // PS := ERnd( Colonists * GDEFENSE * (Defense + 20) / 20 )
            // i.e. for 0 dp planet = Colonists * GDEFENSE
            //         20 dp planet = Colonists * GDEFENSE*2 }
            myStrength = util::divideAndRoundToEven(planetColonists * config[HostConfiguration::GroundDefenseFactor](planetOwner) * (defense + 20), 20, 0);
        }

        // Output
        result.defender = planetOwner;
        result.isPlayable = pl.isPlayable(Object::Playable);
        for (int i = 1; i <= MAX_PLAYERS; ++i) {
            if (players.getAllPlayers().contains(i)) {
                result.name.set(i, players.getPlayerName(i, Player::LongName));
                if (i == planetOwner) {
                    result.strength.set(i, planetColonists);
                } else {
                    // given N=number of attacking clans, their strength is
                    //    N * GATTACK
                    // which must be >= myStrength for the attack to kill our planet.
                    //    N * GATTACK >= myStrength
                    // -> N           >= myStrength / GATTACK
                    // the minimum N hence is ceil(myStrength / GATTACK)
                    int32_t theirRatio = config[HostConfiguration::GroundKillFactor](i);
                    if (theirRatio > 0) {
                        int32_t equivClans = (myStrength + (theirRatio-1)) / theirRatio;
                        result.strength.set(i, equivClans);
                    }
                }
            }
        }
    }

    return result;
}
