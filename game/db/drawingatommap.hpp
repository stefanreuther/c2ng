/**
  *  \file game/db/drawingatommap.hpp
  *  \brief Class game::db::DrawingAtomMap
  */
#ifndef C2NG_GAME_DB_DRAWINGATOMMAP_HPP
#define C2NG_GAME_DB_DRAWINGATOMMAP_HPP

#include <map>
#include "afl/base/types.hpp"
#include "afl/charset/charset.hpp"
#include "afl/io/stream.hpp"
#include "afl/string/translator.hpp"
#include "afl/sys/loglistener.hpp"
#include "util/atomtable.hpp"

namespace game { namespace db {

    /** Drawing Tag/Atom Map for Save/Reload.
        Markers have an associated numeric tag, which can be an atom.
        The scripting interface guarantees that these atoms survive save/reload.
        This class supports save/reload by providing a mapping between the internal atom value and the externally-stored one.

        Note that PCC 1.x always saves the internal atom numbers and remaps only on reload.
        We also remap on save, to compact our potentially larger range of atoms. */
    class DrawingAtomMap {
     public:
        /** Create blank map. */
        DrawingAtomMap();
        ~DrawingAtomMap();

        /** Clear this map.
            The object will behave as if it had just been constructed.
            \post isEmpty() */
        void clear();

        /** Check for emptiness.
            \return true if map is empty */
        bool isEmpty() const;

        /** Add new internal atom.
            Registers that \c a is used as an atom by a relevant drawing.
            \param a atom */
        void add(util::Atom_t a);

        /** Convert external atom to internal.
            \param value Value read from file
            \return equivalent atom to use internally. If a remapping is known, it is applied,
                    otherwise, the value is used as-is. */
        util::Atom_t get(uint16_t value) const;

        /** Convert internal atom to external.
            \param atom Value stored in Drawing object
            \return equivalent value to store in file. If a remapping is known (by previous add() call), it is applied,
                    otherwise, the value is used as-is. */
        uint16_t getExternalValue(util::Atom_t atom) const;

        /** Save object to stream.
            This is used to create the rPaintingTags (11) block in chartX.cc.

            \param out   Stream
            \param cs    Game character set
            \param table Atom table
            \param log   Logger, to log warning messages if needed
            \param tx    Translator */
        void save(afl::io::Stream& out, afl::charset::Charset& cs, const util::AtomTable& table, afl::sys::LogListener& log, afl::string::Translator& tx) const;

        /** Load object from stream.

            \param in    Stream
            \param cs    Game character set
            \param table Atom table, will be updated */
        void load(afl::io::Stream& in, afl::charset::Charset& cs, util::AtomTable& table);

     private:
        typedef std::map<util::Atom_t, uint16_t> Map_t;

        Map_t m_atoms;
        uint16_t m_counter;
    };

} }

#endif
