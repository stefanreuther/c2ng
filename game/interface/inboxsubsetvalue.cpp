/**
  *  \file game/interface/inboxsubsetvalue.cpp
  *  \brief Class game::interface::InboxSubsetValue
  */

#include "game/interface/inboxsubsetvalue.hpp"
#include "game/interface/inboxcontext.hpp"
#include "interpreter/arguments.hpp"

using afl::base::Ref;

namespace {
    class InboxSubsetContext : public interpreter::SimpleContext {
     public:
        InboxSubsetContext(size_t index,
                           const std::vector<size_t>& indexes,
                           game::Session& session,
                           const Ref<const game::Turn>& turn);
        ~InboxSubsetContext();

        virtual interpreter::Context::PropertyAccessor* lookup(const afl::data::NameQuery& name, PropertyIndex_t& result);
        virtual bool next();
        virtual InboxSubsetContext* clone() const;
        virtual afl::base::Deletable* getObject();
        virtual void enumProperties(interpreter::PropertyAcceptor& acceptor) const;
        virtual String_t toString(bool readable) const;
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const;

     private:
        size_t m_index;
        const std::vector<size_t> m_indexes;
        game::Session& m_session;
        const Ref<const game::Turn> m_turn;
        mutable std::auto_ptr<interpreter::Context> m_child;

        interpreter::Context& child() const;
    };
}

InboxSubsetContext::InboxSubsetContext(size_t index, const std::vector<size_t>& indexes,
                                       game::Session& session,
                                       const Ref<const game::Turn>& turn)
    : SimpleContext(), m_index(index), m_indexes(indexes), m_session(session), m_turn(turn), m_child()
{ }

InboxSubsetContext::~InboxSubsetContext()
{ }

interpreter::Context::PropertyAccessor*
InboxSubsetContext::lookup(const afl::data::NameQuery& name, PropertyIndex_t& result)
{
    // Because the child will return a pointer to itself, it cannot be a temporary.
    return child().lookup(name, result);
}

bool
InboxSubsetContext::next()
{
    if (m_index+1 < m_indexes.size()) {
        ++m_index;
        m_child.reset();
        return true;
    } else {
        return false;
    }
}

InboxSubsetContext*
InboxSubsetContext::clone() const
{
    return new InboxSubsetContext(m_index, m_indexes, m_session, m_turn);
}

afl::base::Deletable*
InboxSubsetContext::getObject()
{
    return child().getObject();
}

void
InboxSubsetContext::enumProperties(interpreter::PropertyAcceptor& acceptor) const
{
    child().enumProperties(acceptor);
}

String_t
InboxSubsetContext::toString(bool /*readable*/) const
{
    return "#<message>";
}

void
InboxSubsetContext::store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const
{
    rejectStore(out, aux, ctx);
}

interpreter::Context&
InboxSubsetContext::child() const
{
    if (!m_child.get()) {
        assert(m_index < m_indexes.size());
        m_child.reset(new game::interface::InboxContext(m_indexes[m_index], m_session, m_turn));
    }
    return *m_child;
}


/*
 *  InboxSubsetValue
 */

game::interface::InboxSubsetValue::InboxSubsetValue(const std::vector<size_t>& indexes,
                                                    game::Session& session,
                                                    const afl::base::Ref<const Turn>& turn)
    : IndexableValue(), m_indexes(indexes), m_session(session), m_turn(turn)
{
    // ex IntMappedMessageValue::IntMappedMessageValue
}

game::interface::InboxSubsetValue::~InboxSubsetValue()
{ }

afl::data::Value*
game::interface::InboxSubsetValue::get(interpreter::Arguments& args)
{
    // ex IntMappedMessageValue::get
    args.checkArgumentCount(1);

    size_t n;
    if (!interpreter::checkIndexArg(n, args.getNext(), 1, m_indexes.size())) {
        return 0;
    }

    // In theory, we could return a InboxContext here,
    // but for now, let's preserve the identity as coming from a Inbox subset.
    return new InboxSubsetContext(n, m_indexes, m_session, m_turn);
}

void
game::interface::InboxSubsetValue::set(interpreter::Arguments& args, const afl::data::Value* value)
{
    // ex IntMappedMessageValue::set
    rejectSet(args, value);
}

size_t
game::interface::InboxSubsetValue::getDimension(size_t which) const
{
    // ex IntMappedMessageValue::getDimension
    if (which == 0) {
        return 1;
    } else {
        return m_indexes.size() + 1;
    }
}

interpreter::Context*
game::interface::InboxSubsetValue::makeFirstContext()
{
    // ex IntMappedMessageValue::makeFirstContext
    if (m_indexes.empty()) {
        return 0;
    } else {
        return new InboxSubsetContext(0, m_indexes, m_session, m_turn);
    }
}

game::interface::InboxSubsetValue*
game::interface::InboxSubsetValue::clone() const
{
    // This copies the vector.
    // Since the vectors are short, this is acceptable and simpler than some reference counting scheme.
    return new InboxSubsetValue(m_indexes, m_session, m_turn);
}

String_t
game::interface::InboxSubsetValue::toString(bool /*readable*/) const
{
    return "#<array>";
}

void
game::interface::InboxSubsetValue::store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const
{
    rejectStore(out, aux, ctx);
}

game::interface::InboxSubsetValue*
game::interface::InboxSubsetValue::create(const std::vector<size_t>& indexes,
                                          game::Session& session,
                                          const afl::base::Ref<const Turn>& turn)
{
    // ex IntMappedMessageValue::create
    // We want "If Messages Then..." to be a valid test.
    // Therefore, instead of an empty array, return null.
    if (indexes.empty()) {
        return 0;
    } else {
        return new InboxSubsetValue(indexes, session, turn);
    }
}
