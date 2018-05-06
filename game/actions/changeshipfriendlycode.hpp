/**
  *  \file game/actions/changeshipfriendlycode.hpp
  */
#ifndef C2NG_GAME_ACTIONS_CHANGESHIPFRIENDLYCODE_HPP
#define C2NG_GAME_ACTIONS_CHANGESHIPFRIENDLYCODE_HPP

#include "game/map/universe.hpp"
#include "game/spec/friendlycodelist.hpp"
#include "util/randomnumbergenerator.hpp"

namespace game { namespace actions {

    class ChangeShipFriendlyCode {
     public:
        explicit ChangeShipFriendlyCode(game::map::Universe& univ);
        ~ChangeShipFriendlyCode();

        void addShip(Id_t shipId, game::spec::FriendlyCodeList& fcl, util::RandomNumberGenerator& rng);
        void addFleet(Id_t fleetId, game::spec::FriendlyCodeList& fcl, util::RandomNumberGenerator& rng);

        void setFriendlyCode(String_t fc);
        void unsetFriendlyCode(String_t avoidFC);
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
