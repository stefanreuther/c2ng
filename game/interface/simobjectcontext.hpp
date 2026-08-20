/**
  *  \file game/interface/simobjectcontext.hpp
  *  \brief Class game::interface::SimObjectContext
  */
#ifndef C2NG_GAME_INTERFACE_SIMOBJECTCONTEXT_HPP
#define C2NG_GAME_INTERFACE_SIMOBJECTCONTEXT_HPP

#include "game/root.hpp"
#include "game/sim/object.hpp"
#include "game/sim/planet.hpp"
#include "game/sim/session.hpp"
#include "game/sim/setup.hpp"
#include "game/sim/ship.hpp"
#include "game/spec/shiplist.hpp"
#include "interpreter/simplecontext.hpp"

namespace game { namespace interface {

    /** Context for publishing simulation objects.

        As of August 2026, this context is only used for export, and only publishes read-only properties.
        However, some properties for a future script language integration have been implemented,
        and making properties assignable will make sense. */
    class SimObjectContext : public interpreter::SimpleContext, public interpreter::Context::PropertyAccessor {
     public:
        /** Constructor.
            @param session   Simulator session
            @param root      Root (for player names, config)
            @param shipList  Ship list (for component names)
            @param slot      Slot to export (see game::sim::Setup::getObject)
            @param tx        Translator (for default player names) */
        SimObjectContext(const afl::base::Ref<game::sim::Session>& session,
                         const afl::base::Ref<const Root>& root,
                         const afl::base::Ref<const game::spec::ShipList>& shipList,
                         game::sim::Setup::Slot_t slot,
                         afl::string::Translator& tx);

        /** Destructor. */
        ~SimObjectContext();

        // Context:
        virtual Context::PropertyAccessor* lookup(const afl::data::NameQuery& name, PropertyIndex_t& result);
        virtual bool next();
        virtual SimObjectContext* clone() const;
        virtual game::sim::Object* getObject();
        virtual void enumProperties(interpreter::PropertyAcceptor& acceptor) const;

        // BaseValue:
        virtual String_t toString(bool readable) const;
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const;

        // PropertyAccessor:
        virtual void set(PropertyIndex_t index, const afl::data::Value* value);
        virtual afl::data::Value* get(PropertyIndex_t index);

        /** Get object as planet.
            @return object; null if not looking at a planet */
        game::sim::Planet* getPlanet();

        /** Get object as ship.
            @return object; null if not looking at a ship */
        game::sim::Ship* getShip();

        /** Get slot number.
            @return slot number, 0-based */
        size_t getSlotNumber() const;

     private:
        afl::base::Ref<game::sim::Session> m_session;
        afl::base::Ref<const Root> m_root;
        afl::base::Ref<const game::spec::ShipList> m_shipList;
        game::sim::Setup::Slot_t m_slot;
        afl::string::Translator& m_translator;

        class HasFunctionValue;
        class AmmoStorageValue;
    };

} }

inline size_t
game::interface::SimObjectContext::getSlotNumber() const
{
    return m_slot;
}

#endif
