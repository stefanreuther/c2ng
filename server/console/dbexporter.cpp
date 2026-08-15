/**
  *  \file server/console/dbexporter.cpp
  *  \brief Database Export
  */

#include <stdexcept>
#include "server/console/dbexporter.hpp"
#include "afl/data/access.hpp"
#include "afl/data/segment.hpp"
#include "afl/data/stringlist.hpp"
#include "afl/net/redis/hashkey.hpp"
#include "afl/net/redis/key.hpp"
#include "afl/net/redis/stringkey.hpp"
#include "afl/net/redis/stringlistkey.hpp"
#include "afl/net/redis/stringsetkey.hpp"
#include "afl/string/format.hpp"
#include "server/console/parser.hpp"
#include "server/console/terminal.hpp"
#include "server/types.hpp"
#include "util/string.hpp"

using afl::net::redis::HashKey;
using afl::net::redis::Key;
using afl::net::redis::StringKey;
using afl::net::redis::StringListKey;
using afl::net::redis::StringSetKey;
using afl::string::Format;
using server::console::Parser;

namespace {
    class IndirectSorter {
     public:
        IndirectSorter(const afl::data::StringList_t& values)
            : m_values(values)
            { }
        bool operator()(size_t a, size_t b) const
            { return m_values[a] < m_values[b]; }
     private:
        const afl::data::StringList_t& m_values;
    };


    /** Get keys matching a wildcard (redis KEYS command).
        The redis client does not have a direct mapping for the "keys" command, so we need our own version.
        \param dbConnection Database to work on
        \param match Wildcard
        \param keys [out] List of keys */
    void getKeys(afl::net::CommandHandler& dbConnection, const String_t& match, afl::data::StringList_t& keys)
    {
        std::auto_ptr<afl::data::Value> val(dbConnection.call(afl::data::Segment().pushBackString("KEYS").pushBackString(match)));
        afl::data::Access(val).toStringList(keys);
        std::sort(keys.begin(), keys.end());
    }

    /** Export a database subtree.
        \param out Output receiver
        \param dbConnection Database to work on
        \param match Wildcard to match keys to export */
    void exportSubtree(server::console::Terminal& out, afl::net::CommandHandler& dbConnection, String_t match)
    {
        afl::data::StringList_t keys;
        getKeys(dbConnection, match, keys);
        for (size_t i = 0; i < keys.size(); ++i) {
            const String_t& name = keys[i];
            switch (Key(dbConnection, name).getType()) {
             case Key::None:
                out.printOutput(Format("# warning: key %s got deleted during export", Parser::quoteConsoleString(name)));
                break;
             case Key::String:
                out.printOutput(Format("silent redis set   %-30s %s",
                                       Parser::quoteConsoleString(name),
                                       Parser::quoteConsoleString(StringKey(dbConnection, name).get())));
                break;
             case Key::List: {
                afl::data::StringList_t values;
                StringListKey(dbConnection, name).getAll(values);
                for (size_t j = 0; j < values.size(); ++j) {
                    out.printOutput(Format("silent redis rpush %-30s %s",
                                           Parser::quoteConsoleString(name),
                                           Parser::quoteConsoleString(values[j])));
                }
                break;
             }
             case Key::Set: {
                afl::data::StringList_t values;
                StringSetKey(dbConnection, name).getAll(values);
                std::sort(values.begin(), values.end());
                for (size_t j = 0; j < values.size(); ++j) {
                    out.printOutput(Format("silent redis sadd  %-30s %s",
                                           Parser::quoteConsoleString(name),
                                           Parser::quoteConsoleString(values[j])));
                }
                break;
             }
             case Key::Hash: {
                afl::data::StringList_t values;
                HashKey(dbConnection, name).getAll(values);

                // Sort for reproducability!
                std::vector<size_t> indexes;
                for (size_t j = 0; j+1 < values.size(); j += 2) {
                    indexes.push_back(j);
                }
                std::sort(indexes.begin(), indexes.end(), IndirectSorter(values));

                // Output
                for (size_t j = 0; j < indexes.size(); ++j) {
                    out.printOutput(Format("silent redis hset  %-30s %s %s",
                                           Parser::quoteConsoleString(name),
                                           Parser::quoteConsoleString(values[indexes[j]]),
                                           Parser::quoteConsoleString(values[indexes[j]+1])));
                }
                break;
             }
             case Key::ZSet:
             case Key::Unknown:
                out.printOutput(Format("# warning: key %s has an unsupported type", Parser::quoteConsoleString(name)));
                break;
            }
        }
    }
}

// Export database.
void
server::console::exportDatabase(Terminal& out,
                                afl::net::CommandHandler& dbConnection,
                                interpreter::Arguments& commandLine)
{
    // ex server::dbexport::exportDatabase
    // ex planetscentral/dbexport/exdb.cc:doDatabaseExport
    bool withDelete = false;
    while (commandLine.getNumArgs() > 0) {
        String_t arg = toString(commandLine.getNext());
        if (util::isOption(arg, "delete")) {
            withDelete = true;
        } else if (util::isOption(arg)) {
            throw std::runtime_error("invalid option specified");
        } else {
            if (withDelete) {
                out.printOutput(Format("redis keys %s | silent noerror redis del", Parser::quoteConsoleString(arg)));
            }
            exportSubtree(out, dbConnection, arg);
        }
    }
}
