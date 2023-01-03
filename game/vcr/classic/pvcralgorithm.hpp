/**
  *  \file game/vcr/classic/pvcralgorithm.hpp
  *  \brief Class game::vcr::classic::PVCRAlgorithm
  */
#ifndef C2NG_GAME_VCR_CLASSIC_PVCRALGORITHM_HPP
#define C2NG_GAME_VCR_CLASSIC_PVCRALGORITHM_HPP

#include "game/config/hostconfiguration.hpp"
#include "game/spec/shiplist.hpp"
#include "game/vcr/classic/algorithm.hpp"
#include "game/vcr/statistic.hpp"

namespace game { namespace vcr { namespace classic {

/* If defined, compute everything possible in integers.
   If not defined, use floating-point (same as PHost's internal implementation). */
#define PVCR_INTEGER

/* If defined, optimize random number generation.
   If not defined, use direct implementation closer to original one. */
#define PVCR_PREPARED_RNG

    /** PHost VCR algorithm.
        This is the PHost VCR player, started as a port from the PCC v1.0.15 player, and went through serious tweaking.
        This version is based on the PCC2 2.0 version.

        This algorithm supports all PHost versions from 2.4 upwards, up to at least version 4.1.

        Things remaining to do:
        - some things should be done in subroutines (killFighter...)
        - cache weapon specs */
    class PVCRAlgorithm : public Algorithm {
     public:
        /** Constructor.
            \param phost3Flag false: PHost 2.x combat; true: PHost 3.x/4.x combat.
            \param vis Visualizer to use
            \param config Host configuration (required for PlayerRace)
            \param beams Beams
            \param launchers Torpedo launcher */
        PVCRAlgorithm(bool phost3Flag,
                      Visualizer& vis,
                      const game::config::HostConfiguration& config,
                      const game::spec::BeamVector_t& beams,
                      const game::spec::TorpedoVector_t& launchers);

        /** Destructor. */
        ~PVCRAlgorithm();

        // Algorithm methods:
        virtual bool checkBattle(Object& left, Object& right, uint16_t& seed);
        virtual void initBattle(const Object& left, const Object& right, uint16_t seed);
        virtual void doneBattle(Object& left, Object& right);
        virtual bool setCapabilities(uint16_t cap);
        virtual bool playCycle();
        virtual void playFastForward();

        virtual int getBeamStatus(Side side, int id);
        virtual int getLauncherStatus(Side side, int id);
        virtual int getNumTorpedoes(Side side);
        virtual int getNumFighters(Side side);
        virtual int getShield(Side side);
        virtual int getDamage(Side side);
        virtual int getCrew(Side side);
        virtual int getFighterX(Side side, int id);
        virtual FighterStatus getFighterStatus(Side side, int id);
        virtual int getObjectX(Side side);
        virtual int32_t getDistance();

        virtual StatusToken* createStatusToken();
        virtual void restoreStatus(const StatusToken& token);

        virtual Time_t getTime();
        virtual BattleResult_t getResult();
        virtual Statistic getStatistic(Side side);

     private:
        const game::config::HostConfiguration& m_config;
        const game::spec::BeamVector_t& m_beams;
        const game::spec::TorpedoVector_t& m_launchers;
        bool m_phost3Flag;
        uint32_t m_seed;
        Time_t m_time;

        enum {
            VCR_MAX_BEAMS = 20,
            VCR_MAX_TORPS = 20,
            VCR_MAX_FTRS  = 50,
            VCR_MAX_BAYS  = 50      // 50 allowed on planets, 20 on ship
        };

        struct RegularFormula;
        struct AlternativeFormula;

        /** Changing values. */
        struct RunningStatus {
            int  m_beamStatus[VCR_MAX_BEAMS];            ///< Beam status, [0, ~1000].
            int  m_launcherStatus[VCR_MAX_TORPS];        ///< Torpedo launcher status, [0, ~1000].
            int  m_bayStatus[VCR_MAX_BAYS];              ///< Fighter bay status, [0, ~1000].
            uint8_t m_fighterStatus[VCR_MAX_FTRS];       ///< Fighter status, [0, 2].
            int  m_fighterStrikesLeft[VCR_MAX_FTRS];     ///< Fighter strikes remaining.
            int  m_fighterX[VCR_MAX_FTRS];               ///< Fighter X position, in meters.
            int  m_objectX;                              ///< X position of baseship, in meters.
            int  m_activeFighters;                       ///< Number of fighters currently out.
            int  m_launchCountdown;                      ///< Countdown to next fighter launch.
            Object obj;                                  ///< Object.

#ifndef PVCR_INTEGER
            double shield;        ///< Shield status, [0, 100]
            double damage;        ///< Damage, [0, 150]
            double crew;          ///< Crew, [0, 100]
#else
            int32_t shield_scaled;    ///< Shield status, scaled with scale
            int32_t damage_scaled2;   ///< Damage, scaled with scale*100
            int32_t crew_scaled2;     ///< Crew, scaled with scale*100
#endif
            RunningStatus()
                : m_objectX(-9999)
                { }
        };

#ifdef PVCR_PREPARED_RNG
        struct PreparedRNG {
            uint32_t divi;
            uint32_t limit;

            void operator=(uint32_t max);
        };
        typedef PreparedRNG RandomConfig_t;
#else
        typedef int RandomConfig_t;
#endif

        /** Precomputed values which do not change. */
        struct FixedStatus {
            RandomConfig_t beam_recharge;  ///< Precomputed gross beam recharge rate.
            RandomConfig_t bay_recharge;   ///< Precomputed torp recharge rate
            RandomConfig_t torp_recharge;  ///< Precomputed bay recharge rate
            int beam_hit_odds;      ///< Precomputed gross beam hit odds (%)
            int torp_hit_odds;      ///< Precomputed gross torp hit odds (%)
            int beam_kill;
            int beam_damage;
            int torp_kill;
            int torp_damage;
            Side side;
#ifdef PVCR_INTEGER
            int32_t scale;            ///< Scale factor for status value
            int32_t max_scaled;       ///< Maximum damage value. 10000 * scale
            int32_t damage_limit_scaled; ///< Scaled damage limit, scaled with 100*scale
            int32_t mass_plus1;
#else
            double mass_plus1;
            int damage_limit;       ///< Damage limit (150 or 100).
#endif
            /// \name Cached and adjusted values of combat configuration options.
            //@{
            int ShieldDamageScaling;
            int ShieldKillScaling;  // new 4.0
            int HullDamageScaling;
            int CrewKillScaling;
            int MaxFightersLaunched;
            int StrikesPerFighter;
            int BayLaunchInterval;
            int FighterMovementSpeed;
            int FighterBeamExplosive;
            int FighterBeamKill;
            int FighterFiringRange;
            int BeamHitFighterRange;
            int BeamHitFighterCharge;
            int BeamFiringRange;
            int BeamHitShipCharge;
            int TorpFiringRange;
            int ShipMovementSpeed;
            ///@}
        };

        struct Status {
            FixedStatus f;
            RunningStatus r;
            Statistic m_statistic;
        };

        struct PVCRStatusToken;

        struct DetectorStatus {
#ifdef PVCR_INTEGER
            int32_t shield_scaled, damage_scaled, crew_scaled;
#else
            double shield, damage, crew;
#endif
            int fighters, torps;
        };

        Status m_status[2];
        bool m_done;
        int m_interceptProbability;      // Probability of Fighter Intercept happening
        int m_rightProbability;          // Probability of right fighter winning Intercept Attack
        uint16_t m_capabilities;
        DetectorStatus m_detectorStatus[2];
        bool m_detectorValid;            // True if m_detectorStatus is initialized
        Time_t m_detectorTimer;          // Time when to re-check m_detectorStatus
        BattleResult_t m_result;
        bool m_alternativeCombat;
        bool m_fireOnAttackFighters;
        int32_t m_standoffDistance;

        inline uint32_t random64k();
#ifdef PVCR_PREPARED_RNG
        inline int randomRange(const PreparedRNG& rng);
#else
        inline int randomRange(uint32_t n);
#endif
        inline int randomRange100();
        inline bool randomRange100LT(int comp);

        template<typename Formula>
        static inline bool hitT(Status& st, int kill, int expl, bool is_death_ray);
        bool hit(Status& st, int kill, int expl, bool is_death_ray);

        inline int computeBayRechargeRate(int num, const Object& obj) const;
        void fighterRecharge(Status& st);
        void fighterLaunch(Status& st);
        void fighterMove(Status& st);
        void fighterIntercept();
        bool fighterAttack(Status& st, Status& opp);

        inline int computeBeamHitOdds(const game::spec::Beam& beam, const Object& obj) const;
        inline int computeBeamRechargeRate(const game::spec::Beam& beam, const Object& obj) const;
        void beamRecharge(Status& st);
        int beamFindNearestFighter(const Status& st, const Status& oppst) const;
        bool beamFire(Status& st, Status& opp);

        inline int computeTorpHitOdds(const game::spec::TorpedoLauncher& torp, const Object& obj) const;
        inline int computeTubeRechargeRate(const game::spec::TorpedoLauncher& torp, const Object& obj) const;
        void torpsRecharge(Status& st);
        bool torpsFire(Status& st, Status& opp);

        void moveObjects();
        bool canStillFight(const Status& st, const Status& opp) const;

        void initActivityDetector();
        static bool compareDetectorStatus(const DetectorStatus& a, const Status& st);
        static void setDetectorStatus(DetectorStatus& a, const Status& st);
        bool checkCombatActivity();

        bool checkSide(Object& obj) const;
    };

} } }

#endif
