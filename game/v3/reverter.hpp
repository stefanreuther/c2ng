/**
  *  \file game/v3/reverter.hpp
  *  \brief Class game::v3::Reverter
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
#include "game/turn.hpp"

namespace game { namespace v3 {

    class UndoInformation;

    /** Implementation of Reverter for v3 file formats. */
    class Reverter : public game::map::Reverter {
     public:
        /** Constructor.
            \param turn    Turn (containing the Universe this Reverter is associated with)
            \param session Session (for ShipList; for config from Root) */
        explicit Reverter(Turn& turn, Session& session);

        // game::map::Reverter:
        virtual afl::base::Optional<int> getMinBuildings(int planetId, PlanetaryBuilding building) const;
        virtual int getSuppliesAllowedToBuy(int planetId) const;
        virtual afl::base::Optional<int> getMinTechLevel(int planetId, TechLevel techLevel) const;
        virtual afl::base::Optional<int> getMinBaseStorage(int planetId, TechLevel area, int slot) const;
        virtual int getNumTorpedoesAllowedToSell(int planetId, int slot) const;
        virtual int getNumFightersAllowedToSell(int planetId) const;
        virtual afl::base::Optional<String_t> getPreviousShipFriendlyCode(Id_t shipId) const;
        virtual afl::base::Optional<String_t> getPreviousPlanetFriendlyCode(Id_t planetId) const;
        virtual bool getPreviousShipMission(int shipId, int& m, int& i, int& t) const;
        virtual bool getPreviousShipBuildOrder(int planetId, ShipBuildOrder& result) const;
        virtual game::map::LocationReverter* createLocationReverter(game::map::Point pt) const;

        /** Add ship undo data.
            \param id Ship Id
            \param data Old ship data (shipX.dis) */
        void addShipData(int id, const game::map::ShipData& data);

        /** Add planet undo data.
            \param id Planet Id
            \param data Old planet data (pdataX.dis) */
        void addPlanetData(int id, const game::map::PlanetData& data);

        /** Add starbase undo data.
            \param id Starbase Id
            \param data Old starbase data (bdata.dis) */
        void addBaseData(int id, const game::map::BaseData& data);

        /** Get ship undo data.
            \param id Ship Id
            \return data; null if none known */
        const game::map::ShipData* getShipData(int id) const;

        /** Get planet undo data.
            \param id Planet Id
            \return data; null if none known */
        const game::map::PlanetData* getPlanetData(int id) const;

        /** Get starbase undo data.
            \param id Starbase Id
            \return data; null if none known */
        const game::map::BaseData* getBaseData(int id) const;

     private:
        Turn& m_turn;
        Session& m_session;

        game::map::ObjectVector<game::map::ShipData> m_oldShipData;
        game::map::ObjectVector<game::map::PlanetData> m_oldPlanetData;
        game::map::ObjectVector<game::map::BaseData> m_oldBaseData;

        game::map::Universe& universe() const;
        bool prepareUndoInformation(UndoInformation& u, int planetId) const;

        class MyLocationReverter;
        friend class MyLocationReverter;
    };

} }

#endif
