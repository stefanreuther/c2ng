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
                 HistoryScreen (PCC2, PCC1)
                 FleetScreen (PCC2, PCC1)
                 Search Result (PCC1)
                 Message (PCC1)
             */
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
