/**
  *  \file game/map/object.hpp
  *  \brief Base class game::map::Object
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

    /** Map object base class.
        A map object has the following basic properties:
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
        Actual change notification is done by Universe. */
    class Object : public afl::base::Deletable {
     public:
        /** Playability level. */
        enum Playability {
            NotPlayable,               /**< Not playable. For example a foreign ship. */
            ReadOnly,                  /**< Read only. Used to be playable, just not now. For example, our ship in a past turn. */
            Playable,                  /**< Playable. Can be manipulated according to rules. For example, our ship. */
            Editable                   /**< Editable. Can be manipulated, also outside the rules. */
        };


        /*
         *  Constructor and Destructor
         */

        /** Constructor.
            \param id Object Id. The object Id is the value returned by getId(), and usually remains unchanged
                      during the object's lifetime. If needed, it can be changed using setId(). */
        explicit Object(Id_t id);

        /** Copy constructor.
            Copies the other object's playability/selection status, but not its dirtiness status and signals. */
        Object(const Object& other);

        /** Destructor. */
        virtual ~Object();


        /*
         *  Abstract Methods
         */

        /** Get name of this object.
            A name can always be produced, even if the object isn't actually known.
            In this case, a synthetic name ("Ship #99") is produced.
            \param which Which name format to return
            \param tx    Translator
            \param iface Interface to access interpreter properties
            \return Name */
        virtual String_t getName(ObjectName which, afl::string::Translator& tx, InterpreterInterface& iface) const = 0;

        /** Get owner of this object.
            \return owner if known */
        virtual afl::base::Optional<int> getOwner() const = 0;

        /** Get position in game universe.
            \return position if known */
        virtual afl::base::Optional<Point> getPosition() const = 0;


        /*
         *  Management
         */

        /** Get Id number of this object.
            The Id is always known.
            \return Id */
        Id_t getId() const;

        // Playability:

        /** Check playability level.
            \param p Level to check for
            \return true if actual level is p or better */
        bool isPlayable(Playability p) const;

        /** Set playability.
            \param p New playability */
        void setPlayability(Playability p);

        /** Get playability.
            \return playability */
        Playability getPlayability() const;

        // Dirtiness:

        /** Mark object clean. */
        void markClean();

        /** Mark object dirty. */
        void markDirty();

        /** Check whether object is dirty.
            \return true if dirty */
        bool isDirty() const;

        /** Notify all listeners. If this object is dirty, raise sig_change
            and reset dirtiness state.

            You should not use this directly. Use Universe::notifyListeners()
            instead, which offers more flexibility for users to hook into
            universe change. In particular, widgets that hook into
            Universe::sig_preUpdate (those that observe lots of objects)
            will not be notified by a lone notifyListeners(). */
        void notifyListeners();

        // Selection:

        /** Check whether object is marked.
            \return true if object is marked */
        bool isMarked() const;

        /** Set selection status.
            \param n true to mark object */
        void setIsMarked(bool n);

        /** Signal for object changes.
            \param int getId() of the object */
        afl::base::Signal<void(Id_t)> sig_change;

     protected:
        /** Set Id.
            Set the result of getId().
            For use by derived classes.
            \param id Id */
        void setId(Id_t id);

     private:
        Playability m_playability : 8;       /**< Playability. */
        bool m_isMarked;
        bool m_isDirty;
        Id_t m_id;

        Object& operator=(const Object&);
    };

} }

inline game::Id_t
game::map::Object::getId() const
{
    return m_id;
}

inline bool
game::map::Object::isPlayable(Playability p) const
{
    return m_playability >= p;
}

inline void
game::map::Object::setPlayability(Playability p)
{
    m_playability = p;
}

inline game::map::Object::Playability
game::map::Object::getPlayability() const
{
    return m_playability;
}

inline void
game::map::Object::markClean()
{
    m_isDirty = false;
}

inline void
game::map::Object::markDirty()
{
    m_isDirty = true;
}

inline bool
game::map::Object::isDirty() const
{
    return m_isDirty;
}

inline bool
game::map::Object::isMarked() const
{
    return m_isMarked;
}

inline void
game::map::Object::setIsMarked(bool n)
{
    markDirty();
    m_isMarked = n;
}

#endif
