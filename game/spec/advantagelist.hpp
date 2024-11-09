/**
  *  \file game/spec/advantagelist.hpp
  *  \brief Class game::spec::AdvantageList
  */
#ifndef C2NG_GAME_SPEC_ADVANTAGELIST_HPP
#define C2NG_GAME_SPEC_ADVANTAGELIST_HPP

#include <vector>
#include "afl/base/types.hpp"
#include "afl/container/ptrvector.hpp"
#include "afl/string/string.hpp"
#include "game/playerset.hpp"

namespace game { namespace spec {

    /** List of race advantages.
        Advantages are a configuration mechanism in planets.nu that provides information about racial abilities.
        Advantages have Ids whose meaning is hard-coded.

        With this class, an advantage is referred to by a handle of opaque type Item*.
        Methods receiving a null handle will return default values or be ignored. */
    class AdvantageList {
     public:
        struct Item;

        /** Constructor.
            Makes an empty list. */
        AdvantageList();

        /** Destructor. */
        ~AdvantageList();

        /** Add an advantage.
            If an advantage with the given Id exists, returns it; otherwise, adds a new one.
            @param Handle to created advantage, never null */
        Item* add(int id);

        /** Find an advantage.
            @param id Advantage Id
            @return Handle to found advantage; null if not found */
        Item* find(int id) const;

        /** Get advantage, given index.
            @param index Index, [0,getNumAdvantages())
            @return Handle to advantage; null if index out of range */
        Item* getAdvantageByIndex(size_t index) const;

        /** Get number of advantages.
            @return Number */
        size_t getNumAdvantages() const;

        /** Set name of item.
            @param p      Handle; call is ignored if null
            @param name   New name */
        void setName(Item* p, const String_t& name);

        /** Set description of item.
            @param p             Handle; call is ignored if null
            @param description   New description */
        void setDescription(Item* p, const String_t& description);

        /** Add advantage for a player.
            @param p       Handle; call is ignored if null
            @param player  Player to add */
        void addPlayer(Item* p, int player);

        /** Get Id.
            @param p Handle
            @return Id; 0 if p is null */
        int getId(const Item* p) const;

        /** Get name.
            @param p Handle
            @return name; empty if p is null */
        String_t getName(const Item* p) const;

        /** Get description.
            @param p Handle
            @return description; empty if p is null */
        String_t getDescription(const Item* p) const;

        /** Get players.
            @param p Handle
            @return players; empty if p is null */
        PlayerSet_t getPlayers(const Item* p) const;

     private:
        afl::container::PtrVector<Item> m_data;
    };

} }

#endif
