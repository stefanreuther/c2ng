/**
  *  \file server/dbexport/dbexporter.cpp
  *  \brief Function server::dbexport::exportDatabase
  */

#include <stdexcept>
#include "server/dbexport/dbexporter.hpp"
#include "afl/data/access.hpp"
#include "afl/data/segment.hpp"
#include "afl/data/stringlist.hpp"
#include "afl/net/redis/hashkey.hpp"
#include "afl/net/redis/key.hpp"
#include "afl/net/redis/stringkey.hpp"
#include "afl/net/redis/stringlistkey.hpp"
#include "afl/net/redis/stringsetkey.hpp"
#include "afl/string/format.hpp"

using afl::net::redis::HashKey;
using afl::net::redis::Key;
using afl::net::redis::StringKey;
using afl::net::redis::StringListKey;
using afl::net::redis::StringSetKey;
using afl::string::Format;

namespace {
    String_t quoteConsoleString(const String_t& arg)
    {
        // Check for unprintable characters
        // FIXME: move to utility module?
        bool isPrintable = true;
        for (String_t::size_type i = 0; i < arg.size(); ++i) {
            if (arg[i] < ' ' || arg[i] >= 127) {
                isPrintable = false;
                break;
            }
        }

        // Format it
        if (arg.empty()) {
            return "\"\"";
        } else if (isPrintable && arg.find_first_of(" \"\'<|${}#") == arg.npos) {
            // Nothing special
            return arg;
        } else if (isPrintable && arg.find_first_of("\"\\$") == arg.npos) {
            // Simple double-quote
            return "\"" + arg + "\"";
        } else if (isPrintable && arg.find_first_of("\'") == arg.npos) {
            // Simple single-quote
            return "'" + arg + "'";
        } else {
            // Full version
            String_t result = "\"";
            for (String_t::size_type i = 0; i < arg.size(); ++i) {
                uint8_t ch = arg[i];
                if (ch == '"' || ch == '\\' || ch == '$') {
                    result += '\\';
                    result += char(ch);
                } else if (ch == '\r') {
                    result += "\\r";
                } else if (ch == '\n') {
                    result += "\\n";
                } else if (ch == '\t') {
                    result += "\\t";
                } else if (ch == '\0') {
                    result += "\\0";
                } else if (ch >= ' ' && ch < 127) {
                    result += char(ch);
                } else {
                    result += Format("\\x%02X", ch);
                }
            }
            result += "\"";
            return result;
        }
    }

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
    void exportSubtree(afl::io::TextWriter& out, afl::net::CommandHandler& dbConnection, String_t match)
    {
        afl::data::StringList_t keys;
        getKeys(dbConnection, match, keys);
        for (size_t i = 0; i < keys.size(); ++i) {
            const String_t& name = keys[i];
            switch (Key(dbConnection, name).getType()) {
             case Key::None:
                out.writeLine(Format("# warning: key %s got deleted during export", quoteConsoleString(name)));
                break;
             case Key::String:
                out.writeLine(Format("silent redis set   %-30s %s",
                                     quoteConsoleString(name),
                                     quoteConsoleString(StringKey(dbConnection, name).get())));
                break;
             case Key::List: {
                afl::data::StringList_t values;
                StringListKey(dbConnection, name).getAll(values);
                for (size_t j = 0; j < values.size(); ++j) {
                    out.writeLine(Format("silent redis rpush %-30s %s",
                                         quoteConsoleString(name),
                                         quoteConsoleString(values[j])));
                }
                break;
             }
             case Key::Set: {
                afl::data::StringList_t values;
                StringSetKey(dbConnection, name).getAll(values);
                std::sort(values.begin(), values.end());
                for (size_t j = 0; j < values.size(); ++j) {
                    out.writeLine(Format("silent redis sadd  %-30s %s",
                                         quoteConsoleString(name),
                                         quoteConsoleString(values[j])));
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
                    out.writeLine(Format("silent redis hset  %-30s %s %s",
                                         quoteConsoleString(name),
                                         quoteConsoleString(values[indexes[j]]),
                                         quoteConsoleString(values[indexes[j]+1])));
                }
                break;
             }
             case Key::ZSet:
             case Key::Unknown:
                out.writeLine(Format("# warning: key %s has an unsupported type", quoteConsoleString(name)));
                break;
            }
        }
    }
}

// Export database.
void
server::dbexport::exportDatabase(afl::io::TextWriter& out,
                                 afl::net::CommandHandler& dbConnection,
                                 afl::sys::CommandLineParser& commandLine,
                                 afl::string::Translator& tx)
{
    // ex planetscentral/dbexport/exdb.cc:doDatabaseExport
    bool withDelete = false;
    String_t p;
    bool opt;
    while (commandLine.getNext(opt, p)) {
        if (opt) {
            if (p == "delete") {
                withDelete = true;
            } else {
                throw std::runtime_error(tx("invalid option specified"));
            }
        } else {
            if (withDelete) {
                out.writeLine(Format("redis keys %s | silent noerror redis del", quoteConsoleString(p)));
            }
            exportSubtree(out, dbConnection, p);
        }
    }
}
