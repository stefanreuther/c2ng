/**
  *  \file game/vcr/classic/hostalgorithm.hpp
  */
#ifndef C2NG_GAME_VCR_CLASSIC_HOSTALGORITHM_HPP
#define C2NG_GAME_VCR_CLASSIC_HOSTALGORITHM_HPP

#include "game/vcr/classic/algorithm.hpp"
#include "game/spec/shiplist.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/vcr/statistic.hpp"

namespace game { namespace vcr { namespace classic {


    /** HOST VCR Player.
        This is the THost VCR player, started as a port from the PCC v1.0.13 player, via PCC2 2.0's version.
        It includes all speed optimisations (fast forward, psrandom tables). */
    class HostAlgorithm : public Algorithm {
     public:
        /** Constructor.
            \param nuFlag false: HOST combat; true: NuHost combat.
            \param vis Visualizer to use
            \param config Host configuration (required for PlayerRace)
            \param list Ship list */
        HostAlgorithm(bool nuFlag,
                      Visualizer& vis,
                      const game::config::HostConfiguration& config,
                      const game::spec::ShipList& list);

        /** Destructor. */
        ~HostAlgorithm();

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
        const game::spec::ShipList& m_shipList;
        int m_nuFlag;
        int m_seed;
        Time_t m_time;

        typedef int Random_t;

        enum {
            VCR_MAX_BEAMS = 10,
            VCR_MAX_TORPS = 10,
            VCR_MAX_FTRS  = 19      /* max # of tracks */
        };

        struct Status {
            uint8_t m_beamStatus[VCR_MAX_BEAMS];
            uint8_t m_launcherStatus[VCR_MAX_TORPS];
            uint8_t m_fighterStatus[VCR_MAX_FTRS];
            int16_t m_fighterX[VCR_MAX_FTRS];
            int16_t m_objectX;
            int16_t m_damageLimit;
            int16_t m_numFightersOut;
            Side m_side;
            Object m_obj;

            void init(const Object& obj, Side side);
        };
        class HostStatusToken;

        Status m_status[2];
        Statistic m_statistic[2];
        BattleResult_t m_result;

        inline Random_t getRandom_1_20();
        inline Random_t getRandom_1_100();
        inline Random_t getRandom_1_17();

        inline int32_t rdivadd(int32_t a, int32_t b, int32_t plus);

        inline bool isFreighter(const Status& st);

        void hit(Status& st, int damage, int kill);

        inline void launchFighter(Status& st);
        inline void launchFighters(Status& st);
        inline void fighterShoot(Status& st, Status& opp, int i);
        inline void killFighter(Status& st, int i);
        inline void fighterStuff();

        void rechargeBeams(Status& st);
        void fireBeam(Status& st, Status& opp, int which);
        void fireBeams(Status& st, Status& opp);
        void fireAtFighter(Status& st, Status& opp, int beam);
        void fireBeamsAtFighter(Status& st, Status& opp);

        inline void fireTorp(Status& st, Status& opp, int launcher);
        void fireTorpedoes(Status& st, Status& opp);

        void preloadWeapons(Status& st);

        bool checkSide(Object& obj);
    };

} } }

#endif
