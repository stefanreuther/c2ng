/**
  *  \file game/db/drawingatommap.cpp
  */

#include "game/db/drawingatommap.hpp"
#include "game/db/structures.hpp"
#include "afl/base/growablememory.hpp"
#include "util/io.hpp"

// /** Create blank map. */
game::db::DrawingAtomMap::DrawingAtomMap()
    : m_atoms(),
      m_counter(EXTERNAL_ATOM_MIN)
{
    // ex GDrawingAtomMap::GDrawingAtomMap
}

game::db::DrawingAtomMap::~DrawingAtomMap()
{ }


// /** Clear this map. The object will behave as if it had just been constructed.
//     \post isEmpty() */
void
game::db::DrawingAtomMap::clear()
{
    // ex GDrawingAtomMap::clear
    Map_t().swap(m_atoms);
    m_counter = EXTERNAL_ATOM_MIN;
}

// /** Check for emptiness. */
bool
game::db::DrawingAtomMap::isEmpty() const
{
    // ex GDrawingAtomMap::isEmpty
    return m_counter == EXTERNAL_ATOM_MIN;
}

// /** Add new internal atom. Registers that \c a is used as an atom by a
//     relevant drawing. */
void
game::db::DrawingAtomMap::add(util::Atom_t a)
{
    // ex GDrawingAtomMap::add
    if (m_atoms.find(a) == m_atoms.end()) {
        m_atoms[a] = m_counter++;
    }
}

// /** Convert external atom to internal.
//     \param value Value read from file
//     \return equivalent atom to use internally */
util::Atom_t
game::db::DrawingAtomMap::get(uint16_t value) const
{
    // ex GDrawingAtomMap::get
    // FIXME: we can surely do better
    for (Map_t::const_iterator i = m_atoms.begin(); i != m_atoms.end(); ++i) {
        if (i->second == value) {
            return i->first;
        }
    }
    return util::Atom_t(value);
}

// /** Save object to stream. This is used to create the rPaintingTags
//     (11) record in chartX.cc. */
void
game::db::DrawingAtomMap::save(afl::io::Stream& out, afl::charset::Charset& cs, util::AtomTable& table) const
{
    // ex GDrawingAtomMap::save
    // build value array
    structures::UInt16_t tmp;
    afl::base::GrowableMemory<structures::UInt16_t> valueBuffer;
    for (Map_t::const_iterator i = m_atoms.begin(); i != m_atoms.end(); ++i) {
        if (valueBuffer.size() >= 16000) {
            // too big. PCC 1.x supports up to 64000 bytes for the whole
            // Atom Translation entry, and 16000 is a very optimistic
            // estimate how many fit in there.
            // FIXME: console.write(LOG_WARN, _("Too many different marker tags. Some were ignored."));
            break;
        }
        tmp = i->second;
        valueBuffer.append(tmp);
    }

    // write it out
    tmp = valueBuffer.size();
    out.fullWrite(tmp.m_bytes);
    out.fullWrite(valueBuffer.toBytes());

    int failed_strings = 0;
    for (Map_t::const_iterator i = m_atoms.begin(); i != m_atoms.end(); ++i) {
        const util::Atom_t internal_atom = i->first;
        if (!util::storePascalStringTruncate(out, table.getStringFromAtom(internal_atom), cs)) {
            ++failed_strings;
        }
    }

    if (failed_strings != 0) {
    //     FIXME -> console.write(LOG_WARN, format(_("%d marker tag%!1{s%} were too long to be stored in the chart file, and were truncated to 255 characters."),
    //                                    failed_strings));
    }
}

// /** Save object from stream. This is used to load the rPaintingTags
//     (11) record from chartX.cc. */
void
game::db::DrawingAtomMap::load(afl::io::Stream& in, afl::charset::Charset& cs, util::AtomTable& table)
{
    // ex GDrawingAtomMap::load
    // FIXME: this does not maintain /counter/ and will thus yield an inconsistent state.
    // Maybe it makes sense to move this into a separate class where we can also make
    // a decent get() function?
    clear();

    // load 'count' field
    structures::UInt16_t count;
    in.fullRead(count.m_bytes);
    // FIXME:
    // if (count.value < 0)
    //     throw FileFormatException(in, _("Invalid value"));

    // load value array
    afl::base::GrowableMemory<structures::UInt16_t> valueBuffer;
    valueBuffer.resize(count);
    in.fullRead(valueBuffer.toBytes());

    // load strings and populate array
    afl::base::Memory<const structures::UInt16_t> values(valueBuffer);
    while (const structures::UInt16_t* p = values.eat()) {
        uint16_t externalAtom = *p;
        util::Atom_t internalAtom = table.getAtomFromString(util::loadPascalString(in, cs));
        m_atoms[internalAtom] = externalAtom;
    }
}
