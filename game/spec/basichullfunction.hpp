/**
  *  \file game/spec/basichullfunction.hpp
  *  \brief Class game::spec::BasicHullFunction
  */
#ifndef C2NG_GAME_SPEC_BASICHULLFUNCTION_HPP
#define C2NG_GAME_SPEC_BASICHULLFUNCTION_HPP

#include "afl/string/string.hpp"
#include "afl/base/optional.hpp"
#include "game/config/hostconfiguration.hpp"

namespace game { namespace spec {

    /** Basic hull function.
        This defines a basic hull function as known to the host.
        We read their definition from a definition file,
        to allow easier upgrade (and storage of friendly help texts) for future functions. */
    class BasicHullFunction {
     public:
        /** Standard hull functions numbers.
            These are used by our core to check for presence of specific ship abilities.
            We do not require actual hull function numbers to be one of these constants.
            Hull functions must be defined in hullfunc.cc to be visible to users. */
        static const int MerlinAlchemy     = 0;        ///< 0 = Merlin Alchemy: 9 Sup -> 3 Min
        static const int NeutronicRefinery = 1;        ///< 1 = Neutronic Refinery: Min + Sup -> Fuel
        static const int AriesRefinery     = 2;        ///< 2 = Aries Refinery: Min -> Fuel
        static const int HeatsTo50         = 3;        ///< 3 = Bohemian Terraformer
        static const int CoolsTo50         = 4;        ///< 4 = Eros Terraformer
        static const int HeatsTo100        = 5;        ///< 5 = Onyx Terraformer
        static const int Hyperdrive        = 6;        ///< 6
        static const int Gravitonic        = 7;        ///< 7
        static const int ScansAllWormholes = 8;        ///< 8 = Bohemian
        static const int LadyRoyale        = 9;        ///< 9 = Lady Royale
        static const int LokiAnticloak     = 10;       ///< 10 = Loki
        static const int ImperialAssault   = 11;       ///< 11 = SSD
        static const int FirecloudChunnel  = 12;       ///< 12 = Firecloud
        static const int Ramscoop          = 13;       ///< 13 = Cobol
        static const int FullBioscan       = 14;       ///< 14 = Pawn
        static const int AdvancedCloak     = 15;       ///< 15 = Dark Wing
        static const int Cloak             = 16;       ///< 16
        static const int Bioscan           = 17;       ///< 17
        static const int SaberGlory        = 18;       ///< 18 = Saber (10% damage to own ships)
        static const int D19bGlory         = 19;       ///< 19 = D19b (20% damage to own ships)
        static const int Unclonable        = 20;       ///< 20
        static const int CloneOnce         = 21;       ///< 21
        static const int Ungiveable        = 22;       ///< 22
        static const int GiveOnce          = 23;       ///< 23
        static const int Level2Tow         = 24;       ///< 24
        static const int Tow               = 25;       ///< 25 = depends on AllowOneEngineTowing setting
        static const int ChunnelSelf       = 26;       ///< 26
        static const int ChunnelOthers     = 27;       ///< 27
        static const int ChunnelTarget     = 28;       ///< 28
        static const int PlanetImmunity    = 29;       ///< 29 = Rebels, Klingons, if configured, plus SSD
        static const int OreCondenser      = 30;       ///< 30
        static const int Boarding          = 31;       ///< 31 = Privs, Crystals
        static const int AntiCloakImmunity = 32;       ///< 32 = implied by AntiCloakImmunity option
        static const int Academy           = 33;       ///< 33
        static const int Repairs           = 34;       ///< 34
        static const int FullWeaponry      = 35;       ///< 35 = Feds, if configured
        static const int HardenedEngines   = 36;       ///< 36
        static const int Commander         = 37;       ///< 37
        static const int IonShield         = 38;       ///< 38
        static const int HardenedCloak     = 39;       ///< 39
        static const int AdvancedAntiCloak = 40;       ///< 40

        /** Constructor.
            \param id     [in] Id under which host refers to this function
            \param name   [in] Name (identifier) */
        BasicHullFunction(int id, String_t name);

        /** Destructor. */
        ~BasicHullFunction();

        /** Set function name.
            \param name Name */
        void setName(const String_t& name);

        /** Set short description of function.
            This is what we show to users, a short one-liner.
            \param description new description */
        void setDescription(const String_t& description);

        /** Set explanation of function.
            This is the detailed explanation shown upon user request.
            It can contain multiple lines.
            \param explanation new explanation */
        void setExplanation(const String_t& explanation);

        /** Add to explanation.
            Adds a new line to the existing explanation.
            \param explanation line to add (may include '\\n' but doesn't have to)
            \see setExplanation */
        void addToExplanation(const String_t& explanation);

        /** Set picture name.
            This is used to build resource names for showing this ability to the user.
            \param name Name */
        void setPictureName(const String_t& name);

        /** Set implied function Id.
            Each function can imply another one (usually a lesser version of it),
            meaning that a ship having both will perform only the better one, or, in other words,
            a ship having the better one can also do what the lesser one does.
            \param impliedFunctionId Id of implied function, see getId(). Use -1 for no implied function (default). */
        void setImpliedFunctionId(int impliedFunctionId);

        /** Get function Id.
            \return function Id */
        int getId() const;

        /** Get function name.
            \return function name, see setName() */
        const String_t& getName() const;

        /** Get function description.
            \return description, see setDescription() */
        const String_t& getDescription() const;

        /** Get explanation.
            \return explanation, see setExplanation() */
        const String_t& getExplanation() const;

        /** Get picture name.
            \return picture name, see setPictureName() */
        const String_t& getPictureName() const;

        /** Get implied function Id.
            \return implied function Id, -1 if none; see setImpliedFunctionId(). */
        int getImpliedFunctionId() const;

        /** Get damage limit for a function.
            If the return value is valid, the function ceases to work when the ship has
            at least this much damage (that is, 0 means it never works!).
            An unset return value (Nothing) means it never fails.
            \param forOwner Owner
            \param config Host configuration
            \return damage limit */
        afl::base::Optional<int> getDamageLimit(int forOwner, const game::config::HostConfiguration& config) const;

     private:
        const int m_id;
        String_t m_name;
        String_t m_description;
        String_t m_explanation;
        String_t m_pictureName;
        int m_impliedFunctionId;
    };

} }

#endif
