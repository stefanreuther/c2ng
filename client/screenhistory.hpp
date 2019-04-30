/**
  *  \file client/screenhistory.hpp
  */
#ifndef C2NG_CLIENT_SCREENHISTORY_HPP
#define C2NG_CLIENT_SCREENHISTORY_HPP

#include <vector>
#include "afl/base/types.hpp"
#include "afl/base/memory.hpp"

namespace client {

    class ScreenHistory {
     public:
        enum Type {
            // NullScreen        = -1,
            // RaceScreen        = 0,

            // ShipScreen        = 1,
            // PlanetScreen      = 2,
            // BaseScreen        = 3,
            // Starchart         = 4,

            // HistoryScreen     = 6,

            // FleetScreen       = 10,
            // ShipTaskScreen    = 11,
            // PlanetTaskScreen  = 12,
            // BaseTaskScreen    = 13
            Null,
            Ship,
            Planet,
            Starbase,
            Starchart
        };

        class Reference {
         public:
            Reference();
            Reference(Type type, int x, int y);

            bool isSet() const;
            Type getType() const;
            int getX() const;
            int getY() const;

            bool operator==(const Reference& other) const;
            bool operator!=(const Reference& other) const;
         private:
            Type m_type;
            int m_x;
            int m_y;
        };

        ScreenHistory(size_t sizeLimit);

        void push(Reference ref);
        Reference pop();

        void rotate();
        void clear();

        afl::base::Memory<const Reference> getAll() const;
        void applyMask(afl::base::Memory<const bool> mask);

     private:
        size_t m_sizeLimit;
        std::vector<Reference> m_data;  // ex screen_history
    };

}

#endif
