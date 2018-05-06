/**
  *  \file game/vcr/overview.hpp
  */
#ifndef C2NG_GAME_VCR_OVERVIEW_HPP
#define C2NG_GAME_VCR_OVERVIEW_HPP

#include "game/vcr/battle.hpp"
#include "game/vcr/database.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/spec/shiplist.hpp"

namespace game { namespace vcr {

    class Overview {
     public:
        struct Item {
            bool planet;          ///< true iff this item describes a planet.
            Id_t id;              ///< Id of this object.
            Id_t groupId;         ///< Group Id.
            size_t sequence;         ///< Uniquifier to make sort stable.
            String_t name;
            Item(bool planet, Id_t id, Id_t groupId, size_t sequence, String_t name)
                : planet(planet), id(id), groupId(groupId), sequence(sequence), name(name)
                { }
        };

        Overview();

        void clear();
        void addBattle(Battle& b,
                       const game::config::HostConfiguration& config,
                       const game::spec::ShipList& shipList);
        void addDatabase(Database& db,
                         const game::config::HostConfiguration& config,
                         const game::spec::ShipList& shipList);
        void finish();

     private:
        std::vector<Item> lines;
        Id_t m_groupCounter;

        std::vector<Item>::const_iterator findObject(const Object& obj) const;
        void renameGroup(int from, int to);
    };

} }

#endif
