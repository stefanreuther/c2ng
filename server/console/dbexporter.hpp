/**
  *  \file server/console/dbexporter.hpp
  *  \brief Database Export
  */
#ifndef C2NG_SERVER_CONSOLE_DBEXPORTER_HPP
#define C2NG_SERVER_CONSOLE_DBEXPORTER_HPP

#include <map>
#include "afl/net/commandhandler.hpp"
#include "interpreter/arguments.hpp"
#include "afl/net/redis/stringsetkey.hpp"

namespace server { namespace console {

    class FileExporter;
    class Terminal;

    /** Database object exporter.
        This class allows export of objects with links intact.
        The idea is to be able to export an entire object:
        - all data for a user including all their PMs, forum posts, games, for GDPR compliance
        - all data for a game including all tools and files, for reproduction

        This class works entirely on database (and file) level.
        An object is identified by a string (not an integer like for most services);
        functions try to be upward-compatible and perform very little validation.

        Each object can be exported on different detail levels, see @c Level.

        Output will be redacted to contain user-specific information only if the user is part of the export with at least Full level.
        For example, exporting a forum will only export the user Ids of users participating in the forum,
        but will export watchers only when they are part of the export session.

        To use,
        - construct
        - call add() to add specific objects
        - call complete() to determine additional objects to export
        - call generate() to generate output for database
        - call generateUserFiles(), generateHostFiles() to generate output for files from file services

        As of 2026-08-19, this has the following "privacy leaks":
        - a PM will show the 'ref' attribute (=in how many folders across all users is this message stored)
        - games will contain all users' game directory / hasPlayerFiles settings */
    class DbExporter {
     public:
        /** Object type. */
        enum Type {
            User,               ///< User account (UserManagement interface).
            PM,                 ///< Private message (TalkPM interface).
            Email,              ///< Email address.
            Forum,              ///< Forum (TalkForum interface).
            Thread,             ///< Forum thread (TalkThread interface).
            Post,               ///< Forum posting (TalkPost interface).
            Group,              ///< Forum group (TalkGroup interface).
            Game,               ///< Game (HostGame interface).
            Tool,               ///< Tool for a game (HostTool interface).
            ShipList,           ///< Shiplist for a game (HostTool interface).
            Master,             ///< Master for a game (HostTool interface).
            Host,               ///< Host for a game (HostTool interface).
            Token,              ///< Access token (UserToken interface).
            HostFile,           ///< File tree on host filer (FileBase interface).
            UserFile            ///< File tree on user filer (FileBase interface).
        };
        static const size_t MAX_TYPE = static_cast<size_t>(UserFile) + 1;

        /** Export detail level. */
        enum Level {
            /** Just the bare minimum ("object exists"). */
            Header,

            /** Full object, and the bare minimum of related objects. */
            Full,

            /** Full object and more information of related objects. */
            Recursive
        };

        /** Constructor. */
        DbExporter();

        /** Destructor. */
        ~DbExporter();

        /** Add a single object.
            If the object is already queued with a lower level, increases the level.
            An empty or zero Id is ignored.
            @param t   Type
            @param id  Id
            @param lev Level */
        void add(Type t, String_t id, Level lev);

        /** Add objects from a StringSetKey.
            Performs add() for each element of the set.
            @param t   Type
            @param key Key to read
            @param lev Level */
        void add(Type t, afl::net::redis::StringSetKey key, Level lev);

        /** Check for level.
            @param t   Type
            @param id  Id
            @param lev Level
            @return true if add() has been called for this object, with at least the given level */
        bool hasLevel(Type t, String_t id, Level lev) const;

        /** Check for object type.
            @param t   Type
            @return true if add() has been called for at the given object at least once. */
        bool hasAny(Type t) const;

        /** Complete the jobs by collecting additional objects.
            @param dbConnection  Database connection */
        void complete(afl::net::CommandHandler& dbConnection);

        /** Write output for all database objects.
            @param out           Output receiver
            @param dbConnection  Database connection */
        void generate(Terminal& out, afl::net::CommandHandler& dbConnection);

        /** Write output for all host files.
            @param out             Output receiver
            @param fileConnection  Connection to "hostfile" service
            @param fx              File export policy */
        void generateHostFiles(Terminal& out, afl::net::CommandHandler& fileConnection, FileExporter& fx);

        /** Write output for all user files.
            @param out             Output receiver
            @param fileConnection  Connection to "file" service
            @param fx              File export policy */
        void generateUserFiles(Terminal& out, afl::net::CommandHandler& fileConnection, FileExporter& fx);

     private:
        typedef std::map<String_t,Level> FieldMap_t;
        FieldMap_t m_job[MAX_TYPE];

        void completeUsers(afl::net::CommandHandler& dbConnection, const FieldMap_t& job);
        void completePMs(afl::net::CommandHandler& dbConnection, const FieldMap_t& job);
        void completeEmails(afl::net::CommandHandler& dbConnection, const FieldMap_t& job);
        void completeForums(afl::net::CommandHandler& dbConnection, const FieldMap_t& job);
        void completeThreads(afl::net::CommandHandler& dbConnection, const FieldMap_t& job);
        void completePosts(afl::net::CommandHandler& dbConnection, const FieldMap_t& job);
        void completeGroups(afl::net::CommandHandler& dbConnection, const FieldMap_t& job);
        void completeGames(afl::net::CommandHandler& dbConnection, const FieldMap_t& job);
        void completeTools(afl::net::CommandHandler& dbConnection, const FieldMap_t& job);
        void completeShipLists(afl::net::CommandHandler& dbConnection, const FieldMap_t& job);
        void completeMasters(afl::net::CommandHandler& dbConnection, const FieldMap_t& job);
        void completeHosts(afl::net::CommandHandler& dbConnection, const FieldMap_t& job);
        void completeTokens(afl::net::CommandHandler& dbConnection, const FieldMap_t& job);

        void generateUsers(Terminal& out, afl::net::CommandHandler& dbConnection, const FieldMap_t& job) const;
        void generatePMs(Terminal& out, afl::net::CommandHandler& dbConnection, const FieldMap_t& job) const;
        void generateEmails(Terminal& out, afl::net::CommandHandler& dbConnection, const FieldMap_t& job) const;
        void generateForums(Terminal& out, afl::net::CommandHandler& dbConnection, const FieldMap_t& job) const;
        void generateThreads(Terminal& out, afl::net::CommandHandler& dbConnection, const FieldMap_t& job) const;
        void generatePosts(Terminal& out, afl::net::CommandHandler& dbConnection, const FieldMap_t& job) const;
        void generateGroups(Terminal& out, afl::net::CommandHandler& dbConnection, const FieldMap_t& job) const;
        void generateGames(Terminal& out, afl::net::CommandHandler& dbConnection, const FieldMap_t& job) const;
        void generateTools(Terminal& out, afl::net::CommandHandler& dbConnection, const FieldMap_t& job) const;
        void generateShipLists(Terminal& out, afl::net::CommandHandler& dbConnection, const FieldMap_t& job) const;
        void generateMasters(Terminal& out, afl::net::CommandHandler& dbConnection, const FieldMap_t& job) const;
        void generateHosts(Terminal& out, afl::net::CommandHandler& dbConnection, const FieldMap_t& job) const;
        void generateTokens(Terminal& out, afl::net::CommandHandler& dbConnection, const FieldMap_t& job) const;
        void generateFiles(Terminal& out, afl::net::CommandHandler& fileConnection, String_t contextName, const FieldMap_t& job, FileExporter& fx) const;
    };

    /** Export database.
        @param out          Output receiver
        @param dbConnection Database connection
        @param commandLine  Command line, parsed for options and values to export. */
    void exportDatabase(Terminal& out,
                        afl::net::CommandHandler& dbConnection,
                        interpreter::Arguments& commandLine);

} }

#endif
