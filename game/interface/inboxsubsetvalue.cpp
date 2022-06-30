/**
  *  \file game/interface/inboxsubsetvalue.cpp
  */

#include "game/interface/inboxsubsetvalue.hpp"
#include "game/interface/inboxcontext.hpp"
#include "interpreter/arguments.hpp"

namespace {
    class InboxSubsetContext : public interpreter::Context {
     public:
        InboxSubsetContext(size_t index,
                           const std::vector<size_t>& indexes,
                           afl::string::Translator& tx,
                           afl::base::Ref<const game::Root> root,
                           afl::base::Ref<const game::Game> game);
        ~InboxSubsetContext();

        virtual interpreter::Context::PropertyAccessor* lookup(const afl::data::NameQuery& name, PropertyIndex_t& result);
        virtual bool next();
        virtual InboxSubsetContext* clone() const;
        virtual game::map::Object* getObject();
        virtual void enumProperties(interpreter::PropertyAcceptor& acceptor);
        virtual String_t toString(bool readable) const;
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const;

     private:
        size_t m_index;
        std::vector<size_t> m_indexes;
        afl::string::Translator& m_translator;
        afl::base::Ref<const game::Root> m_root;
        afl::base::Ref<const game::Game> m_game;
        std::auto_ptr<interpreter::Context> m_child;

        interpreter::Context& child();
    };
}

InboxSubsetContext::InboxSubsetContext(size_t index, const std::vector<size_t>& indexes, afl::string::Translator& tx, afl::base::Ref<const game::Root> root, afl::base::Ref<const game::Game> game)
    : Context(), m_index(index), m_indexes(indexes), m_translator(tx), m_root(root), m_game(game), m_child()
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
    return new InboxSubsetContext(m_index, m_indexes, m_translator, m_root, m_game);
}

game::map::Object*
InboxSubsetContext::getObject()
{
    return child().getObject();
}

void
InboxSubsetContext::enumProperties(interpreter::PropertyAcceptor& acceptor)
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
InboxSubsetContext::child()
{
    if (!m_child.get()) {
        assert(m_index < m_indexes.size());
        m_child.reset(new game::interface::InboxContext(m_indexes[m_index], m_translator, m_root, m_game));
    }
    return *m_child;
}


/*
 *  InboxSubsetValue
 */

game::interface::InboxSubsetValue::InboxSubsetValue(const std::vector<size_t>& indexes, afl::string::Translator& tx, afl::base::Ref<const Root> root, afl::base::Ref<const Game> game)
    : IndexableValue(), m_indexes(indexes), m_translator(tx), m_root(root), m_game(game)
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

    int32_t n;
    if (!interpreter::checkIntegerArg(n, args.getNext(), 1, static_cast<int32_t>(m_indexes.size()))) {
        return 0;
    }

    // In theory, we could return a InboxContext here,
    // but for now, let's preserve the identity as coming from a Inbox subset.
    return new InboxSubsetContext(n-1, m_indexes, m_translator, m_root, m_game);
}

void
game::interface::InboxSubsetValue::set(interpreter::Arguments& args, afl::data::Value* value)
{
    // ex IntMappedMessageValue::set
    rejectSet(args, value);
}

int32_t
game::interface::InboxSubsetValue::getDimension(int32_t which) const
{
    // ex IntMappedMessageValue::getDimension
    if (which == 0) {
        return 1;
    } else {
        return static_cast<int32_t>(m_indexes.size()) + 1;
    }
}

interpreter::Context*
game::interface::InboxSubsetValue::makeFirstContext()
{
    // ex IntMappedMessageValue::makeFirstContext
    if (m_indexes.empty()) {
        return 0;
    } else {
        return new InboxSubsetContext(0, m_indexes, m_translator, m_root, m_game);
    }
}

game::interface::InboxSubsetValue*
game::interface::InboxSubsetValue::clone() const
{
    // This copies the vector.
    // Since the vectors are short, this is acceptable and simpler than some reference counting scheme.
    return new InboxSubsetValue(m_indexes, m_translator, m_root, m_game);
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
game::interface::InboxSubsetValue::create(const std::vector<size_t>& indexes, afl::string::Translator& tx, afl::base::Ref<const Root> root, afl::base::Ref<const Game> game)
{
    // ex IntMappedMessageValue::create
    // We want "If Messages Then..." to be a valid test.
    // Therefore, instead of an empty array, return null.
    if (indexes.empty()) {
        return 0;
    } else {
        return new InboxSubsetValue(indexes, tx, root, game);
    }
}
