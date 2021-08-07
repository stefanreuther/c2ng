/**
  *  \file game/proxy/vcrdatabaseproxy.hpp
  *  \brief Class game::proxy::VcrDatabaseProxy
  */
#ifndef C2NG_GAME_PROXY_VCRDATABASEPROXY_HPP
#define C2NG_GAME_PROXY_VCRDATABASEPROXY_HPP

#include <memory>
#include "afl/base/optional.hpp"
#include "afl/base/signal.hpp"
#include "afl/string/translator.hpp"
#include "afl/sys/loglistener.hpp"
#include "game/playerarray.hpp"
#include "game/proxy/vcrdatabaseadaptor.hpp"
#include "game/reference.hpp"
#include "game/root.hpp"
#include "game/spec/info/picturenamer.hpp"
#include "game/spec/shiplist.hpp"
#include "game/teamsettings.hpp"
#include "game/vcr/database.hpp"
#include "game/vcr/info.hpp"
#include "game/vcr/objectinfo.hpp"
#include "util/requestdispatcher.hpp"
#include "util/requestreceiver.hpp"
#include "util/requestsender.hpp"
#include "util/stringlist.hpp"
#include "game/shipquery.hpp"

namespace game { namespace proxy {

    class WaitIndicator;

    /** Bidirectional proxy for VCR database access.
        Proxies access to a game::vcr::Database.

        The game::vcr::Database object is selected using a VcrDatabaseAdaptor instance provided by the caller.
        That adaptor also provides a few surrounding objects, as well as the ability to store a current position.

        For now, this proxy implements a little more dynamic behaviour than others.
        When lag starts to build up, it internally produces dummy data (information containing just a heading) to remain responsive,
        and does not forward outdated information.

        Synchronous, bidirectional:
        - query position and count
        - query TeamSettings, player names (offered here so you don't need a Session sender if you have a VcrDatabaseAdaptor sender)

        Asynchronous, bidirectional:
        - request one battle's information and details

        To request ship details, in this order:
        - call getStatus(); this will report the valid range of battles
        - call setCurrentBattle() to select a battle; this will respond with sig_update, indicating the number of sides.
        - call setSide() to select a select a side; this will respond with sig_sideUpdate, offering a number of possible hull types.
        - call setHullType() to select a hull; this will respond with sig_hullUpdate. */
    class VcrDatabaseProxy {
     public:
        /** Kind of battles. */
        enum Kind {
            UnknownCombat,
            ClassicCombat,
            FlakCombat
        };

        /** Status. */
        struct Status {
            size_t numBattles;                           ///< Number of battles in database. \see game::vcr::Database::getNumBattles().
            size_t currentBattle;                        ///< Current battle. \see VcrDatabaseAdaptor::getCurrentBattle().
            Kind kind;                                   ///< Kind of battles.
            Status()
                : numBattles(0), currentBattle(0), kind(UnknownCombat)
                { }
        };

        /** Detail information about a side. */
        struct SideInfo {
            String_t name;                               ///< Unit name.
            String_t subtitle;                           ///< Subtitle (Id, owner, type).
            bool isPlanet;                               ///< true if this is a planet.
            Reference reference;                         ///< Reference to game object, if any.
            util::StringList typeChoices;                ///< Possible hulls with names. Contains a single entry with Id 0 if hull cannot be determined.

            SideInfo()
                : name(), subtitle(), isPlanet(false), reference(), typeChoices()
                { }
        };

        /** Detail information about a unit with a chosen hull. */
        struct HullInfo {
            String_t imageName;                                     ///< Name of image.
            afl::base::Optional<game::vcr::PlanetInfo> planetInfo;  ///< Planet information, if this is a planet.
            afl::base::Optional<game::vcr::ShipInfo> shipInfo;      ///< Ship information, if this is a ship.
            afl::base::Optional<ShipQuery> shipQuery;               ///< ShipQuery, if this is a ship.
        };

        /** Result for addToSimulation. */
        enum AddResult {
            Success,               ///< Successfully added.
            Error,                 ///< Internal error, e.g.\ no simulation or index out of range.
            NotPlayable,           ///< Fight is not playable.
            NotParseable,          ///< Data cannot be interpreted (e.g.\ planet with 200 mass but no fighters).
            UnitDied               ///< Unit died.
        };


        /** Constructor.
            \param sender     Sender
            \param receiver   RequestDispatcher to receive updates in this thread
            \param tx         Translator
            \param picNamer   Picture Namer. Can be null, then you won't get picture names. */
        VcrDatabaseProxy(util::RequestSender<VcrDatabaseAdaptor> sender, util::RequestDispatcher& recv, afl::string::Translator& tx, std::auto_ptr<game::spec::info::PictureNamer> picNamer);

        /** Destructor. */
        ~VcrDatabaseProxy();

        /** Get current status.
            Retrieves number of battles and last position.
            \param [in]  ind    WaitIndicator for UI synchronisation
            \param [out] status Status */
        void getStatus(WaitIndicator& ind, Status& status);

        /** Get TeamSettings.
            If the VcrDatabaseAdaptor knows about a TeamSettings object, returns a copy of that.
            \param [in]  ind    WaitIndicator for UI synchronisation
            \param [out] teams  TeamSettings */
        void getTeamSettings(WaitIndicator& ind, TeamSettings& teams);

        /** Get player names.
            \param [in]  ind    WaitIndicator for UI synchronisation
            \param [in]  which  Name to query
            \return Array of player names */
        PlayerArray<String_t> getPlayerNames(WaitIndicator& ind, Player::Name which);

        /** Set current battle.
            Replies with one or more sig_update calls.
            Valid indexes are [0, Status::numBattles).
            \param index New index */
        void setCurrentBattle(size_t index);

        /** Set side.
            Must be called after a battle has been chosen using setCurrentBattle().
            Replies with sig_sideUpdate.
            Valid sides are indexes into game::vcr::BattleInfo::units from the last sig_update.
            \param side     Side Side
            \param setHull  true to automatically select the first possible hull as if setHullType were called, and generate a sig_hullUpdate. */
        void setSide(size_t side, bool setHull);

        /** Set hull type.
            Must be called after a side has been chosen with setSide, to pick a hull type for comparison.
            Replies with sig_hullUpdate.
            Valid hull types are taken from the last sig_sideUpdate's SideInfo::typeChoices.
            \param hullType Hull type */
        void setHullType(int hullType);

        /** Add to battle simulation.
            \param ind      WaitIndicator
            \param hullType Hull type; see setHullType
            \param after    true: after fight; false: before fight
            \return status */
        AddResult addToSimulation(WaitIndicator& ind, int hullType, bool after);

        /** Signal: data update.
            \param index Battle index
            \param data  Data */
        afl::base::Signal<void(size_t, const game::vcr::BattleInfo&)> sig_update;

        /** Signal: side info update.
            \param info Information */
        afl::base::Signal<void(const SideInfo&)> sig_sideUpdate;

        /** Signal: hull info update.
            \param info Information */
        afl::base::Signal<void(const HullInfo&)> sig_hullUpdate;

     private:
        class Trampoline;
        class TrampolineFromAdaptor;

        util::RequestReceiver<VcrDatabaseProxy> m_reply;
        util::RequestSender<Trampoline> m_request;
        afl::string::Translator& m_translator;

        bool m_isActiveQuery;
        size_t m_currentIndex;
        size_t m_currentSide;

        afl::base::Optional<size_t> m_numBattles;

        void updateCurrentBattle(size_t index, size_t numBattles, game::vcr::BattleInfo data);
        void updateSideInfo(SideInfo info);
        void updateHullInfo(HullInfo info);
        void updateTemporaryState();
        void renderHeading(game::vcr::BattleInfo& data, size_t numBattles);
    };

} }

#endif
