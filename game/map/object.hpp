/**
  *  \file game/map/object.hpp
  */
#ifndef C2NG_GAME_MAP_OBJECT_HPP
#define C2NG_GAME_MAP_OBJECT_HPP

#include "afl/base/deletable.hpp"
#include "afl/base/signal.hpp"
#include "afl/string/translator.hpp"
#include "afl/base/types.hpp"
#include "game/types.hpp"
#include "game/interpreterinterface.hpp"
#include "game/map/point.hpp"

namespace game { namespace map {

    /** Game object base class.
        A game object has the following basic properties:
        - a name (coming in three flavours, plain/long/detailed)
        - an owner
        - an Id
        - a position

        An object can represent an item that may or may not be currently visible.
        Visibility is decided by an ObjectType instance and can be different for different views.

        Each object has a playability attribute:
        - NotPlayable means the object cannot be played. Examples include foreign ships. Those may not have full data.
        - ReadOnly means the unit has full data, but still cannot be played. Examples include objects in history.
        - Playable means the unit can be played. It has full data and can be manipulated according to rules.
        - Editable means the unit can be edited. It has full data and can be manipulated even outside the rules.

        Each object has a "marked" flag to represent the user selection.

        Finally, objects have a "dirty" flag used to track changes,
        and a sig_change signal to allow others to hook into these changes.
        Actual change notification is done by GUniverse. */
    class Object : public afl::base::Deletable {
     public:
        /** Playability level. */
        enum Playability {
            NotPlayable,               /**< Not playable. For example a foreign ship. */
            ReadOnly,                  /**< Read only. Used to be playable, just not now. For example, our ship in a past turn. */
            Playable,                  /**< Playable. Can be manipulated according to rules. For example, our ship. */
            Editable                   /**< Editable. Can be manipulated, also outside the rules. */
        };

        // Constructor and Destructor:
        Object();
        Object(const Object& other);
        virtual ~Object();

        /*
         *  Abstract Methods
         */

        /** Get name of this object.
            A name can always be produced, even if the object isn't actually known.
            In this case, a synthetic name ("Ship #99") is produced.
            \param which Which name format to return */
        virtual String_t getName(ObjectName which, afl::string::Translator& tx, InterpreterInterface& iface) const = 0;

        /** Get Id number of this object.
            The Id is always known. */
        virtual Id_t getId() const = 0;

        /** Get owner of this object.
            \param result [out] Result
            \return true if owner is known */
        virtual bool getOwner(int& result) const = 0;

        /** Get position in game universe. */
        virtual bool getPosition(Point& result) const = 0;


        /*
         *  Management
         */

        // Playability:
        /** Check playability level. */
        bool isPlayable(Playability p) const;
        void setPlayability(Playability p);
        Playability getPlayability() const;

        // Dirtiness:
        void markClean();
        void markDirty();
        bool isDirty() const;
        void notifyListeners();

        // Selection:
        bool isMarked() const;
        void setIsMarked(bool n);

        /** Signal for object changes.
            \param int getId() of the object */
        afl::base::Signal<void(Id_t)> sig_change;

     private:
        Playability m_playability : 8;       /**< Playability. */
        bool m_isMarked;
        bool m_isDirty;

        Object& operator=(const Object&);
    };

} }

#endif
