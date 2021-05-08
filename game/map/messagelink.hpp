/**
  *  \file game/map/messagelink.hpp
  *  \brief Class game::map::MessageLink
  */
#ifndef C2NG_GAME_MAP_MESSAGELINK_HPP
#define C2NG_GAME_MAP_MESSAGELINK_HPP

#include <vector>
#include <cstddef>

namespace game { namespace map {

    /** List of message links for a unit. */
    class MessageLink {
     public:
        typedef std::vector<size_t> Vector_t;

        /** Constructor.
            Makes an empty object.
            \post empty() */
        MessageLink();

        /** Add a message number.
            \param nr Number */
        void add(size_t nr);

        /** Check emptiness.
            \return true if container is empty */
        bool empty() const;

        /** Access vector of message numbers. */
        const Vector_t& get() const;

     private:
        Vector_t m_data;
    };

} }

#endif
