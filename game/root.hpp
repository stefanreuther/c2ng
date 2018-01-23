/**
  *  \file game/root.hpp
  *  \brief Class game::Root
  */
#ifndef C2NG_GAME_ROOT_HPP
#define C2NG_GAME_ROOT_HPP

#include <memory>
#include "afl/base/ptr.hpp"
#include "afl/base/refcounted.hpp"
#include "afl/io/directory.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/config/userconfiguration.hpp"
#include "game/hostversion.hpp"
#include "game/playerlist.hpp"

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
        /** Constructor.

            Note that the host configuration and player list must be initialized separately.
            FIXME: reconsider.

            \param specificationDirectory specification directory, see m_specificationDirectory. Must not be null.
            \param gameDirectory game directory, see m_gameDirectory. Must not be null.
            \param specLoader specification loader, see m_specificationLoader. Must not be null.
            \param hostVersion host version, see m_hostVersion
            \param registrationKey registration status, see m_registrationKey. Must not be null.
            \param stringVerifier string verifier, see m_stringVerifier. Must not be null. */
        Root(afl::base::Ref<afl::io::Directory> specificationDirectory,
             afl::base::Ref<afl::io::Directory> gameDirectory,
             afl::base::Ref<SpecificationLoader> specLoader,
             game::HostVersion hostVersion,
             std::auto_ptr<RegistrationKey> registrationKey,
             std::auto_ptr<StringVerifier> stringVerifier);

        /** Destructor. */
        ~Root();

        /** Access specification directory.
            See m_specificationDirectory. */
        afl::io::Directory& specificationDirectory() const;

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

        /** Access host configuration.
            See m_hostConfiguration. */
        game::config::HostConfiguration& hostConfiguration();
        const game::config::HostConfiguration& hostConfiguration() const;

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
        StringVerifier& stringVerifier();

        /** Set turn loader. */
        void setTurnLoader(afl::base::Ptr<TurnLoader> turnLoader);

        /** Get turn loader. */
        afl::base::Ptr<TurnLoader> getTurnLoader() const;

        /** Notify listeners. */
        void notifyListeners();

     private:
        /** Specification directory.
            If this is a game with local data, points to a MultiDirectory consisting of the game directory and the root specification directory.
            Otherwise, points to the root specification directory only.

            Specific users will load standard specification files here (e.g. "engspec.dat").
            Generic users will load PCC2-specific specification files here (e.g. "hullfunc.cc", "names.cc"). */
        afl::base::Ref<afl::io::Directory> m_specificationDirectory;

        /** Game directory.
            If this is a game with local data, points there.
            Otherwise, points at an internal directory within the user profile.

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

        /** Turn loader. */
        // FIXME: this is incomplete; possibly change it again
        afl::base::Ptr<TurnLoader> m_turnLoader;
    };

}

#endif
