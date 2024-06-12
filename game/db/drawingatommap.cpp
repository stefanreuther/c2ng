/**
  *  \file game/db/drawingatommap.cpp
  *  \brief Class game::db::DrawingAtomMap
  */

#include "game/db/drawingatommap.hpp"
#include "afl/base/growablememory.hpp"
#include "afl/string/format.hpp"
#include "game/db/structures.hpp"
#include "util/io.hpp"

namespace {
    const char*const LOG_NAME = "game.db";

    /** Start value for allocation of external atoms. */
    const uint16_t EXTERNAL_ATOM_MIN = 20000;
}

// Create blank map.
game::db::DrawingAtomMap::DrawingAtomMap()
    : m_atoms(),
      m_counter(EXTERNAL_ATOM_MIN)
{
    // ex GDrawingAtomMap::GDrawingAtomMap
}

game::db::DrawingAtomMap::~DrawingAtomMap()
{ }


// Clear this map.
void
game::db::DrawingAtomMap::clear()
{
    // ex GDrawingAtomMap::clear
    Map_t().swap(m_atoms);
    m_counter = EXTERNAL_ATOM_MIN;
}

// Check for emptiness.
bool
game::db::DrawingAtomMap::isEmpty() const
{
    // ex GDrawingAtomMap::isEmpty
    return m_counter == EXTERNAL_ATOM_MIN;
}

// Add new internal atom.
void
game::db::DrawingAtomMap::add(util::Atom_t a)
{
    // ex GDrawingAtomMap::add
    if (m_atoms.find(a) == m_atoms.end()) {
        m_atoms[a] = m_counter++;
    }
}

// Convert external atom to internal.
util::Atom_t
game::db::DrawingAtomMap::get(uint16_t value) const
{
    // ex GDrawingAtomMap::get
    for (Map_t::const_iterator i = m_atoms.begin(); i != m_atoms.end(); ++i) {
        if (i->second == value) {
            return i->first;
        }
    }
    return util::Atom_t(value);
}

// Convert internal atom to external.
uint16_t
game::db::DrawingAtomMap::getExternalValue(util::Atom_t atom) const
{
    // ex GDrawingAtomMap::getExternalValue
    Map_t::const_iterator i = m_atoms.find(atom);
    if (i != m_atoms.end()) {
        return i->second;
    }
    return int16_t(atom);
}

// Save object to stream.
void
game::db::DrawingAtomMap::save(afl::io::Stream& out, afl::charset::Charset& cs, const util::AtomTable& table,
                               afl::sys::LogListener& log, afl::string::Translator& tx) const
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
            log.write(afl::sys::LogListener::Warn, LOG_NAME, tx("Too many different marker tags. Some were ignored."));
            break;
        }
        tmp = i->second;
        valueBuffer.append(tmp);
    }

    // write it out
    tmp = uint16_t(valueBuffer.size());
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
        log.write(afl::sys::LogListener::Warn, LOG_NAME,
                  afl::string::Format(tx("%d marker tag%!1{s were%| was%} too long to be stored in the chart file, and truncated to 255 characters."),
                                      failed_strings));
    }
}

// Load object from stream.
void
game::db::DrawingAtomMap::load(afl::io::Stream& in, afl::charset::Charset& cs, util::AtomTable& table)
{
    // ex GDrawingAtomMap::load
    clear();

    // load 'count' field
    structures::UInt16_t count;
    in.fullRead(count.m_bytes);

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
