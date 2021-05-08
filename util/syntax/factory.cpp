/**
  *  \file util/syntax/factory.cpp
  *  \brief Class util::syntax::Factory
  */

#include "util/syntax/factory.hpp"
#include "util/syntax/chighlighter.hpp"
#include "util/syntax/inihighlighter.hpp"
#include "util/syntax/lisphighlighter.hpp"
#include "util/syntax/nullhighlighter.hpp"
#include "util/syntax/pascalhighlighter.hpp"
#include "util/syntax/scripthighlighter.hpp"

// Constructor.
util::syntax::Factory::Factory(const KeywordTable& tab)
    : m_table(tab)
{ }

// Create highlighter.
util::syntax::Highlighter&
util::syntax::Factory::create(String_t name, afl::base::Deleter& del)
{
    // ex planetscentral/talk/syntaxnames.cc:createHighlighter

    // Canonicalize name: lowercase, strip ".frag" used for config fragments
    name = afl::string::strLCase(name);
    if (name.size() > 5 && name.compare(name.size()-5, 5, ".frag", 5) == 0) {
        name.erase(name.size()-5);
    }

    // Fixed names
    if (name == "pconfig.src" || name == "shiplist.txt") {
        return del.addNew(new IniHighlighter(m_table, "phost"));
    }
    if (name == "hullfunc.txt") {
        return del.addNew(new IniHighlighter(m_table, "hullfunc"));
    }
    if (name == "amaster.src") {
        return del.addNew(new IniHighlighter(m_table, "amaster"));
    }
    if (name == "pmaster.cfg") {
        return del.addNew(new IniHighlighter(m_table, "pmaster"));
    }
    if (name == "explmap.cfg") {
        return del.addNew(new IniHighlighter(m_table, "explmap"));
    }
    if (name == "map.ini") {
        return del.addNew(new IniHighlighter(m_table, "map"));
    }

    // Extensions (and language names)
    String_t::size_type dot = name.rfind('.');
    if (dot != name.npos) {
        name.erase(0, dot+1);
    }
    if (name == "ini" || name == "cfg" || name == "src") {
        return del.addNew(new IniHighlighter(m_table, String_t()));
    }
    if (name == "q" || name == "ccscript") {
        return del.addNew(new ScriptHighlighter(m_table));
    }
    if (name == "c") {
        return del.addNew(new CHighlighter(CHighlighter::LangC));
    }
    if (name == "c++" || name == "cxx" || name == "cc" || name == "cpp"
        || name == "h++" || name == "hxx" || name == "hh" || name == "hpp"
        || name == "h")
    {
        return del.addNew(new CHighlighter(CHighlighter::LangCXX));
    }
    if (name == "java") {
        return del.addNew(new CHighlighter(CHighlighter::LangJava));
    }
    if (name == "js" || name == "as" || name == "javascript" || name == "jscript") {
        return del.addNew(new CHighlighter(CHighlighter::LangJavaScript));
    }
    if (name == "pas") {
        return del.addNew(new PascalHighlighter());
    }
    if (name == "el" || name == "lisp") {
        return del.addNew(new LispHighlighter());
    }
    return del.addNew(new NullHighlighter());
}
