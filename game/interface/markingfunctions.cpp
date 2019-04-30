/**
  *  \file game/interface/markingfunctions.cpp
  */

#include "game/interface/markingfunctions.hpp"
#include "afl/base/refcounted.hpp"
#include "afl/bits/smallset.hpp"
#include "afl/except/assertionfailedexception.hpp"
#include "afl/except/fileformatexception.hpp"
#include "afl/except/filetooshortexception.hpp"
#include "afl/string/format.hpp"
#include "afl/string/parse.hpp"
#include "afl/test/assert.hpp"
#include "game/actions/preconditions.hpp"
#include "game/game.hpp"
#include "game/map/markings.hpp"
#include "game/turn.hpp"
#include "interpreter/genericvalue.hpp"
#include "interpreter/values.hpp"
#include "util/translation.hpp"

using afl::string::Format;
using game::map::Markings;
using game::map::MarkingVector;

namespace {
    /*
     *  Selection Loading
     *
     *  The SelectionLoad command is rather complex in PCC1/PCC2: the
     *  'u' option gives an optional user interface that is intermixed
     *  with the actual loading. For c2ng, we split the function into
     *  three parts:
     *
     *     CC$SelReadHeader - open file, read header, stash away state
     *     CC$SelGetQuestion - determine question to ask user
     *     CC$SelReadContent - read content
     *
     *  The actual SelectionLoad command is implemented in core_game.q.
     *
     *  The logic is mostly taken from PCC2 and not much polished.
     */
    enum Flag {
        /** Timeless behaviour.
            - saveSelection: create a timeless file which will always be accepted as current by loadSelection
            - loadSelection: accept files with mismatching timestamp */
        selio_Timeless,
        /** Accept files containing all selections.
            Handled by loadSelection only. */
        selio_AcceptAll,
        /** Accept files containing a single selection.
            Handled by loadSelection only. */
        selio_AcceptSingle,
        /** Accept files from the current turn.
            Handled by loadSelection only. */
        selio_AcceptCurrent,
        /** Merge instead of replacing the current selection.
            Handled by loadSelection only. */
        selio_Merge
    };
    typedef afl::bits::SmallSet<Flag> Flags_t;

    struct State : public afl::base::RefCounted {
        size_t fd;
        size_t targetLayer;
        Flags_t targetFlags;
        Flags_t fileFlags;
        String_t fileTime;
        bool useUI;
    };
    typedef afl::base::Ref<State> StateRef_t;
    typedef interpreter::GenericValue<StateRef_t> StateValue_t;

    /** Read selection from file.
        Corresponds roughly to search.cpp::LoadSelectionFromFile.
        \param tf Text file to read from
        \param sel Read into this object
        \param univ Universe to determine Id limits (DoS protection)
        \retval true success
        \retval false file format error */
    bool readSelection(afl::io::TextFile& tf, Markings& sel, const game::map::Universe& univ)
    {
        // ex game/selio.cc:readSelection
        const size_t NUM_LAYERS = sel.getNumLayers();
        const int MASK = (1 << NUM_LAYERS);
        String_t line;
        while (tf.readLine(line)) {
            if (!line.empty()) {
                // Classify line
                Markings::Kind k;
                int limit;
                if (line[0] == '}') {
                    return true;
                } else if (line[0] == 'P' || line[0] == 'p') {
                    k = Markings::Planet;
                    limit = univ.planets().size();
                } else if (line[0] == 'S' || line[0] == 's') {
                    k = Markings::Ship;
                    limit = univ.ships().size();
                } else {
                    return false;
                }

                // Parse line
                String_t::size_type n = line.find(' ');
                if (n == String_t::npos) {
                    return false;
                }
                int id = 0;
                if (!afl::string::strToInteger(line.substr(1, n-1), id) || id <= 0 || id > limit) {
                    return false;
                }

                int mask;
                if (!afl::string::strToInteger(line.substr(n+1), mask) || mask < 0 || mask >= MASK) {
                    return false;
                }

                // Mark it
                for (size_t i = 0; i < NUM_LAYERS; ++i) {
                    if (mask & (1 << i)) {
                        if (MarkingVector* pv = sel.get(k, i)) {
                            pv->set(id, true);
                        }
                    }
                }
            }
        }
        return true;
    }

    void mergeSelections(Markings& out, size_t outLayer,
                         Markings& in, size_t inLayer,
                         bool merge, Markings::Kind kind)
    {
        MarkingVector* inVec = in.get(kind, inLayer);
        MarkingVector* outVec = out.get(kind, outLayer);
        if (inVec != 0 && outVec != 0) {
            if (!merge) {
                outVec->clear();
            }
            outVec->mergeFrom(*inVec);
        }
    }

    /*
     *  Selection Saving
     */

    int getSelectionMask(Markings& sel, Markings::Kind k, game::Id_t id, int layer)
    {
        if (layer < 0) {
            // All layers
            int result = 0;
            for (size_t i = 0, n = sel.getNumLayers(); i < n; ++i) {
                if (MarkingVector* p = sel.get(k, i)) {
                    if (p->get(id)) {
                        result |= 1 << i;
                    }
                }
            }
            return result;
        } else {
            // One layer
            if (MarkingVector* p = sel.get(k, static_cast<size_t>(layer))) {
                return p->get(id);
            } else {
                return 0;
            }
        }
    }

    
    void saveSelection(afl::io::TextFile& tf, game::Game& g, int layer, bool timeless)
    {
        /* Make sure selection is consistent with universe */
        game::map::Universe& univ = g.currentTurn().universe();
        g.markings().copyFrom(univ, g.markings().getCurrentLayer());

        // Build header
        String_t header = "CCsel0 ";
        if (timeless) {
            header += "-";
        } else {
            header += g.currentTurn().getTimestamp().getTimestampAsString();
        }
        if (layer < 0) {
            header += " 8";
        } else {
            header += " 1";
        }
        tf.writeLine(header);

        // Write file
        for (game::Id_t sid = 1, n = univ.ships().size(); sid <= n; ++sid) {
            if (int val = getSelectionMask(g.markings(), Markings::Ship, sid, layer)) {
                tf.writeLine(Format("s%d %d", sid, val));
            }
        }
        for (game::Id_t pid = 1, n = univ.planets().size(); pid <= n; ++pid) {
            if (int val = getSelectionMask(g.markings(), Markings::Planet, pid, layer)) {
                tf.writeLine(Format("p%d %d", pid, val));
            }
        }

    }
}


/* @q CC$SelReadHeader(file:File, flags:Str):Any (Internal Function)
   Read selection file header and prepare a state.
   Returns the state.
   @since PCC2 2.40.6 */
afl::data::Value*
game::interface::IFCCSelReadHeader(game::Session& session, interpreter::Arguments& args)
{
    // ex IFSelectionLoad (part), loadSelection
    Game& g = game::actions::mustHaveGame(session);
    args.checkArgumentCount(1, 2);

    // File is mandatory
    size_t fd;
    if (!session.world().fileTable().checkFileArg(fd, args.getNext(), true)) {
        return 0;
    }

    // Flags are optional
    int32_t userFlags = 0;
    int32_t userLayer = static_cast<int32_t>(g.markings().getCurrentLayer());
    interpreter::checkFlagArg(userFlags, &userLayer, args.getNext(), "TAMU");

    // Convert layer
    size_t targetLayer = static_cast<size_t>(userLayer);
    if (targetLayer >= g.markings().getNumLayers()) {
        throw interpreter::Error::rangeError();
    }

    // Convert flags
    Flags_t targetFlags;
    bool useUI = false;
    if ((userFlags & 4) != 0) {
        // M = Merge
        targetFlags += selio_Merge;
    }
    if ((userFlags & 8) != 0) {
        // U - User interaction
        useUI = true;
    } else {
        // Not user-interface.
        if ((userFlags & 1) != 0) {
            targetFlags += selio_Timeless;
        }
        if ((userFlags & 2) != 0) {
            targetFlags += selio_AcceptAll;
        }
        targetFlags += selio_AcceptCurrent;
        targetFlags += selio_AcceptSingle;
    }

    // Read file
    afl::io::TextFile* tf = session.world().fileTable().getFile(fd);
    afl::except::checkAssertion(tf, "file not open");

    // - header line
    String_t header;
    if (!tf->readLine(header)) {
        throw afl::except::FileTooShortException(*tf);
    }
    if (afl::string::strNthWord(header, 0) != "CCsel0") {
        throw afl::except::FileFormatException(*tf, session.translator()("File is missing required signature"));
    }

    // - layers
    Flags_t fileFlags;
    String_t fileLayer = afl::string::strNthWord(header, 2);
    if (fileLayer == "1") {
        fileFlags += selio_AcceptSingle;
    } else if (fileLayer == "8") {
        fileFlags += selio_AcceptAll;
    } else {
        throw afl::except::FileFormatException(*tf, session.translator()("Invalid layer count"));
    }

    // - time
    String_t fileTime = afl::string::strNthWord(header, 1);
    if (fileTime != "-" && fileTime != g.currentTurn().getTimestamp().getTimestampAsString()) {
        fileFlags += selio_Timeless;
    } else {
        fileFlags += selio_AcceptCurrent;
    }

    // If no UI requested, and file does not match, bail out now
    if (!useUI) {
        if ((targetFlags & fileFlags) != fileFlags) {
            if (fileFlags.contains(selio_Timeless)) {
                throw interpreter::Error("Stale file");
            } else {
                throw interpreter::Error("File doesn't match requested content");
            }
        }
    }

    // Build result
    StateRef_t result(*new State());
    result->fd = fd;
    result->targetLayer = targetLayer;
    result->targetFlags = targetFlags;
    result->fileFlags = fileFlags;
    result->fileTime = fileTime;
    result->useUI = useUI;
    return new StateValue_t(result);
}

/* @q CC$SelReadContent(Obj:Any):void (Internal Function)
   Read selection file content according to given state.
   Returns the state.
   @since PCC2 2.40.6 */
afl::data::Value*
game::interface::IFCCSelReadContent(game::Session& session, interpreter::Arguments& args)
{
    // ex loadSelection (part)
    Game& g = game::actions::mustHaveGame(session);
    args.checkArgumentCount(1);
    StateValue_t* sv = dynamic_cast<StateValue_t*>(args.getNext());
    if (sv == 0) {
        throw interpreter::Error::typeError(interpreter::Error::ExpectNone);
    }
    State& st = *sv->get();

    // Text file
    afl::io::TextFile* tf = session.world().fileTable().getFile(st.fd);
    if (tf == 0) {
        throw interpreter::Error("File not open");
    }

    // Read it
    Markings& result = g.markings();
    game::map::Universe& univ = g.currentTurn().universe();
    Markings tmp;
    if (!readSelection(*tf, tmp, univ)) {
        throw afl::except::FileFormatException(*tf, session.translator()("File format error"));
    }


    // Assimilate into main database
    size_t nlayers, firstlayer;
    if (st.fileFlags.contains(selio_AcceptAll)) {
        nlayers    = result.getNumLayers();
        firstlayer = 0;
    } else {
        nlayers    = 1;
        firstlayer = st.targetLayer;
    }

    bool merge = st.targetFlags.contains(selio_Merge);
    for (size_t L = 0; L < nlayers; ++L) {
        mergeSelections(result, firstlayer + L, tmp, L, merge, Markings::Ship);
        mergeSelections(result, firstlayer + L, tmp, L, merge, Markings::Planet);
        result.limitToExistingObjects(univ, firstlayer + L);
    }

    // In any case, this operation has caused the main selection to be changed, so update everything.
    result.copyTo(univ, result.getCurrentLayer());
    result.sig_selectionChange.raise();
    return 0;
}

/* @q CC$SelGetQuestion(obj:Any):Str (Internal Function)
   If the selection state needs us to ask any questions, return the question text.
   @since PCC2 2.40.6 */
afl::data::Value*
game::interface::IFCCSelGetQuestion(game::Session& session, interpreter::Arguments& args)
{
    args.checkArgumentCount(1);
    StateValue_t* sv = dynamic_cast<StateValue_t*>(args.getNext());
    if (sv == 0) {
        throw interpreter::Error::typeError(interpreter::Error::ExpectNone);
    }
    State& st = *sv->get();

    const char* result = 0;
    if (st.useUI && (st.targetFlags & st.fileFlags) != st.fileFlags) {
        if (st.fileTime != "-") {
            if (st.fileFlags.contains(selio_AcceptCurrent)) {
                result = st.fileFlags.contains(selio_AcceptAll)
                    ? N_("File contains all selections from current turn")
                    : N_("File contains one selection from current turn");
            } else {
                result = st.fileFlags.contains(selio_AcceptAll)
                    ? N_("File contains all selections from a different turn; timestamp is %s")
                    : N_("File contains one selection from a different turn; timestamp is %s");
            }
        } else {
            result = st.fileFlags.contains(selio_AcceptAll)
                ? N_("File contains all selections")
                : N_("File contains one selection");
        }
    }

    if (result) {
        return interpreter::makeStringValue(Format(session.translator()(result), st.fileTime));
    } else {
        return 0;
    }
}




/* @q SelectionSave file:File, Optional flags:Str (Global Command)
   Save selection into file.

   The %flags argument is a combination of the following options:
   - %t ("timeless") to create a file without timestamp that can be loaded in any turn
   - a selection layer number to save just that layer (default: all)
   @see SelectionLoad, Selection.Layer
   @since PCC 1.1.3, PCC2 1.99.13, PCC2 2.40.6 */
void
game::interface::IFSelectionSave(interpreter::Process& /*proc*/, game::Session& session, interpreter::Arguments& args)
{
    // SelectionSave "f[T#]"
    Game& g = game::actions::mustHaveGame(session);
    
    afl::io::TextFile* tf = 0;
    int32_t flags = 0;
    int32_t layer = -1;

    args.checkArgumentCount(1, 2);
    if (!session.world().fileTable().checkFileArg(tf, args.getNext())) {
        return;
    }
    interpreter::checkFlagArg(flags, &layer, args.getNext(), "T");
    if (layer < -1 || layer >= int32_t(g.markings().getNumLayers())) {
        throw interpreter::Error::rangeError();
    }

    saveSelection(*tf, g, layer, flags != 0);
}
