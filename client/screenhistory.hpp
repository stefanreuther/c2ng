/**
  *  \file client/screenhistory.hpp
  *  \brief Class client::ScreenHistory
  */
#ifndef C2NG_CLIENT_SCREENHISTORY_HPP
#define C2NG_CLIENT_SCREENHISTORY_HPP

#include <vector>
#include "afl/base/types.hpp"
#include "afl/base/memory.hpp"

namespace client {

    /** History of screens.
        Stores a list of references to screens, and provides operations to manage them. */
    class ScreenHistory {
     public:
        /** Screen type. */
        enum Type {
            Null,
            Ship,
            Planet,
            Starbase,
            HistoryShip,
            Fleet,
            ShipTask,
            PlanetTask,
            StarbaseTask,
            Starchart
            /*
               Missing:
                 RaceScreen (PCC2)
                 Search Result (PCC1)
                 Message (PCC1)
             */
        };

        /** Reference to a screen. */
        class Reference {
         public:
            /** Create a null reference.
                @post !isSet */
            Reference();

            /** Create a reference to the given screen.
                @param type   Type
                @param x      For screens, object Id. For starchart, X coordinate.
                @param y      For screens, 0. For starchart, Y coordinate. */
            Reference(Type type, int x, int y);

            /** Check whether this reference is not null.
                @return true if getType() != Null */
            bool isSet() const;

            /** Get screen type.
                @return screen type */
            Type getType() const;

            /** Get X coordinate or ID.
                @return X coordinate or ID */
            int getX() const;

            /** Get Y coordinate.
                @return Y coordinate */
            int getY() const;

            /** Compare for equality.
                @param other Other reference
                @return true if references are equal */
            bool operator==(const Reference& other) const;

            /** Compare for inequality.
                @param other Other reference
                @return true if references are different */
            bool operator!=(const Reference& other) const;

         private:
            Type m_type;
            int m_x;
            int m_y;
        };

        /** Constructor.
            @param sizeLimit Maximum number of history elements */
        explicit ScreenHistory(size_t sizeLimit);

        /** Push new history Id.
            Call when we're displaying the given context.
            This function will avoid duplicates and can therefore be called at any time.
            @param ref New reference */
        void push(Reference ref);

        /** Pop history Id.
            @return reference; null reference if history is empty */
        Reference pop();

        /** Rotate history.
            Turns situation A-B-C-D-E into E-A-B-C-D. */
        void rotate();

        /** Clear screen history. */
        void clear();

        /** Get entire history.
            @return history, first element is oldest */
        afl::base::Memory<const Reference> getAll() const;

        /** Apply mask filter.
            Removes all history elements except for those where @c mask is present and true. */
        void applyMask(afl::base::Memory<const bool> mask);

     private:
        size_t m_sizeLimit;
        std::vector<Reference> m_data;  // ex screen_history
    };

}

inline
client::ScreenHistory::Reference::Reference()
    : m_type(Null), m_x(0), m_y(0)
{
    // ex WScreenId::WScreenId
}

inline
client::ScreenHistory::Reference::Reference(Type type, int x, int y)
    : m_type(type), m_x(x), m_y(y)
{
    // ex WScreenId::WScreenId
}

inline bool
client::ScreenHistory::Reference::isSet() const
{
    // ex WScreenId::isNonNull
    // This function is not called "isValid" because it cannot check the validity of the referenced screen
    // (that is, whether "Planet 123" actually exists).
    return m_type != Null;
}

inline client::ScreenHistory::Type
client::ScreenHistory::Reference::getType() const
{
    // ex WScreenId::getScreenId
    return m_type;
}

inline int
client::ScreenHistory::Reference::getX() const
{
    // ex WScreenId::getId, WScreenId::getPos (simplified)
    return m_x;
}

inline int
client::ScreenHistory::Reference::getY() const
{
    return m_y;
}

inline bool
client::ScreenHistory::Reference::operator!=(const Reference& other) const
{
    // ex WScreenId::operator!=
    return !operator==(other);
}

#endif
