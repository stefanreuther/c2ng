/**
  *  \file game/actions/changeshipfriendlycode.hpp
  *  \brief Class game::actions::ChangeShipFriendlyCode
  */
#ifndef C2NG_GAME_ACTIONS_CHANGESHIPFRIENDLYCODE_HPP
#define C2NG_GAME_ACTIONS_CHANGESHIPFRIENDLYCODE_HPP

#include "game/map/universe.hpp"
#include "game/spec/friendlycodelist.hpp"
#include "util/randomnumbergenerator.hpp"

namespace game { namespace actions {

    /** Change ship friendly codes.
        Used for programmatic friendly code changes (i.e. set a fleet's fcode to "HYP").

        Usage:
        - construct
        - call addShip(), addFleet() as required
        - call setFriendlyCode(), unsetFriendlyCode(), undo() as required

        Changes are immediate, there's no need to commit. */
    class ChangeShipFriendlyCode {
     public:
        /** Constructor.
            \param univ Universe */
        explicit ChangeShipFriendlyCode(game::map::Universe& univ);

        /** Destructor. */
        ~ChangeShipFriendlyCode();

        /** Add single ship.
            \param shipId  Ship Id
            \param fcl     Friendly Code List (to generate fallback friendly codes)
            \param rng     Random Number Generator (to generate fallback friendly codes) */
        void addShip(Id_t shipId, game::spec::FriendlyCodeList& fcl, util::RandomNumberGenerator& rng);

        /** Add fleet.
            \param fleetId Fleet Id (Id of fleet leader or lone ship)
            \param fcl     Friendly Code List (to generate fallback friendly codes)
            \param rng     Random Number Generator (to generate fallback friendly codes) */
        void addFleet(Id_t fleetId, game::spec::FriendlyCodeList& fcl, util::RandomNumberGenerator& rng);

        /** Set friendly code.
            Sets all ships' friendly codes to the given value.
            \param fc Friendly code */
        void setFriendlyCode(String_t fc);

        /** Unset friendly code.
            Sets all ships' friendly codes to avoid the given value.
            The friendly code is reverted to the original friendly code,
            friendly code at beginning of turn, or random friendly code.
            \param avoidFC friendly code to avoid */
        void unsetFriendlyCode(String_t avoidFC);

        /** Undo.
            Set all friendly codes back to the original values. */
        void undo();

     private:
        game::map::Universe& m_universe;

        struct Info {
            /** Ship Id. */
            Id_t shipId;

            /** Old friendly code. Code that was active before the ChangeShipFriendlyCode object was constructed. */
            String_t oldFriendlyCode;

            /** Random friendly code.
                Precomputed so that unsetFriendlyCode always has the same result, no matter how often it's called. */
            String_t randomFriendlyCode;

            Info(Id_t id, String_t oldFC, String_t randomFC)
                : shipId(id), oldFriendlyCode(oldFC), randomFriendlyCode(randomFC)
                { }
        };
        std::vector<Info> m_info;
    };

} }

#endif
