/**
  *  \file game/browser/account.hpp
  *  \brief game::browser::Account
  */
#ifndef C2NG_GAME_BROWSER_ACCOUNT_HPP
#define C2NG_GAME_BROWSER_ACCOUNT_HPP

#include <map>
#include "afl/string/string.hpp"
#include "afl/io/textfile.hpp"

namespace game { namespace browser {

    /** Network account.
        This stores everything required for a network account.
        Information associated with every account has the form of a key/value mapping (ini file).
        Information can be persistent (saved across sessions in network.ini) or transient (like: a password the user chose not to save).

        Each account has three mandatory fields that serve to identify it:
        - user (user name)
        - type (account type)
        - host (host name)

        Each account also has a name, which defaults to "user @ host",
        but can be changed by the user and does not serve as identification in program interfaces.

        Optional, well-known attributes are:
        - password (stored base64-encoded)
        - url (actual server URL; usually left blank to invoke the default)
        - game:<id> (local game folders for games on that server)
        Use of these attributes is at the implementation's discretion.

        This is a plain data class that does not contain any account logic ("how to talk to PlanetsCentral?").
        Account logic is Handler objects known to the browser,
        in particular, its Handler::createAccountFolder() and Handler::loadGameRootMaybe().

        <b>Relation between host, url, and what actually happens:</b>

        \c host is the name users casually use to refer to the server ("I play at planetscentral.com").
        \c url is the underlying API URL ("http://planetscentral.com/api/").
        The program actually talks to an API endpoint built from \c url ("http://planetscentral.com/api/user.cgi").

        <b>Game folders:</b>

        Each game has a local storage associated with it.
        When a game is played, the local storage is created.
        When the user browses to the game via the account ("Account > Game 1"), the link is made via the account object ("game:1 = /path").
        When the user browses to the local storage ("My Computer > C:\ > Games > 1"), the links is made via the game's "c2game.ini".

        Note that we specify a game <b>folder</b> here.
        Whereas a <b>directory</b> would be a physical folder name and nothing else,
        this also allows "game:xxx" shortcuts to $PROFILE/.pcc2/games/xxx", and network storage ("http://planetscentral.com/file.cgi/user/games/1"). */
    class Account {
     public:
        /** Default constructor.
            Makes an empty (invalid) account.
            Call setName(), setUser(), setHost(), setType() to fill it. */
        Account();

        /** Destructor. */
        ~Account();

        /** Set name.
            The name does not serve identification purposes.
            \param name Name */
        void setName(String_t name);

        /** Get name.
            \return name */
        String_t getName() const;

        /** Set attribute.
            \param key Name of attribute. Must consist of identifier letters (alphanumerics) only.
            \param value Value of attribute. Must not contain leading/trailing spaces or newlines. See setEncoded().
            \param persistent true to persist this attribute between sessions, false to discard it at the end */
        void set(String_t key, String_t value, bool persistent);

        /** Get attribute.
            \param key Name of attribute. Must consist of identifier letters (alphanumerics) only.
            \return pointer to attribute value if found; null if not found */
        const String_t* get(String_t key) const;

        /** Get attribute, with default.
            \param key Name of attribute. Must consist of identifier letters (alphanumerics) only.
            \param defaultValue Default value of attribute.
            \return attribute value if found, defaultValue if not found */
        String_t get(String_t key, String_t defaultValue) const;

        /** Set encoded attribute.
            The attribute is stored in base64 encoding.
            Use this for attributes that should be masked from casual readers (e.g. passwords)
            or could possibly contain unsafe characters (leading/trailing spaces, newlines) in their value (e.g. opaque cookies).
            \param key Name of attribute. Must consist of identifier letters (alphanumerics) only.
            \param value Value of attribute.
            \param persistent true to persist this attribute between sessions, false to discard it at the end */
        void setEncoded(String_t key, String_t value, bool persistent);

        /** Get encoded attribute.
            The attribute is stored in base64 encoding.
            \param key [in] Name of attribute. Must consist of identifier letters (alphanumerics) only.
            \param result [out] Value of attribute.
            \retval true Attribute found and decoded
            \retval false Attribute not found */
        bool getEncoded(String_t key, String_t& result) const;

        /** Check validity.
            A valid account has all required mandatory fields.
            Accounts lacking these fields cannot be used.
            \return status */
        bool isValid() const;

        /** Get user name.
            Shortcut for getting the user name, otherwise identical to get().
            \return user name; empty string of not set (error case). */
        String_t getUser() const;

        /** Set user name.
            Shortcut for setting the user name, otherwise identical to set().
            \param user User name */
        void setUser(String_t user);

        /** Get account type.
            Shortcut for getting the account type, otherwise identical to get().
            \return account type; empty string of not set (error case). */
        String_t getType() const;

        /** Set account type.
            Shortcut for setting the account type, otherwise identical to set().
            \param type Account type, alphanumeric */
        void setType(String_t type);

        /** Get host name.
            Shortcut for getting the host name, otherwise identical to get().
            \return host name; empty string of not set (error case). */
        String_t getHost() const;

        /** Set host name.
            Shortcut for setting the host name, otherwise identical to set().
            \param host Host name (domain name) */
        void setHost(String_t host);

        /** Set game folder name.
            \param gameId Game identifier. Typically an alphanumeric string.
            \param folderName Folder name; empty to remove. See class description. */
        void setGameFolderName(String_t gameId, String_t folderName);

        /** Get game folder name.
            \param gameId Game identifier. Typically an alphanumeric string.
            \return Game folder name; null if none set. */
        const String_t* getGameFolderName(String_t gameId) const;

        /** Save this account's data to a text file.
            This is used to create the accounts.ini file.
            \param file Text file */
        void write(afl::io::TextFile& file) const;

     private:
        /** Account information item.
            - first: value
            - second: persistence flag */
        typedef std::pair<String_t, bool> Item_t;

        /** Account name. */
        String_t m_name;

        /** Account information. */
        std::map<String_t, Item_t> m_data;
    };

} }

#endif
