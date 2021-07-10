/**
  *  \file game/vcr/flak/eventrecorder.hpp
  *  \brief Class game::vcr::flak::EventRecorder
  */
#ifndef C2NG_GAME_VCR_FLAK_EVENTRECORDER_HPP
#define C2NG_GAME_VCR_FLAK_EVENTRECORDER_HPP

#include "game/vcr/flak/visualizer.hpp"
#include "util/stringinstructionlist.hpp"

namespace game { namespace vcr { namespace flak {

    /** Event Recorder.
        This implements the Visualizer interface to record and replay events.
        Events are recorded into a StringInstructionList which is a data object that can be passed around between threads.

        The actual serialisation format is private to this class. */
    class EventRecorder : public Visualizer {
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
            Calls all callbacks that were called on this object, in the same order, on the given Visualizer.
            \param vis Listener to receive callbacks */
        void replay(Visualizer& vis) const;

        /** Get approximation of size of content.
            This result has only informative purposes.
            Zero means no callbacks are contained in this recording.
            \return approximation of size */
        size_t size() const;

        // Visualizer:
        virtual void updateTime(int32_t time);
        virtual void fireBeamFighterFighter(Object_t from, Object_t to, bool hits);
        virtual void fireBeamFighterShip(Object_t from, Ship_t to, bool hits);
        virtual void fireBeamShipFighter(Ship_t from, int beamNr, Object_t to, bool hits);
        virtual void fireBeamShipShip(Ship_t from, int beamNr, Ship_t to, bool hits);
        virtual void createFighter(Object_t id, const Position& pos, int player, Ship_t enemy);
        virtual void killFighter(Object_t id);
        virtual void landFighter(Object_t id);
        virtual void moveFighter(Object_t id, const Position& pos, Ship_t to);
        virtual void createFleet(Fleet_t fleetNr, int32_t x, int32_t y, int player, Ship_t firstShip, size_t numShips);
        virtual void setEnemy(Fleet_t fleetNr, Ship_t enemy);
        virtual void killFleet(Fleet_t fleetNr);
        virtual void moveFleet(Fleet_t fleetNr, int32_t x, int32_t y);
        virtual void createShip(Ship_t shipNr, const Position& pos, const ShipInfo& info);
        virtual void killShip(Ship_t shipNr);
        virtual void moveShip(Ship_t shipNr, const Position& pos);
        virtual void createTorpedo(Object_t id, const Position& pos, int player, Ship_t enemy);
        virtual void hitTorpedo(Object_t id, Ship_t shipNr);
        virtual void missTorpedo(Object_t id);
        virtual void moveTorpedo(Object_t id, const Position& pos);

     private:
        util::StringInstructionList m_content;
    };

} } }

#endif
