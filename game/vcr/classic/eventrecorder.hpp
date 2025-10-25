/**
  *  \file game/vcr/classic/eventrecorder.hpp
  *  \brief Class game::vcr::classic::EventRecorder
  */
#ifndef C2NG_GAME_VCR_CLASSIC_EVENTRECORDER_HPP
#define C2NG_GAME_VCR_CLASSIC_EVENTRECORDER_HPP

#include "game/vcr/classic/eventlistener.hpp"
#include "util/stringinstructionlist.hpp"

namespace game { namespace vcr { namespace classic {

    /** Event Recorder.
        This implements the EventListener interface to record and replay events.
        Events are recorded into a StringInstructionList which is a data object that can be passed around between threads.

        The actual serialisation format is private to this class. */
    class EventRecorder : public EventListener {
     public:
        /** Constructor.
            Make an empty EventRecorder. */
        EventRecorder();

        /** Destructor. */
        ~EventRecorder();

        /** Swap content.
            Exchanges this EventRecorder's content with the given StringInstructionList.
            This can be used to extract a recording for passing around,
            or for loading a recording to play back.
            \param content StringInstructionList to swap with */
        void swapContent(util::StringInstructionList& content);

        /** Replay content.
            Calls all callbacks that were called on this object, in the same order, on the given EventListener.
            \param listener Listener to receive callbacks */
        void replay(EventListener& listener);

        /** Get approximation of size of content.
            This result has only informative purposes.
            Zero means no callbacks are contained in this recording.
            \return approximation of size */
        size_t size() const;

        // EventListener:
        virtual void placeObject(Side side, const UnitInfo& info);
        virtual void updateTime(Time_t time, int32_t distance);
        virtual void startFighter(Side side, int track, int position, int distance, int fighterDiff);
        virtual void landFighter(Side side, int track, int fighterDiff);
        virtual void killFighter(Side side, int track);
        virtual void fireBeam(Side side, int track, int target, int hit, int damage, int kill, const HitEffect& effect);
        virtual void fireTorpedo(Side side, int hit, int launcher, int torpedoDiff, const HitEffect& effect);
        virtual void updateBeam(Side side, int id, int value);
        virtual void updateLauncher(Side side, int id, int value);
        virtual void moveObject(Side side, int position);
        virtual void moveFighter(Side side, int track, int position, int distance, FighterStatus status);
        virtual void killObject(Side side);
        virtual void updateObject(Side side, int damage, int crew, int shield);
        virtual void updateAmmo(Side side, int numTorpedoes, int numFighters);
        virtual void updateFighter(Side side, int track, int position, int distance, FighterStatus status);
        virtual void setResult(BattleResult_t result);
        virtual void removeAnimations();

     private:
        util::StringInstructionList m_content;
    };

} } }

#endif
