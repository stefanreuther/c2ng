/**
  *  \file game/interface/costsummarycontext.hpp
  *  \brief Class game::interface::CostSummaryContext
  */
#ifndef C2NG_GAME_INTERFACE_COSTSUMMARYCONTEXT_HPP
#define C2NG_GAME_INTERFACE_COSTSUMMARYCONTEXT_HPP

#include "afl/base/ptr.hpp"
#include "game/spec/costsummary.hpp"
#include "interpreter/simplecontext.hpp"

namespace game { namespace interface {

    /** Cost summary context.
        Publishes a game::spec::CostSummary object, mainly for exporting.

        Since a CostSummaryContext can stay around for considerable amount of time,
        it requires the CostSummary object to be allocated on the heap.
        (PCC2 used internal knowledge about the exporter behaviour to work on stack-allocated objects.)

        A CostSummary must be non-empty to be exported by CostSummaryContext.
        The constructor function, CostSummaryContext::create, will verify this. */
    class CostSummaryContext : public interpreter::SimpleContext, public interpreter::Context::ReadOnlyAccessor {
     public:
        /** Create a CostSummaryContext.
            @param cs CostSummary
            @return Newly-allocated CostSummaryContext if cs is non-null and non-empty */
        static CostSummaryContext* create(afl::base::Ptr<game::spec::CostSummary> cs);

        /** Destructor. */
        ~CostSummaryContext();

        // Context:
        virtual Context::PropertyAccessor* lookup(const afl::data::NameQuery& name, PropertyIndex_t& result);
        virtual afl::data::Value* get(PropertyIndex_t index);
        virtual bool next();
        virtual CostSummaryContext* clone() const;
        virtual afl::base::Deletable* getObject();
        virtual void enumProperties(interpreter::PropertyAcceptor& acceptor) const;

        // BaseValue:
        virtual String_t toString(bool readable) const;
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const;

     private:
        CostSummaryContext(afl::base::Ptr<game::spec::CostSummary> cs, size_t index)
            : m_costSummary(cs), m_index(index)
            { }

        afl::base::Ptr<game::spec::CostSummary> m_costSummary;
        size_t m_index;
    };

} }

#endif
