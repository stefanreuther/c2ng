/**
  *  \file game/interface/simunitresultcontext.hpp
  *  \brief Class game::interface::SimUnitResultContext
  */
#ifndef C2NG_GAME_INTERFACE_SIMUNITRESULTCONTEXT_HPP
#define C2NG_GAME_INTERFACE_SIMUNITRESULTCONTEXT_HPP

#include "game/interface/simobjectcontext.hpp"
#include "game/sim/resultlist.hpp"

namespace game { namespace interface {

    /** Context for publishing per-unit simulation results.

        As of August 2026, this context is only used for export, and only publishes read-only properties.
        It embeds and extends s SimObjectContext.

        To use this, usually use SimUnitResultContext::create().
        This will take a ResultList, and extract all information from it. */
    class SimUnitResultContext : public interpreter::SimpleContext, public interpreter::Context::ReadOnlyAccessor {
     public:
        /** Shortcut for per-unit results. */
        typedef std::vector<game::sim::ResultList::UnitInfo> Infos_t;

        /** Create SimUnitResultContext.
            @param result    ResultList. Per-unit results will be copied out.
            @param session   Simulator session
            @param root      Game root
            @param shipList  Ship list
            @param tx        Translator */
        static SimUnitResultContext* create(const game::sim::ResultList& result,
                                            const afl::base::Ref<game::sim::Session>& session,
                                            const afl::base::Ref<const Root>& root,
                                            const afl::base::Ref<const game::spec::ShipList>& shipList,
                                            afl::string::Translator& tx);

        /** Constructor.
            @param infos     Per-unit results to publish.
            @param session   Simulator session
            @param root      Root (for player names, config)
            @param shipList  Ship list (for component names)
            @param slot      Slot to export (see game::sim::Setup::getObject)
            @param tx        Translator (for default player names) */
        SimUnitResultContext(afl::base::Ptr<Infos_t> infos,
                             const afl::base::Ref<game::sim::Session>& session,
                             const afl::base::Ref<const Root>& root,
                             const afl::base::Ref<const game::spec::ShipList>& shipList,
                             game::sim::Setup::Slot_t slot,
                             afl::string::Translator& tx);

        /** Destructor. */
        ~SimUnitResultContext();

        // Context:
        virtual Context::PropertyAccessor* lookup(const afl::data::NameQuery& name, PropertyIndex_t& result);
        virtual bool next();
        virtual SimUnitResultContext* clone() const;
        virtual afl::base::Deletable* getObject();
        virtual void enumProperties(interpreter::PropertyAcceptor& acceptor) const;

        // BaseValue:
        virtual String_t toString(bool readable) const;
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const;

        // ReadOnlyAccessor:
        virtual afl::data::Value* get(PropertyIndex_t index);

     private:
        SimObjectContext m_delegate;
        afl::base::Ptr<Infos_t> m_infos;

        const game::sim::ResultList::UnitInfo* getUnitInfo();
        const game::sim::ResultList::UnitInfo::Item* getItem(game::sim::ResultList::UnitInfo::Type type);
    };

} }

#endif
