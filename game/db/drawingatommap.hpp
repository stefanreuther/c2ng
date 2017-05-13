/**
  *  \file game/db/drawingatommap.hpp
  */
#ifndef C2NG_GAME_DB_DRAWINGATOMMAP_HPP
#define C2NG_GAME_DB_DRAWINGATOMMAP_HPP

#include <map>
#include "util/atomtable.hpp"
#include "afl/base/types.hpp"
#include "afl/io/stream.hpp"
#include "afl/charset/charset.hpp"

namespace game { namespace db {

    // /** Drawing Tag Map for Save/Reload. Markers have an associated
    //     numeric tag, which can be an atom. The scripting interface
    //     guarantees that these atoms survive save/reload. This class
    //     supports save/reload by providing a mapping between the internal
    //     atom value and the externally-stored one. This class can also help
    //     in generating a list of used tags for filtering or drawing.
 
    //     Note that PCC 1.x always saves the internal atom numbers and
    //     remaps only on reload. We also remap on save, to compact our
    //     potentially larger range of atoms. */
    class DrawingAtomMap {
     public:
        DrawingAtomMap();
        ~DrawingAtomMap();

        void clear();
        bool isEmpty() const;

        void add(util::Atom_t a);

        util::Atom_t get(uint16_t value) const;

        void save(afl::io::Stream& out, afl::charset::Charset& cs, const util::AtomTable& table) const;
        void load(afl::io::Stream& in, afl::charset::Charset& cs, util::AtomTable& table);

     private:
        typedef std::map<util::Atom_t, uint16_t> Map_t;

        Map_t m_atoms;
        uint16_t m_counter;

        enum { EXTERNAL_ATOM_MIN = 20000 };
 // public:
 //    typedef member_iterator<map_t::const_iterator,
 //                            const atom_t,
 //                            &map_t::const_iterator::value_type::first> iterator;


 //    void add(atom_t a);

 //    void save(Stream& out);
 //    void load(Stream& in);

 //    iterator begin() const;
 //    iterator end() const;

 //    int16 getExternalValue(atom_t atom) const;
        
    };

} }

#endif
