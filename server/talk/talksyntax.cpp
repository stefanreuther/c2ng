/**
  *  \file server/talk/talksyntax.cpp
  *  \brief Class server::talk::TalkSyntax
  */

#include "server/talk/talksyntax.hpp"
#include "server/types.hpp"
#include "server/errors.hpp"

// Constructor.
server::talk::TalkSyntax::TalkSyntax(const util::syntax::KeywordTable& table)
    : m_table(table)
{ }

// Get single syntax entry.
String_t
server::talk::TalkSyntax::get(String_t key)
{
    // ex doSyntaxGet
    if (const String_t* p = m_table.get(key)) {
        return *p;
    } else {
        throw std::runtime_error(SYNTAX_NOT_FOUND);
    }
}

// Get multiple syntax entries.
afl::base::Ref<afl::data::Vector>
server::talk::TalkSyntax::mget(afl::base::Memory<const String_t> keys)
{
    // ex doSyntaxMGet
    afl::base::Ref<afl::data::Vector> r = afl::data::Vector::create();
    while (const String_t* in = keys.eat()) {
        if (const String_t* p = m_table.get(*in)) {
            r->pushBackString(*p);
        } else {
            r->pushBackNew(0);
        }
    }
    return *r;
}
