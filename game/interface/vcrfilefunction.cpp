/**
  *  \file game/interface/vcrfilefunction.cpp
  *  \brief Class game::interface::VcrFileFunction
  */

#include "game/interface/vcrfilefunction.hpp"

#include "afl/except/filetooshortexception.hpp"
#include "afl/io/stream.hpp"
#include "afl/io/textfile.hpp"
#include "game/actions/preconditions.hpp"
#include "game/interface/vcrcontext.hpp"
#include "game/vcr/classic/database.hpp"
#include "game/vcr/flak/database.hpp"
#include "interpreter/arguments.hpp"

using afl::base::Ref;
using afl::io::Stream;
using afl::io::TextFile;

namespace {
    /*
     *  File formats:
     *
     *           Classic                      FLAK
     *   0       Count_low       any          Magic     'F'
     *   1       Count_high      >=0          Magic     'L'
     *   2       Seed_low        any          Magic     'A'
     *   3       Seed_high       any          Magic     'K'
     *   4       Sig_low         any          Magic     'V'
     *   5       Sig_high        any          Magic     'C'
     *   6       Temp/cap_low    any          Magic     'R'
     *   7       Temp/cap_high   0 or 0x80    Magic     26
     *   8       Type_low        0-1          Version   0
     *   9       Type_high       0            Version   0
     *
     *  We want a 99.9% reliable way to distinguish VCR vs. FLAK vs. other.
     *  Whereas FLAK has a magic number, almost any bytestream could be a VCR in theory.
     *  This check wields out most text and binary files.
     *  As most notable mis-identification, it identifies a bdataX.dat file as VCR.
     *  This could be avoided by checking number of available bytes (for a good VCR file, 100*N+2 .. 100*N+12),
     *  but that would no longer allow reading directly from RSTs, for example.
     */
    const size_t LOOKAHEAD = 10;

    bool isClassicVcr(uint8_t (&bytes)[LOOKAHEAD])
    {
        return bytes[1] < 0x80
            && (bytes[7] == 0 || bytes[7] == 0x80)
            && (bytes[8] == 0 || bytes[8] == 1)
            && bytes[9] == 0;
    }
}

game::interface::VcrFileFunction*
game::interface::VcrFileFunction::create(Session& session, afl::base::Ref<game::vcr::Database> db)
{
    if (db->getNumBattles() != 0) {
        return new VcrFileFunction(session, db);
    } else {
        return 0;
    }
}

inline
game::interface::VcrFileFunction::VcrFileFunction(Session& session, const afl::base::Ref<game::vcr::Database>& db)
    : m_session(session),
      m_battles(db)
{ }

game::interface::VcrContext*
game::interface::VcrFileFunction::get(interpreter::Arguments& args)
{
    args.checkArgumentCount(1);
    size_t i;
    if (!interpreter::checkIndexArg(i, args.getNext(), 1, getNumBattles())) {
        return 0;
    } else {
        return VcrContext::create(i, m_session, m_battles.asPtr());
    }
}

void
game::interface::VcrFileFunction::set(interpreter::Arguments& args, const afl::data::Value* value)
{
    rejectSet(args, value);
}

// CallableValue:
size_t
game::interface::VcrFileFunction::getDimension(size_t which) const
{
    if (which == 0) {
        return 1;
    } else {
        return getNumBattles() + 1;
    }
}

game::interface::VcrContext*
game::interface::VcrFileFunction::makeFirstContext()
{
    return VcrContext::create(0, m_session, m_battles.asPtr());
}

game::interface::VcrFileFunction*
game::interface::VcrFileFunction::clone() const
{
    return new VcrFileFunction(m_session, m_battles);
}

// BaseValue:
String_t
game::interface::VcrFileFunction::toString(bool /*readable*/) const
{
    return "#<array:VcrFile>";
}

void
game::interface::VcrFileFunction::store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const
{
    rejectStore(out, aux, ctx);
}

inline size_t
game::interface::VcrFileFunction::getNumBattles() const
{
    return m_battles->getNumBattles();
}

/* @q VcrFile(#fd:File):Obj (Function)
   Load VCRs from a file, and access their properties.

   If the file contains valid combat recordings, returns an array of those.
   If the file is empty, returns EMPTY.
   If the file does not have a valid format, reports an error.
   | Try
   |   Dim v = VcrFile(#3)
   |   If v Then
   |     ForEach v Do ...
   |   Else
   |     MessageBox "File is empty"
   |   EndIf
   | Else
   |   MessageBox "Invalid file"
   | EndIf

   The file pointer ({Seek}) must be at the beginning of the file.

   @see int:index:group:combatproperty|Combat Properties
   @since PCC2 2.41.4 */
afl::data::Value*
game::interface::IFVcrFile(game::Session& session, interpreter::Arguments& args)
{
    // Acquire file argument
    args.checkArgumentCount(1);
    TextFile* tf = 0;
    if (!session.world().fileTable().checkFileArg(tf, args.getNext())) {
        return 0;
    }

    // Acquire environment
    Root& root = game::actions::mustHaveRoot(session);

    // Read magic
    Stream::FileSize_t pos = tf->getPos();
    uint8_t bytes[LOOKAHEAD];
    size_t n = tf->read(bytes);
    tf->setPos(pos);

    // Verify
    if (n == 0 || (n >= 2 && bytes[0] == 0 && bytes[1] == 0)) {
        return 0;
    }
    if (n < LOOKAHEAD) {
        throw afl::except::FileTooShortException(*tf);
    }

    // Load
    if (isClassicVcr(bytes)) {
        Ref<game::vcr::classic::Database> db(*new game::vcr::classic::Database());
        db->load(*tf, root.hostConfiguration(), root.charset());
        return VcrFileFunction::create(session, db);
    } else {
        Ref<game::vcr::flak::Database> db(*new game::vcr::flak::Database());
        db->load(*tf, root.charset(), session.translator());
        return VcrFileFunction::create(session, db);
    }
}
