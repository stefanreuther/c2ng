/**
  *  \file game/interface/simclassresultcontext.hpp
  *  \brief Class game::interface::SimClassResultContext
  */
#ifndef C2NG_GAME_INTERFACE_SIMCLASSRESULTCONTEXT_HPP
#define C2NG_GAME_INTERFACE_SIMCLASSRESULTCONTEXT_HPP

#include "game/sim/resultlist.hpp"
#include "interpreter/simplecontext.hpp"
#include "util/numberformatter.hpp"

namespace game { namespace interface {

    /** Context for publishing simulation class results.

        As of August 2026, this context is only used for export, and only publishes read-only properties.

        To use this, usually use SimClassResultContext::create().
        This will take a ResultList, and extract all information from it. */
    class SimClassResultContext : public interpreter::SimpleContext, public interpreter::Context::ReadOnlyAccessor {
     public:
        /** Shortcut for results. */
        typedef std::vector<game::sim::ResultList::ClassInfo> Infos_t;

        /** Maximum number of players.
            For the benefit of the export function, we publish player results as individual scalars.
            This is the number of scalars we publish. */
        static const int MAX_PLAYERS = 12;

        /** Create SimClassResultContext.
            @param result    ResultList. Class results will be copied out.
            @param fmt       Number formatter */
        static SimClassResultContext* create(const game::sim::ResultList& result, const util::NumberFormatter& fmt);

        /** Constructor.
            @param infos Results to publish.
            @param cumulativeWeight Cumulative weight (to compute percentages)
            @param slot  Slot (index) */
        SimClassResultContext(afl::base::Ptr<Infos_t> infos, int32_t cumulativeWeight, size_t slot);

        /** Destructor. */
        ~SimClassResultContext();

        // Context:
        virtual Context::PropertyAccessor* lookup(const afl::data::NameQuery& name, PropertyIndex_t& result);
        virtual bool next();
        virtual SimClassResultContext* clone() const;
        virtual afl::base::Deletable* getObject();
        virtual void enumProperties(interpreter::PropertyAcceptor& acceptor) const;

        // BaseValue:
        virtual String_t toString(bool readable) const;
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const;

        // ReadOnlyAccessor:
        virtual afl::data::Value* get(PropertyIndex_t index);

     private:
        afl::base::Ptr<Infos_t> m_infos;
        int32_t m_cumulativeWeight;
        size_t m_slot;

        const game::sim::ResultList::ClassInfo* getClassInfo();
    };

} }

#endif
