/**
  *  \file server/console/dbexporter.cpp
  *  \brief Database Export
  */

#include <stdexcept>
#include "server/console/dbexporter.hpp"
#include "afl/base/closure.hpp"
#include "afl/data/access.hpp"
#include "afl/data/segment.hpp"
#include "afl/data/stringlist.hpp"
#include "afl/net/redis/hashkey.hpp"
#include "afl/net/redis/key.hpp"
#include "afl/net/redis/sortoperation.hpp"
#include "afl/net/redis/stringfield.hpp"
#include "afl/net/redis/stringkey.hpp"
#include "afl/net/redis/stringlistkey.hpp"
#include "afl/net/redis/stringsetkey.hpp"
#include "afl/net/redis/subtree.hpp"
#include "afl/string/format.hpp"
#include "server/console/fileexporter.hpp"
#include "server/console/parser.hpp"
#include "server/console/terminal.hpp"
#include "server/interface/filebase.hpp"
#include "server/interface/filebaseclient.hpp"
#include "server/types.hpp"
#include "util/string.hpp"

using afl::data::StringList_t;
using afl::net::redis::HashKey;
using afl::net::redis::Key;
using afl::net::redis::StringKey;
using afl::net::redis::StringListKey;
using afl::net::redis::StringSetKey;
using afl::net::redis::Subtree;
using afl::string::Format;
using server::console::DbExporter;
using server::console::Parser;
using server::interface::FileBase;

namespace {
    /* Sort predicate for sorting an array-of-indexes-into-a-StringList_t. */
    class IndirectSorter {
     public:
        IndirectSorter(const StringList_t& values)
            : m_values(values)
            { }
        bool operator()(size_t a, size_t b) const
            { return m_values[a] < m_values[b]; }
     private:
        const StringList_t& m_values;
    };

    /* Filter for exportHash that permits all keys */
    class ShowAll : public afl::base::Closure<bool(const String_t&)> {
     public:
        bool call(const String_t&)
            { return true; }
    };

    /* Filter for exportHash that permits user-specific keys only when refering to users being exported.
       A user-specific key ends with "/userId". */
    class FilterUsers : public afl::base::Closure<bool(const String_t&)> {
     public:
        FilterUsers(const DbExporter& parent)
            : m_parent(parent)
            { }
        bool call(const String_t& key)
            {
                size_t p = key.find('/');
                if (p != String_t::npos) {
                    return m_parent.hasLevel(DbExporter::User, key.substr(p+1), DbExporter::Full);
                } else {
                    return true;
                }
            }
     private:
        const DbExporter& m_parent;
    };

    /** Get keys matching a wildcard (redis KEYS command).
        The redis client does not have a direct mapping for the "keys" command, so we need our own version.
        @param dbConnection Database to work on
        @param match Wildcard
        @param keys [out] List of keys */
    void getKeys(afl::net::CommandHandler& dbConnection, const String_t& match, StringList_t& keys)
    {
        std::auto_ptr<afl::data::Value> val(dbConnection.call(afl::data::Segment().pushBackString("KEYS").pushBackString(match)));
        afl::data::Access(val).toStringList(keys);
        std::sort(keys.begin(), keys.end());
    }

    /** Export a hash key.
        Uses the given filter to restrict the fields that are output.
        @param out          Output receiver
        @param dbConnection Database to work on
        @param name         HashKey name
        @param flter        Filter; called for each key name */
    void exportHash(server::console::Terminal& out, afl::net::CommandHandler& dbConnection, const String_t& name, afl::base::Closure<bool(const String_t&)>& filter)
    {
        StringList_t values;
        HashKey(dbConnection, name).getAll(values);

        // Sort for reproducability!
        std::vector<size_t> indexes;
        for (size_t j = 0; j+1 < values.size(); j += 2) {
            indexes.push_back(j);
        }
        std::sort(indexes.begin(), indexes.end(), IndirectSorter(values));

        // Output
        for (size_t j = 0; j < indexes.size(); ++j) {
            if (filter.call(values[indexes[j]])) {
                out.printOutput(Format("silent redis hset  %-30s %s %s",
                                       Parser::quoteConsoleString(name),
                                       Parser::quoteConsoleString(values[indexes[j]]),
                                       Parser::quoteConsoleString(values[indexes[j]+1])));
            }
        }
    }

    /** Export a single key.
        @param out          Output receiver
        @param dbConnection Database to work on
        @param name         Key name */
    void exportKey(server::console::Terminal& out, afl::net::CommandHandler& dbConnection, const String_t& name)
    {
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
            StringList_t values;
            StringListKey(dbConnection, name).getAll(values);
            for (size_t j = 0; j < values.size(); ++j) {
                out.printOutput(Format("silent redis rpush %-30s %s",
                                       Parser::quoteConsoleString(name),
                                       Parser::quoteConsoleString(values[j])));
            }
            break;
         }
         case Key::Set: {
            StringList_t values;
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
            ShowAll filter;
            exportHash(out, dbConnection, name, filter);
            break;
         }

         case Key::ZSet:
         case Key::Unknown:
            out.printOutput(Format("# warning: key %s has an unsupported type", Parser::quoteConsoleString(name)));
            break;
        }
    }

    /** Export a restricted set.
        Exports the given key as a string set, but only reports elements that have at least the given publicity level in the current job.
        @param out          Output receiver
        @param dbConnection Database to work on
        @param name         StringSetKey name
        @param parent       Calling DbExporter
        @param type         Type to check
        @param level        Level to check for */
    void exportRestrictedSet(server::console::Terminal& out, afl::net::CommandHandler& dbConnection, String_t name, const DbExporter& parent, DbExporter::Type type, DbExporter::Level level)
    {
        if (parent.hasAny(type)) {
            StringList_t values;
            StringSetKey(dbConnection, name).getAll(values);
            std::sort(values.begin(), values.end());
            for (size_t j = 0; j < values.size(); ++j) {
                if (parent.hasLevel(type, values[j], level)) {
                    out.printOutput(Format("silent redis sadd  %-30s %s",
                                           Parser::quoteConsoleString(name),
                                           Parser::quoteConsoleString(values[j])));
                }
            }
        }
    }

    /** Export a restricted set, given a wildcard.
        Like exportRestrictedSet(), but the key names are determined by expanding a wildcard.
        @param out          Output receiver
        @param dbConnection Database to work on
        @param wildName     Wildcard
        @param parent       Calling DbExporter
        @param type         Type to check
        @param level        Level to check for */
    void exportWildSet(server::console::Terminal& out, afl::net::CommandHandler& dbConnection, String_t wildName, const DbExporter& parent, DbExporter::Type type, DbExporter::Level level)
    {
        StringList_t keyNames;
        getKeys(dbConnection, wildName, keyNames);
        for (size_t i = 0; i < keyNames.size(); ++i) {
            exportRestrictedSet(out, dbConnection, keyNames[i], parent, type, level);
        }
    }

    /** Export a database subtree.
        @param out Output receiver
        @param dbConnection Database to work on
        @param match Wildcard to match keys to export */
    void exportSubtree(server::console::Terminal& out, afl::net::CommandHandler& dbConnection, String_t match)
    {
        StringList_t keys;
        getKeys(dbConnection, match, keys);
        for (size_t i = 0; i < keys.size(); ++i) {
            exportKey(out, dbConnection, keys[i]);
        }
    }

    /** Enumerate PM folders and add PMs to export job.
        @param parent Exporter
        @param tree   User tree
        @param folderSet Key containing set of folders */
    void addPMs(DbExporter& parent, Subtree tree, StringSetKey folderSet)
    {
        StringList_t folderIds;
        folderSet.sort().sortLexicographical().get().getResult(folderIds);
        for (size_t i = 0, n = folderIds.size(); i < n; ++i) {
            parent.add(DbExporter::PM, tree.stringSetKey("pm:folder:" + folderIds[i] + ":messages"), DbExporter::Full);
        }
    }
}

server::console::DbExporter::DbExporter()
    : m_job()
{ }

server::console::DbExporter::~DbExporter()
{ }

void
server::console::DbExporter::add(Type t, String_t id, Level lev)
{
    if (id.find_first_not_of("0") != String_t::npos) {
        Level& e = m_job[t][id];
        if (e < lev) {
            e = lev;
        }
    }
}

void
server::console::DbExporter::add(Type t, afl::net::redis::StringSetKey key, Level lev)
{
    StringList_t ids;
    key.getAll(ids);
    for (size_t i = 0, n = ids.size(); i < n; ++i) {
        add(t, ids[i], lev);
    }
}

bool
server::console::DbExporter::hasLevel(Type t, String_t id, Level lev) const
{
    FieldMap_t::const_iterator it = m_job[t].find(id);
    return it != m_job[t].end()
        && it->second >= lev;
}

bool
server::console::DbExporter::hasAny(Type t) const
{
    return !m_job[t].empty();
}

void
server::console::DbExporter::complete(afl::net::CommandHandler& dbConnection)
{
    completeUsers(dbConnection, m_job[User]);
    completePMs(dbConnection, m_job[PM]);
    completeEmails(dbConnection, m_job[Email]);
    completeForums(dbConnection, m_job[Forum]);
    completeThreads(dbConnection, m_job[Thread]);
    completePosts(dbConnection, m_job[Post]);
    completeGroups(dbConnection, m_job[Group]);
    completeGames(dbConnection, m_job[Game]);
    completeTools(dbConnection, m_job[Tool]);
    completeShipLists(dbConnection, m_job[ShipList]);
    completeMasters(dbConnection, m_job[Master]);
    completeHosts(dbConnection, m_job[Host]);
    completeTokens(dbConnection, m_job[Token]);
}

void
server::console::DbExporter::generate(Terminal& out, afl::net::CommandHandler& dbConnection)
{
    generateUsers(out, dbConnection, m_job[User]);
    generatePMs(out, dbConnection, m_job[PM]);
    generateEmails(out, dbConnection, m_job[Email]);
    generateForums(out, dbConnection, m_job[Forum]);
    generateThreads(out, dbConnection, m_job[Thread]);
    generatePosts(out, dbConnection, m_job[Post]);
    generateGroups(out, dbConnection, m_job[Group]);
    generateGames(out, dbConnection, m_job[Game]);
    generateTools(out, dbConnection, m_job[Tool]);
    generateShipLists(out, dbConnection, m_job[ShipList]);
    generateMasters(out, dbConnection, m_job[Master]);
    generateHosts(out, dbConnection, m_job[Host]);
    generateTokens(out, dbConnection, m_job[Token]);
}

void
server::console::DbExporter::generateHostFiles(Terminal& out, afl::net::CommandHandler& fileConnection, FileExporter& fx)
{
    generateFiles(out, fileConnection, "hostfile", m_job[HostFile], fx);
}

void
server::console::DbExporter::generateUserFiles(Terminal& out, afl::net::CommandHandler& fileConnection, FileExporter& fx)
{
    generateFiles(out, fileConnection, "file", m_job[UserFile], fx);
}

/*
 *  "Complete" functions
 *
 *  When output of an object is requested, we must add other objects to satisfy references.
 *
 *  Each of these functions must
 *  - only work on Full or Recursive requests (a Header request must not add new objects)
 *  - add Full requests only for objects processed later on
 *  - never add Recursive requests
 */

void
server::console::DbExporter::completeUsers(afl::net::CommandHandler& dbConnection, const FieldMap_t& job)
{
    for (FieldMap_t::const_iterator it = job.begin(); it != job.end(); ++it) {
        if (it->second >= Full) {
            Subtree tree(dbConnection, "user:" + it->first + ":");
            add(Post,   tree.stringSetKey("forum:posted"), Full);
            add(Forum,  tree.stringSetKey("forum:watchedForums"), Header);
            add(Thread, tree.stringSetKey("forum:watchedThreads"), Header);
            add(Game,   tree.stringSetKey("ownedGames"), Header);

            StringList_t gameIds;
            tree.hashKey("games").getFieldNames(gameIds);
            std::sort(gameIds.begin(), gameIds.end());
            for (size_t i = 0, n = gameIds.size(); i < n; ++i) {
                add(Game, gameIds[i], Full);
            }

            addPMs(*this, tree, tree.stringSetKey("pm:folder:all"));
            addPMs(*this, tree, StringSetKey(dbConnection, "default:folder:all"));

            add(Email, tree.hashKey("profile").stringField("email").get(), Full);

            StringList_t tokens;
            tree.subtree("tokens").getKeyNames(tokens);
            for (size_t i = 0, n = tokens.size(); i < n; ++i) {
                add(Token, tree.subtree("tokens").stringSetKey(tokens[i]), Full);
            }

            if (it->second >= Recursive) {
                String_t userName = tree.stringKey("name").get();
                if (!userName.empty()) {
                    add(UserFile, "u/" + userName, Full);
                }
            }
        }
    }
}

void
server::console::DbExporter::completePMs(afl::net::CommandHandler& dbConnection, const FieldMap_t& job)
{
    for (FieldMap_t::const_iterator it = job.begin(); it != job.end(); ++it) {
        if (it->second >= Full) {
            Subtree tree(dbConnection, "pm:" + it->first + ":");
            add(User, tree.hashKey("header").stringField("author").get(), Header);

            StringList_t to = util::parsePath(tree.hashKey("header").stringField("to").get(), ',');
            for (size_t i = 0, n = to.size(); i < n; ++i) {
                if (const char* p = util::strStartsWith(to[i], "u:")) {
                    add(User, p, Header);
                }
            }
        }
    }
}

void
server::console::DbExporter::completeEmails(afl::net::CommandHandler& dbConnection, const FieldMap_t& job)
{
    for (FieldMap_t::const_iterator it = job.begin(); it != job.end(); ++it) {
        if (it->second >= Full) {
            // When an email address is requested, report all users using it
            Subtree tree(dbConnection, "email:" + it->first + ":");
            const Level nestedLevel = it->second == Recursive ? Full : Header;

            StringList_t keys;
            tree.hashKey("status").getFieldNames(keys);
            for (size_t i = 0, n = keys.size(); i < n; ++i) {
                size_t p = keys[i].find('/');
                if (p != String_t::npos) {
                    add(User, keys[i].substr(p+1), nestedLevel);
                }
            }
        }
    }
}

void
server::console::DbExporter::completeForums(afl::net::CommandHandler& dbConnection, const FieldMap_t& job)
{
    for (FieldMap_t::const_iterator it = job.begin(); it != job.end(); ++it) {
        if (it->second >= Full) {
            // Forums contain references to posts/threads
            Subtree tree(dbConnection, "forum:" + it->first + ":");
            const Level nestedLevel = it->second == Recursive ? Full : Header;
            add(Post, tree.stringSetKey("messages"), nestedLevel);
            add(Thread, tree.stringSetKey("threads"), nestedLevel);
            add(Thread, tree.stringSetKey("stickythreads"), nestedLevel);
            add(Group, tree.hashKey("header").stringField("parent").get(), Header);

            // For recursive, also report watchers
            if (it->second == Recursive) {
                add(User, tree.stringSetKey("watchers"), Header);
            }
        }
    }
}

void
server::console::DbExporter::completeThreads(afl::net::CommandHandler& dbConnection, const FieldMap_t& job)
{
    for (FieldMap_t::const_iterator it = job.begin(); it != job.end(); ++it) {
        if (it->second >= Full) {
            // Threads contain references to posts
            Subtree tree(dbConnection, "thread:" + it->first + ":");
            const Level nestedLevel = it->second == Recursive ? Full : Header;
            add(Post, tree.stringSetKey("messages"), nestedLevel);

            // Referenced forums
            add(Forum, tree.hashKey("header").stringField("forum").get(), Header);
            add(Forum, tree.stringSetKey("also"), Header);

            // For recursive, also report watchers
            if (it->second == Recursive) {
                add(User, tree.stringSetKey("watchers"), Header);
            }
        }
    }
}

void
server::console::DbExporter::completePosts(afl::net::CommandHandler& dbConnection, const FieldMap_t& job)
{
    for (FieldMap_t::const_iterator it = job.begin(); it != job.end(); ++it) {
        if (it->second >= Full) {
            // Post contains references to users, other posts, and threads, which may need headers
            afl::net::redis::HashKey tree(dbConnection, "msg:" + it->first + ":header");
            add(User,   tree.stringField("author").get(), Header);
            add(Post,   tree.stringField("parent").get(), Header);
            add(Thread, tree.stringField("thread").get(), Header);
        }
    }
}

void
server::console::DbExporter::completeGroups(afl::net::CommandHandler& dbConnection, const FieldMap_t& job)
{
    for (FieldMap_t::const_iterator it = job.begin(); it != job.end(); ++it) {
        if (it->second >= Full) {
            // Group references forums and other groups
            Subtree tree(dbConnection, "group:" + it->first + ":");
            add(Forum, tree.stringSetKey("forums"), Header);
            add(Group, tree.stringSetKey("groups"), Header);
            add(Group, tree.hashKey("header").stringField("parent").get(), Header);
        }
    }
}

void
server::console::DbExporter::completeGames(afl::net::CommandHandler& dbConnection, const FieldMap_t& job)
{
    for (FieldMap_t::const_iterator it = job.begin(); it != job.end(); ++it) {
        if (it->second >= Full) {
            // All players should be referenced in :users, so we only check that.
            Subtree tree(dbConnection, "game:" + it->first + ":");
            StringList_t keys;
            tree.hashKey("users").getFieldNames(keys);
            for (size_t i = 0; i < keys.size(); ++i) {
                add(User, keys[i], Header);
            }

            // Further references
            add(Forum,    tree.hashKey("settings").stringField("forum").get(),    Header);
            add(Master,   tree.hashKey("settings").stringField("master").get(),   Header);
            add(ShipList, tree.hashKey("settings").stringField("shiplist").get(), Header);
            add(Host,     tree.hashKey("settings").stringField("host").get(),     Header);
            add(Tool,     tree.stringSetKey("tools"),                             Header);

            if (it->second >= Recursive) {
                add(HostFile, tree.stringKey("dir").get(), Full);
            }
        }
    }
}

void
server::console::DbExporter::completeTools(afl::net::CommandHandler& dbConnection, const FieldMap_t& job)
{
    for (FieldMap_t::const_iterator it = job.begin(); it != job.end(); ++it) {
        if (it->second >= Recursive) {
            add(HostFile, HashKey(dbConnection, "prog:tool:prog:" + it->first).stringField("path").get(), Full);
        }
    }
}

void
server::console::DbExporter::completeShipLists(afl::net::CommandHandler& dbConnection, const FieldMap_t& job)
{
    for (FieldMap_t::const_iterator it = job.begin(); it != job.end(); ++it) {
        if (it->second >= Recursive) {
            add(HostFile, HashKey(dbConnection, "prog:sl:prog:" + it->first).stringField("path").get(), Full);
        }
    }
}

void
server::console::DbExporter::completeMasters(afl::net::CommandHandler& dbConnection, const FieldMap_t& job)
{
    for (FieldMap_t::const_iterator it = job.begin(); it != job.end(); ++it) {
        if (it->second >= Recursive) {
            add(HostFile, HashKey(dbConnection, "prog:master:prog:" + it->first).stringField("path").get(), Full);
        }
    }
}

void
server::console::DbExporter::completeHosts(afl::net::CommandHandler& dbConnection, const FieldMap_t& job)
{
    for (FieldMap_t::const_iterator it = job.begin(); it != job.end(); ++it) {
        if (it->second >= Recursive) {
            add(HostFile, HashKey(dbConnection, "prog:host:prog:" + it->first).stringField("path").get(), Full);
        }
    }
}

void
server::console::DbExporter::completeTokens(afl::net::CommandHandler& dbConnection, const FieldMap_t& job)
{
    for (FieldMap_t::const_iterator it = job.begin(); it != job.end(); ++it) {
        if (it->second >= Full) {
            add(User, HashKey(dbConnection, "token:t:" + it->first).stringField("user").get(), Header);
        }
    }
}

/*
 *  "generate" functions
 */

void
server::console::DbExporter::generateUsers(Terminal& out, afl::net::CommandHandler& dbConnection, const FieldMap_t& job) const
{
    for (FieldMap_t::const_iterator it = job.begin(); it != job.end(); ++it) {
        String_t nameKey = "user:" + it->first + ":name";
        if (it->second == Header) {
            // Header: just identifying information
            exportKey(out, dbConnection, nameKey);
        } else {
            // Full content
            exportSubtree(out, dbConnection, "user:" + it->first + ":*");
        }

        // Always export backlink allowing to resolve name to uid
        String_t nameVal = StringKey(dbConnection, nameKey).get();
        if (!nameVal.empty()) {
            exportKey(out, dbConnection, "uid:" + nameVal);
        }
    }
}

void
server::console::DbExporter::generatePMs(Terminal& out, afl::net::CommandHandler& dbConnection, const FieldMap_t& job) const
{
    for (FieldMap_t::const_iterator it = job.begin(); it != job.end(); ++it) {
        // PMs are always exported completely (but redacted for users):
        // Only for full users, we export the flags (which shows whether user has read the message)
        FilterUsers f(*this);
        exportKey(out, dbConnection, "pm:" + it->first + ":text");
        exportHash(out, dbConnection, "pm:" + it->first + ":header", f);
    }
}

void
server::console::DbExporter::generateEmails(Terminal& out, afl::net::CommandHandler& dbConnection, const FieldMap_t& job) const
{
    for (FieldMap_t::const_iterator it = job.begin(); it != job.end(); ++it) {
        // Redact email status for users that are fully exported
        FilterUsers f(*this);
        exportHash(out, dbConnection, "email:" + it->first + ":status", f);
    }
}

void
server::console::DbExporter::generateForums(Terminal& out, afl::net::CommandHandler& dbConnection, const FieldMap_t& job) const
{
    for (FieldMap_t::const_iterator it = job.begin(); it != job.end(); ++it) {
        if (it->second >= Recursive) {
            // Just export all; completeForums has provided all users
            exportSubtree(out, dbConnection, "forum:" + it->first + ":*");
        } else if (it->second >= Full) {
            // Specific export
            exportKey(out, dbConnection, "forum:" + it->first + ":header");
            exportKey(out, dbConnection, "forum:" + it->first + ":messages");
            exportKey(out, dbConnection, "forum:" + it->first + ":strickythreads");
            exportKey(out, dbConnection, "forum:" + it->first + ":threads");
            exportRestrictedSet(out, dbConnection, "forum:" + it->first + ":watchers", *this, User, Full);
        } else {
            exportKey(out, dbConnection, "forum:" + it->first + ":header");
            exportRestrictedSet(out, dbConnection, "forum:" + it->first + ":messages", *this, Post, Header);
            exportRestrictedSet(out, dbConnection, "forum:" + it->first + ":strickythreads", *this, Post, Header);
            exportRestrictedSet(out, dbConnection, "forum:" + it->first + ":messages", *this, Post, Header);
        }
    }
}

void
server::console::DbExporter::generateThreads(Terminal& out, afl::net::CommandHandler& dbConnection, const FieldMap_t& job) const
{
    for (FieldMap_t::const_iterator it = job.begin(); it != job.end(); ++it) {
        if (it->second >= Recursive) {
            // Just export all; completeForums has provided all users
            exportSubtree(out, dbConnection, "thread:" + it->first + ":*");
        } else if (it->second >= Full) {
            // Specific export
            exportKey(out, dbConnection, "forum:" + it->first + ":header");
            exportKey(out, dbConnection, "forum:" + it->first + ":messages");
            exportRestrictedSet(out, dbConnection, "forum:" + it->first + ":watchers", *this, User, Full);
        } else {
            exportKey(out, dbConnection, "thread:" + it->first + ":header");
            exportRestrictedSet(out, dbConnection, "forum:" + it->first + ":messages", *this, User, Full);
        }
    }
}

void
server::console::DbExporter::generatePosts(Terminal& out, afl::net::CommandHandler& dbConnection, const FieldMap_t& job) const
{
    for (FieldMap_t::const_iterator it = job.begin(); it != job.end(); ++it) {
        if (it->second >= Full) {
            exportSubtree(out, dbConnection, "msg:" + it->first + ":*");
        } else {
            exportKey(out, dbConnection, "msg:" + it->first + ":header");
        }
    }
}

void
server::console::DbExporter::generateGroups(Terminal& out, afl::net::CommandHandler& dbConnection, const FieldMap_t& job) const
{
    for (FieldMap_t::const_iterator it = job.begin(); it != job.end(); ++it) {
        if (it->second >= Full) {
            exportSubtree(out, dbConnection, "group:" + it->first + ":*");
        } else {
            exportKey(out, dbConnection, "group:" + it->first + ":header");
            exportRestrictedSet(out, dbConnection, "group:" + it->first + ":forums", *this, Forum, Header);
            exportRestrictedSet(out, dbConnection, "group:" + it->first + ":groups", *this, Group, Header);
        }
    }
}

void
server::console::DbExporter::generateGames(Terminal& out, afl::net::CommandHandler& dbConnection, const FieldMap_t& job) const
{
    for (FieldMap_t::const_iterator it = job.begin(); it != job.end(); ++it) {
        if (it->second >= Full) {
            exportSubtree(out, dbConnection, "game:" + it->first + ":*");
        } else {
            exportKey(out, dbConnection, "game:" + it->first + ":name");
            exportKey(out, dbConnection, "game:" + it->first + ":type");
            exportKey(out, dbConnection, "game:" + it->first + ":state");
        }
    }

    // Indexes
    exportRestrictedSet(out, dbConnection, "game:all", *this, Game, Header);
    exportRestrictedSet(out, dbConnection, "game:broken", *this, Game, Header);
    exportWildSet(out, dbConnection, "game:state:*", *this, Game, Header);
    exportWildSet(out, dbConnection, "game:pubstate:*", *this, Game, Header);
}

void
server::console::DbExporter::generateTools(Terminal& out, afl::net::CommandHandler& dbConnection, const FieldMap_t& job) const
{
    for (FieldMap_t::const_iterator it = job.begin(); it != job.end(); ++it) {
        exportSubtree(out, dbConnection, "prog:tool:prog:" + it->first);
    }
    exportRestrictedSet(out, dbConnection, "prog:tool:list", *this, Tool, Header);
}

void
server::console::DbExporter::generateShipLists(Terminal& out, afl::net::CommandHandler& dbConnection, const FieldMap_t& job) const
{
    for (FieldMap_t::const_iterator it = job.begin(); it != job.end(); ++it) {
        exportSubtree(out, dbConnection, "prog:tool:sl:" + it->first);
    }
    exportRestrictedSet(out, dbConnection, "prog:sl:list", *this, ShipList, Header);
}

void
server::console::DbExporter::generateMasters(Terminal& out, afl::net::CommandHandler& dbConnection, const FieldMap_t& job) const
{
    for (FieldMap_t::const_iterator it = job.begin(); it != job.end(); ++it) {
        exportSubtree(out, dbConnection, "prog:master:prog:" + it->first);
    }
    exportRestrictedSet(out, dbConnection, "prog:master:list", *this, Master, Header);
}

void
server::console::DbExporter::generateHosts(Terminal& out, afl::net::CommandHandler& dbConnection, const FieldMap_t& job) const
{
    for (FieldMap_t::const_iterator it = job.begin(); it != job.end(); ++it) {
        exportSubtree(out, dbConnection, "prog:host:prog:" + it->first);
    }
    exportRestrictedSet(out, dbConnection, "prog:host:list", *this, Host, Header);
}

void
server::console::DbExporter::generateTokens(Terminal& out, afl::net::CommandHandler& dbConnection, const FieldMap_t& job) const
{
    for (FieldMap_t::const_iterator it = job.begin(); it != job.end(); ++it) {
        exportKey(out, dbConnection, "token:t:" + it->first);
    }
    exportRestrictedSet(out, dbConnection, "token:all", *this, Token, Header);
}

void
server::console::DbExporter::generateFiles(Terminal& out, afl::net::CommandHandler& fileConnection, String_t contextName, const FieldMap_t& job, FileExporter& fx) const
{
    server::interface::FileBaseClient client(fileConnection);
    for (FieldMap_t::const_iterator it = job.begin(); it != job.end(); ++it) {
        StringList_t queue;
        queue.push_back(it->first);
        while (!queue.empty()) {
            // Take directory name
            String_t dirName = queue.back();
            queue.pop_back();

            // List content
            try {
                FileBase::ContentInfoMap_t content;
                client.getDirectoryContent(dirName, content);

                out.printOutput(Format("silent %-8s mkdirhier %s", contextName, Parser::quoteConsoleString(dirName)));
                for (FileBase::ContentInfoMap_t::const_iterator it = content.begin(); it != content.end(); ++it) {
                    const String_t fullName = dirName + "/" + it->first;
                    switch (it->second->type) {
                     case FileBase::IsFile:
                        out.printOutput(Format("silent %-8s put       %-30s %s", contextName, Parser::quoteConsoleString(fullName), fx.exportFile(fullName, client.getFile(fullName))));
                        break;
                     case FileBase::IsDirectory:
                        queue.push_back(fullName);
                        break;
                     case FileBase::IsUnknown:
                        out.printOutput(Format("# warning: unknown item %s has been ignored", Parser::quoteConsoleString(fullName)));
                        break;
                    }
                }
            }
            catch (std::exception& e) {
                out.printOutput(Format("# warning: error while exporting %s, %s", Parser::quoteConsoleString(dirName), Parser::quoteConsoleString(e.what())));
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
