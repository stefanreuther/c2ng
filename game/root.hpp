/**
  *  \file game/root.hpp
  *  \brief Class game::Root
  */
#ifndef C2NG_GAME_ROOT_HPP
#define C2NG_GAME_ROOT_HPP

#include <memory>
#include "afl/base/ptr.hpp"
#include "afl/base/refcounted.hpp"
#include "afl/bits/smallset.hpp"
#include "afl/charset/charset.hpp"
#include "afl/io/directory.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/config/userconfiguration.hpp"
#include "game/hostversion.hpp"
#include "game/playerlist.hpp"
#include "game/vcr/flak/configuration.hpp"

namespace game {

    class RegistrationKey;
    class SpecificationLoader;
    class StringVerifier;
    class TurnLoader;

    /** Root aggregates all information that is known when a location has been identified as playable.
        After a Root is constructed, all further accesses go through interfaces and can be independant from the actual implementation
        (network, local, or combined). */
    class Root : public afl::base::RefCounted {
     public:
        enum Action {
            /** Allow user to configure a local directory.
                If this is set, the user can configure the local directory associated with this game.
                FIXME: how is this property communicated to the Folder/Account instance? */
            aLocalSetup,

            /** This game can be opened for editing (playing).
                This means the game has a local directory we can use to store data in; gameDirectory() points at a file-system directory.
                If a game does not have this flag, it can only be opened read-only; gameDirectory() is an in-memory or temporary directory.
                This flag is only meaningful if this Root has a TurnLoader.
                If this flag is not set, the TurnLoader will only produce read-only (game::map::Object::ReadOnly) data. */
            aLoadEditable,

            /** Allow user to configure the character set (Game_Charset).

                This flag reports whether the TurnLoader and SpecificationLoader (if present), and the entity creating this Root, honor the Game_Charset.
                Since the character set already affected creation of the Root, changes will get effective when this Root is recreated. */
            aConfigureCharset,

            /** Allow user to configure the finished status of the game (Game_Finished).
                A finished game can be opened and client-side data being edited (e.g. comments), but no commands being given.
                It needs a local directory to work in.

                This flag reports whether the TurnLoader (if present) honors the Game_Finished option. */
            aConfigureFinished,

            /** Allow user to configure the read-only status of the game (Game_ReadOnly).
                A read-only game can be opened for viewing only, no persistent modifications can be done.

                This flag reports whether the TurnLoader (if present) honors the Game_ReadOnly option. */
            aConfigureReadOnly,

            // aConfigureSynchronisation,
            // aConfigureLocalBackups,
            // aConfigureHostVisibility,
            // aImportExport,

            /** Allow user to use the "Sweep" function.
                If this is set, gameDirectory() points at a file-system directory.
                The Root must be recreated after sweeping (Folder::loadGameRoot()). */
            aSweep,

            /** Allow user to use the "Unpack" function.
                If this is set, gameDirectory() points at a file-system directory.
                The Root must be recreated after unpacking (Folder::loadGameRoot()). */
            aUnpack,

            /** Allow user to use the "Maketurn" function.
                If this is set, gameDirectory() points at a file-system directory.
                The Root should be recreated after Maketurn (Folder::loadGameRoot()). */
            aMaketurn
        };
        typedef afl::bits::SmallSet<Action> Actions_t;

        /** Constructor.

            Note that the host configuration and player list must be initialized separately.
            FIXME: reconsider.

            \param gameDirectory game directory, see m_gameDirectory. Must not be null.
            \param specLoader specification loader, see m_specificationLoader. Must not be null.
            \param hostVersion host version, see m_hostVersion
            \param registrationKey registration status, see m_registrationKey. Must not be null.
            \param stringVerifier string verifier, see m_stringVerifier. Must not be null.
            \param charset Character set for files in gameDirectory.
            \param actions allowed actions */
        Root(afl::base::Ref<afl::io::Directory> gameDirectory,
             afl::base::Ref<SpecificationLoader> specLoader,
             game::HostVersion hostVersion,
             std::auto_ptr<RegistrationKey> registrationKey,
             std::auto_ptr<StringVerifier> stringVerifier,
             std::auto_ptr<afl::charset::Charset> charset,
             Actions_t actions);

        /** Destructor. */
        ~Root();

        /** Access game directory.
            See m_gameDirectory. */
        afl::io::Directory& gameDirectory() const;

        /** Access specification loader.
            See m_specificationLoader. */
        SpecificationLoader& specificationLoader();

        /** Access host version.
            See m_hostVersion. */
        HostVersion& hostVersion();
        const HostVersion& hostVersion() const;

        /** Access character set.
            See m_charset. */
        afl::charset::Charset& charset() const;

        /** Access host configuration.
            See m_hostConfiguration. */
        game::config::HostConfiguration& hostConfiguration();
        const game::config::HostConfiguration& hostConfiguration() const;

        /** Access FLAK configuration.
            See m_flakConfiguration. */
        game::vcr::flak::Configuration& flakConfiguration();
        const game::vcr::flak::Configuration& flakConfiguration() const;

        /** Access user configuration (preferences).
            See m_userConfiguration. */
        game::config::UserConfiguration& userConfiguration();
        const game::config::UserConfiguration& userConfiguration() const;

        /** Access player list.
            See m_playerList. */
        PlayerList& playerList();
        const PlayerList& playerList() const;

        /** Access registration status.
            See m_registrationKey. */
        RegistrationKey& registrationKey();
        const RegistrationKey& registrationKey() const;

        /** Access string verifier. */
        const StringVerifier& stringVerifier() const;

        /** Set turn loader. */
        void setTurnLoader(afl::base::Ptr<TurnLoader> turnLoader);

        /** Get turn loader. */
        afl::base::Ptr<TurnLoader> getTurnLoader() const;

        /** Get permitted actions. */
        Actions_t getPossibleActions() const;

        /** Notify listeners. */
        void notifyListeners();

     private:
        /** Game directory.
            If this is a game with local data, points there.
            Otherwise, points at an internal directory within the user profile.
            Scripts can write their state here; this directory is used as the default load directory.

            Specific users will load and save standard data files here (e.g. "player1.rst", "ship1.dat").
            Generic users will load and save PCC2-specific files here (e.g. "chart1.cc"). */
        const afl::base::Ref<afl::io::Directory> m_gameDirectory;

        /** Specification loader.
            This is an implementation-specific class allowing to load game specification files. */
        const afl::base::Ref<SpecificationLoader> m_specificationLoader;

        /** Host version.
            Stores the host version.
            Most code requires host type and version to be known beforehand. */
        HostVersion m_hostVersion;

        /** Host configuration.
            Must be initialized by the creator of the Root object.
            Downstream code can modify this as more information becomes available. */
        game::config::HostConfiguration m_hostConfiguration;

        /** FLAK configuration.
            Must be initialized by the creator of the Root object. */
        game::vcr::flak::Configuration m_flakConfiguration;

        /** User configuration (preferences).
            Must be initialized by the creator of the Root object for now.
            FIXME: make more generic */
        game::config::UserConfiguration m_userConfiguration;

        /** Player list.
            Must be initialized by the creator of the Root object.
            Downstream code can modify this as more information becomes available, but must not change the structure. */
        PlayerList m_playerList;

        /** Registration key.
            This is an implementation-specific class implementing payment status that unfortunately still need to be. */
        std::auto_ptr<RegistrationKey> m_registrationKey;

        /** String verifier.
            This is an implementation-specific class implementing verification of strings. */
        std::auto_ptr<StringVerifier> m_stringVerifier;

        /** Character set for files in m_gameDirectory.
            This applies to common files (which need a character set even if we're playing from a server that provides one). */
        std::auto_ptr<afl::charset::Charset> m_charset;

        /** Turn loader. */
        // FIXME: this is incomplete; possibly change it again
        afl::base::Ptr<TurnLoader> m_turnLoader;

        /** Actions. */
        Actions_t m_actions;
    };

}

// Access game directory.
inline afl::io::Directory&
game::Root::gameDirectory() const
{
    return *m_gameDirectory;
}

// Access specification loader.
inline game::SpecificationLoader&
game::Root::specificationLoader()
{
    return *m_specificationLoader;
}

// Access host version.
inline game::HostVersion&
game::Root::hostVersion()
{
    return m_hostVersion;
}

// Access host version.
inline const game::HostVersion&
game::Root::hostVersion() const
{
    return m_hostVersion;
}

// Access character set.
inline afl::charset::Charset&
game::Root::charset() const
{
    return *m_charset;
}

// Access host configuration.
inline game::config::HostConfiguration&
game::Root::hostConfiguration()
{
    return m_hostConfiguration;
}

inline const game::config::HostConfiguration&
game::Root::hostConfiguration() const
{
    return m_hostConfiguration;
}

// Access FLAK configuration.
inline game::vcr::flak::Configuration&
game::Root::flakConfiguration()
{
    return m_flakConfiguration;
}

inline const game::vcr::flak::Configuration&
game::Root::flakConfiguration() const
{
    return m_flakConfiguration;
}

// Access user configuration (preferences).
inline game::config::UserConfiguration&
game::Root::userConfiguration()
{
    return m_userConfiguration;
}

inline const game::config::UserConfiguration&
game::Root::userConfiguration() const
{
    return m_userConfiguration;
}

// Access player list.
inline game::PlayerList&
game::Root::playerList()
{
    return m_playerList;
}

inline const game::PlayerList&
game::Root::playerList() const
{
    return m_playerList;
}

// Access registration status.
inline game::RegistrationKey&
game::Root::registrationKey()
{
    return *m_registrationKey;
}

inline const game::RegistrationKey&
game::Root::registrationKey() const
{
    return *m_registrationKey;
}

// Access string verifier.
inline const game::StringVerifier&
game::Root::stringVerifier() const
{
    return *m_stringVerifier;
}

#endif
