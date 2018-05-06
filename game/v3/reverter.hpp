/**
  *  \file game/v3/reverter.hpp
  */
#ifndef C2NG_GAME_V3_REVERTER_HPP
#define C2NG_GAME_V3_REVERTER_HPP

#include "game/map/reverter.hpp"
#include "game/map/basedata.hpp"
#include "game/map/objectvector.hpp"
#include "game/map/planetdata.hpp"
#include "game/map/shipdata.hpp"
#include "game/map/universe.hpp"
#include "game/session.hpp"

namespace game { namespace v3 {

    class UndoInformation;

    class Reverter : public game::map::Reverter {
     public:
        explicit Reverter(game::map::Universe& univ, Session& session);

        // game::Reverter:
        virtual afl::base::Optional<int> getMinBuildings(int planetId, PlanetaryBuilding building) const;
        virtual int getSuppliesAllowedToBuy(int planetId) const;
        virtual afl::base::Optional<int> getMinTechLevel(int planetId, TechLevel techLevel) const;
        virtual afl::base::Optional<int> getMinBaseStorage(int planetId, TechLevel area, int slot) const;
        virtual int getNumTorpedoesAllowedToSell(int planetId, int slot) const;
        virtual int getNumFightersAllowedToSell(int planetId) const;
        virtual afl::base::Optional<String_t> getPreviousShipFriendlyCode(Id_t shipId) const;
        virtual afl::base::Optional<String_t> getPreviousPlanetFriendlyCode(Id_t planetId) const;

        // v3::Reverter:
        void addShipData(int id, const game::map::ShipData& data);
        void addPlanetData(int id, const game::map::PlanetData& data);
        void addBaseData(int id, const game::map::BaseData& data);

        const game::map::ShipData* getShipData(int id) const;
        const game::map::PlanetData* getPlanetData(int id) const;
        const game::map::BaseData* getBaseData(int id) const;

     private:
        game::map::Universe& m_universe;
        Session& m_session;

        game::map::ObjectVector<game::map::ShipData> m_oldShipData;
        game::map::ObjectVector<game::map::PlanetData> m_oldPlanetData;
        game::map::ObjectVector<game::map::BaseData> m_oldBaseData;

        bool prepareUndoInformation(UndoInformation& u, int planetId) const;
    };

} }

#endif
