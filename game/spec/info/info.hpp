/**
  *  \file game/spec/info/info.hpp
  *  \brief Specification formatting functions
  */
#ifndef C2NG_GAME_SPEC_INFO_INFO_HPP
#define C2NG_GAME_SPEC_INFO_INFO_HPP

#include "game/root.hpp"
#include "game/shipquery.hpp"
#include "game/spec/beam.hpp"
#include "game/spec/engine.hpp"
#include "game/spec/fighter.hpp"
#include "game/spec/info/types.hpp"
#include "game/spec/shiplist.hpp"
#include "game/spec/torpedolauncher.hpp"

namespace game { namespace spec { namespace info {

    class PictureNamer;

    /** Get flags for a HullFunction instance.
        \param [in]  func             Function to check
        \param [in]  basicFunctions   BasicHullFunctionList (for function damage levels)
        \param [in]  query            ShipQuery describing the ship we're asking the question for
        \param [in]  config           Host configuration (for damage levels)
        \return flags */
    AbilityFlags_t getAbilityFlags(const HullFunction& func, const BasicHullFunctionList& basicFunctions, const ShipQuery& query, const game::config::HostConfiguration& config);

    /** Describe a hull.
        Output is intended to be human-readable.
        \param [out] content          Result produced here
        \param [in]  id               Hull Id. If out of range, no content is produced.
        \param [in]  shipList         Ship list (used to locate the hull, hull functions)
        \param [in]  withCost         Include cost in formatted output
        \param [in]  picNamer         Picture namer
        \param [in]  root             Root (used for configuration, host version)
        \param [in]  viewpointPlayer  Viewpoint player (used for configuration)
        \param [in]  tx               Translator */
    void describeHull(PageContent& content, Id_t id, const ShipList& shipList, bool withCost, const PictureNamer& picNamer, const Root& root, int viewpointPlayer, afl::string::Translator& tx);

    /** Describe a list of hull functions.
        \param [out] out              Result produced here
        \param [in]  hfList           Hull functions to describe
        \param [in]  pQuery           Ship query (optional, to describe ability flags)
        \param [in]  shipList         Ship list (for hull functions)
        \param [in]  picNamer         Picture namer
        \param [in]  root             Root (used for configuration, host version)
        \param [in]  tx               Translator */
    void describeHullFunctions(Abilities_t& out, const HullFunctionList& hfList, const ShipQuery* pQuery, const ShipList& shipList, const PictureNamer& picNamer, const Root& root, afl::string::Translator& tx);

    /** Describe a list of hull functions, detailed version.
        \param [out] out              Result produced here
        \param [in]  hfList           Hull functions to describe
        \param [in]  pQuery           Ship query (optional, to describe ability flags)
        \param [in]  shipList         Ship list (for hull functions)
        \param [in]  picNamer         Picture namer
        \param [in]  useNormalPictures If true, use original pictures (do not use flags)
        \param [in]  root             Root (used for configuration, host version)
        \param [in]  tx               Translator */
    void describeHullFunctionDetails(AbilityDetails_t& out, const HullFunctionList& hfList, const ShipQuery* pQuery, const ShipList& shipList, const PictureNamer& picNamer, bool useNormalPictures, const Root& root, afl::string::Translator& tx);

    /** Describe an engine.
        Output is intended to be human-readable.
        \param [out] content          Result produced here
        \param [in]  id               Engine Id. If out of range, no content is produced.
        \param [in]  shipList         Ship list (used to locate the engine)
        \param [in]  withCost         Include cost in formatted output
        \param [in]  picNamer         Picture namer
        \param [in]  root             Root (used for configuration, host version)
        \param [in]  viewpointPlayer  Viewpoint player (used for configuration)
        \param [in]  tx               Translator */
    void describeEngine(PageContent& content, Id_t id, const ShipList& shipList, bool withCost, const PictureNamer& picNamer, const Root& root, int viewpointPlayer, afl::string::Translator& tx);

    /** Describe a beam.
        Output is intended to be human-readable.
        \param [out] content          Result produced here
        \param [in]  id               Beam Id. If out of range, no content is produced.
        \param [in]  shipList         Ship list (used to locate the beam)
        \param [in]  withCost         Include cost in formatted output
        \param [in]  picNamer         Picture namer
        \param [in]  root             Root (used for configuration, host version)
        \param [in]  viewpointPlayer  Viewpoint player (used for configuration)
        \param [in]  tx               Translator */
    void describeBeam(PageContent& content, Id_t id, const ShipList& shipList, bool withCost, const PictureNamer& picNamer, const Root& root, int viewpointPlayer, afl::string::Translator& tx);

    /** Describe a torpedo launcher.
        Output is intended to be human-readable.
        \param [out] content          Result produced here
        \param [in]  id               Torpedo Id. If out of range, no content is produced.
        \param [in]  shipList         Ship list (used to locate the launcher)
        \param [in]  withCost         Include cost in formatted output
        \param [in]  picNamer         Picture namer
        \param [in]  root             Root (used for configuration, host version)
        \param [in]  viewpointPlayer  Viewpoint player (used for configuration)
        \param [in]  tx               Translator */
    void describeTorpedo(PageContent& content, Id_t id, const ShipList& shipList, bool withCost, const PictureNamer& picNamer, const Root& root, int viewpointPlayer, afl::string::Translator& tx);

    /** Describe a fighter launcher.
        Output is intended to be human-readable.
        \param [out] content          Result produced here
        \param [in]  player           Player number.
        \param [in]  shipList         Ship list (required for naming)
        \param [in]  withCost         Include cost in formatted output
        \param [in]  picNamer         Picture namer
        \param [in]  root             Root (used for configuration, host version)
        \param [in]  tx               Translator */
    void describeFighter(PageContent& content, int player, const ShipList& shipList, bool withCost, const PictureNamer& picNamer, const Root& root, afl::string::Translator& tx);

    /** Describe weapon effects against a ship.
        \param [out] result           Result produced here
        \param [in]  query            ShipQuery describing the ship targeted by weapons
        \param [in]  shipList         Ship list
        \param [in]  root             Root (used for configuration, host version)
        \param [in]  tx               Translator */
    void describeWeaponEffects(WeaponEffects& result, const ShipQuery& query, const ShipList& shipList, const Root& root, afl::string::Translator& tx);


    /** Get hull attribute.
        The attribute is identified by a FilterAttribute value; if an invalid/unknown value is requested, the function returns Nothing.
        \param h Hull
        \param att Attribute to retrieve
        \return Attribute if att corresponds to a valid attribute */
    OptionalInt_t getHullAttribute(const Hull& h, FilterAttribute att);

    /** Get engine attribute.
        The attribute is identified by a FilterAttribute value; if an invalid/unknown value is requested, the function returns Nothing.
        \param engine Engine
        \param att Attribute to retrieve
        \return Attribute if att corresponds to a valid attribute */
    OptionalInt_t getEngineAttribute(const Engine& engine, FilterAttribute att);

    /** Get beam attribute.
        The attribute is identified by a FilterAttribute value; if an invalid/unknown value is requested, the function returns Nothing.
        \param beam Beam
        \param att Attribute to retrieve
        \param root Root (used for configuration, host version)
        \param viewpointPlayer Viewpoint player (used for configuration)
        \return Attribute if att corresponds to a valid attribute */
    OptionalInt_t getBeamAttribute(const Beam& beam, FilterAttribute att, const Root& root, int viewpointPlayer);

    /** Get torpedo launcher attribute.
        The attribute is identified by a FilterAttribute value; if an invalid/unknown value is requested, the function returns Nothing.
        \param torp Torpedo launcher
        \param att Attribute to retrieve
        \param root Root (used for configuration, host version)
        \param viewpointPlayer Viewpoint player (used for configuration)
        \return Attribute if att corresponds to a valid attribute */
    OptionalInt_t getTorpedoAttribute(const TorpedoLauncher& torp, FilterAttribute att, const Root& root, int viewpointPlayer);

    /** Get fighter attribute.
        The attribute is identified by a FilterAttribute value; if an invalid/unknown value is requested, the function returns Nothing.
        \param ftr Fighter
        \param att Attribute to retrieve
        \param root Root (used for configuration, host version)
        \return Attribute if att corresponds to a valid attribute */
    OptionalInt_t getFighterAttribute(const Fighter& ftr, FilterAttribute att, const Root& root);

} } }

#endif
