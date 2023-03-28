/**
  *  \file game/interface/selectionfunctions.cpp
  *  \brief Selection I/O Functions
  */

#include "game/interface/selectionfunctions.hpp"
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
#include "game/map/selections.hpp"
#include "game/turn.hpp"
#include "interpreter/genericvalue.hpp"
#include "interpreter/values.hpp"
#include "util/translation.hpp"

using afl::string::Format;
using game::actions::mustExist;
using game::map::SelectionVector;
using game::map::Selections;
using interpreter::Error;

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
     *
     *  The file header uses the format "CCsel0 <timestamp> <layers>".
     *  For now, we use layers=1 or layers=8 to mean "one layer" or
     *  "all layers", independant of NUM_LAYERS.
     */
    enum Flag {
        TimelessFlag,                 ///< Accept files with mismatching timestamp.
        AcceptAllFlag,                ///< Accept files containing all selections.
        AcceptSingleFlag,             ///< Accept files containing a single selection.
        AcceptCurrentFlag,            ///< Accept files from the current turn.
        MergeFlag                     ///< Merge instead of replacing the current selection.
    };
    typedef afl::bits::SmallSet<Flag> Flags_t;

    /* State. Used as opaque state value on script side. */
    struct State : public afl::base::RefCounted {
        size_t fd;                    ///< User-provided fd.
        size_t targetLayer;           ///< Target layer.
        Flags_t targetFlags;          ///< Target flags (required criteria; from user-specified options).
        Flags_t fileFlags;            ///< File flags (available criteria).
        String_t fileTime;            ///< File timestamp.
        bool useUI;                   ///< UI flag (from user-specified options).
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
    bool readSelection(afl::io::TextFile& tf, Selections& sel, const game::map::Universe& univ)
    {
        // ex game/selio.cc:readSelection, search.pas:LoadSelectionFromFile
        const size_t NUM_LAYERS = sel.getNumLayers();
        const int MASK_LIMIT = (1 << NUM_LAYERS);
        String_t line;
        while (tf.readLine(line)) {
            if (!line.empty()) {
                // Classify line
                Selections::Kind k;
                int limit;
                if (line[0] == '}') {
                    return true;
                } else if (line[0] == 'P' || line[0] == 'p') {
                    k = Selections::Planet;
                    limit = univ.planets().size();
                } else if (line[0] == 'S' || line[0] == 's') {
                    k = Selections::Ship;
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
                if (!afl::string::strToInteger(line.substr(n+1), mask) || mask < 0 || mask >= MASK_LIMIT) {
                    return false;
                }

                // Mark it
                for (size_t i = 0; i < NUM_LAYERS; ++i) {
                    if (mask & (1 << i)) {
                        if (SelectionVector* pv = sel.get(k, i)) {
                            pv->set(id, true);
                        }
                    }
                }
            }
        }
        return true;
    }

    /** Copy or merge a selection layer from one Selections object into another.
        \param [out] out       Target selections
        \param [out] outLayer  Layer in target selections
        \param [in]  in        Source selections
        \param [in]  inLayer   Layer in source selections
        \param [in]  merge     true to merge, false to copy/overwrite
        \param [in]  kind      Selection kind to work on */
    void mergeSelections(Selections& out, size_t outLayer, const Selections& in, size_t inLayer, bool merge, Selections::Kind kind)
    {
        const SelectionVector* inVec = in.get(kind, inLayer);
        SelectionVector* outVec = out.get(kind, outLayer);
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

    /** Get selection mask (value to save) for an object.
        \param sel    Selections
        \param k      Selection kind (object type) to deal with
        \param id     Object Id
        \param layer  User-specified layer selection */
    int getSelectionMask(const Selections& sel, Selections::Kind k, game::Id_t id, int layer)
    {
        if (layer < 0) {
            // All layers
            int result = 0;
            for (size_t i = 0, n = sel.getNumLayers(); i < n; ++i) {
                if (const SelectionVector* p = sel.get(k, i)) {
                    if (p->get(id)) {
                        result |= 1 << i;
                    }
                }
            }
            return result;
        } else {
            // One layer
            if (const SelectionVector* p = sel.get(k, static_cast<size_t>(layer))) {
                return p->get(id);
            } else {
                return 0;
            }
        }
    }

    /** Save selection.
        Writes the entire file.
        \param tf       File
        \param g        Game
        \param layer    User-specified layer selection
        \param timeless true to make a timeless file */
    void saveSelection(afl::io::TextFile& tf, game::Game& g, int layer, bool timeless)
    {
        // ex search.pas:ScriptSaveSelection
        // Make sure selection is consistent with universe
        // g/t/univ are mutable, because copyFrom requires a mutable universe, because it uses ObjectType::getObjectByIndex() which is mutable.
        game::Turn& t = mustExist(g.getViewpointTurn().get());
        game::map::Universe& univ = t.universe();
        g.selections().copyFrom(univ, g.selections().getCurrentLayer());

        // Build header
        String_t header = "CCsel0 ";
        if (timeless) {
            header += "-";
        } else {
            header += t.getTimestamp().getTimestampAsString();
        }
        if (layer < 0) {
            header += " 8";
        } else {
            header += " 1";
        }
        tf.writeLine(header);

        // Write file
        for (game::Id_t sid = 1, n = univ.ships().size(); sid <= n; ++sid) {
            if (int val = getSelectionMask(g.selections(), Selections::Ship, sid, layer)) {
                tf.writeLine(Format("s%d %d", sid, val));
            }
        }
        for (game::Id_t pid = 1, n = univ.planets().size(); pid <= n; ++pid) {
            if (int val = getSelectionMask(g.selections(), Selections::Planet, pid, layer)) {
                tf.writeLine(Format("p%d %d", pid, val));
            }
        }
    }
}

/* @q CC$SelReadHeader(file:File, flags:Str):Any (Internal)
   Read selection file header and prepare a state.
   Returns the state.
   @since PCC2 2.40.6 */
afl::data::Value*
game::interface::IFCCSelReadHeader(Session& session, interpreter::Arguments& args)
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
    int32_t userLayer = static_cast<int32_t>(g.selections().getCurrentLayer());
    interpreter::checkFlagArg(userFlags, &userLayer, args.getNext(), "TAMU");

    // Convert layer
    size_t targetLayer = static_cast<size_t>(userLayer);
    if (targetLayer >= g.selections().getNumLayers()) {
        throw Error::rangeError();
    }

    // Convert flags
    Flags_t targetFlags;
    bool useUI = false;
    if ((userFlags & 4) != 0) {
        // M = Merge
        targetFlags += MergeFlag;
    }
    if ((userFlags & 8) != 0) {
        // U - User interaction
        useUI = true;
    } else {
        // Not user-interface.
        if ((userFlags & 1) != 0) {
            targetFlags += TimelessFlag;
        }
        if ((userFlags & 2) != 0) {
            targetFlags += AcceptAllFlag;
        }
        targetFlags += AcceptCurrentFlag;
        targetFlags += AcceptSingleFlag;
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
        fileFlags += AcceptSingleFlag;
    } else if (fileLayer == "8") {
        fileFlags += AcceptAllFlag;
    } else {
        throw afl::except::FileFormatException(*tf, session.translator()("Invalid layer count"));
    }

    // - time
    String_t fileTime = afl::string::strNthWord(header, 1);
    if (fileTime != "-" && fileTime != mustExist(g.getViewpointTurn().get()).getTimestamp().getTimestampAsString()) {
        fileFlags += TimelessFlag;
    } else {
        fileFlags += AcceptCurrentFlag;
    }

    // If no UI requested, and file does not match, bail out now
    if (!useUI) {
        if ((targetFlags & fileFlags) != fileFlags) {
            // Using 'Error' here because those are not translated, in case anyone wants to test the text.
            if (fileFlags.contains(TimelessFlag)) {
                throw Error("Stale file");
            } else {
                throw Error("File doesn't match requested content");
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

/* @q CC$SelReadContent(Obj:Any):void (Internal)
   Read selection file content according to given state.
   Returns the state.
   @since PCC2 2.40.6 */
afl::data::Value*
game::interface::IFCCSelReadContent(Session& session, interpreter::Arguments& args)
{
    // ex loadSelection (part)
    Game& g = game::actions::mustHaveGame(session);
    args.checkArgumentCount(1);
    StateValue_t* sv = dynamic_cast<StateValue_t*>(args.getNext());
    if (sv == 0) {
        throw Error::typeError(Error::ExpectNone);
    }
    State& st = *sv->get();

    // Text file
    afl::io::TextFile* tf = session.world().fileTable().getFile(st.fd);
    if (tf == 0) {
        throw Error("File not open");
    }

    // Read it
    Selections& result = g.selections();
    game::map::Universe& univ = mustExist(g.getViewpointTurn().get()).universe();
    Selections tmp;
    if (!readSelection(*tf, tmp, univ)) {
        throw afl::except::FileFormatException(*tf, session.translator()("File format error"));
    }

    // Assimilate into main database
    size_t nlayers, firstlayer;
    if (st.fileFlags.contains(AcceptAllFlag)) {
        nlayers    = result.getNumLayers();
        firstlayer = 0;
    } else {
        nlayers    = 1;
        firstlayer = st.targetLayer;
    }

    bool merge = st.targetFlags.contains(MergeFlag);
    result.copyFrom(univ, result.getCurrentLayer());
    for (size_t L = 0; L < nlayers; ++L) {
        mergeSelections(result, firstlayer + L, tmp, L, merge, Selections::Ship);
        mergeSelections(result, firstlayer + L, tmp, L, merge, Selections::Planet);
        result.limitToExistingObjects(univ, firstlayer + L);
    }

    // In any case, this operation has caused the main selection to be changed, so update everything.
    result.copyTo(univ, result.getCurrentLayer());
    result.sig_selectionChange.raise();
    return 0;
}

/* @q CC$SelGetQuestion(obj:Any):Str (Internal)
   If the selection state needs us to ask any questions, return the question text.
   @since PCC2 2.40.6 */
afl::data::Value*
game::interface::IFCCSelGetQuestion(Session& session, interpreter::Arguments& args)
{
    args.checkArgumentCount(1);
    StateValue_t* sv = dynamic_cast<StateValue_t*>(args.getNext());
    if (sv == 0) {
        throw Error::typeError(Error::ExpectNone);
    }
    State& st = *sv->get();

    const char* result = 0;
    if (st.useUI && (st.targetFlags & st.fileFlags) != st.fileFlags) {
        if (st.fileTime != "-") {
            if (st.fileFlags.contains(AcceptCurrentFlag)) {
                result = st.fileFlags.contains(AcceptAllFlag)
                    ? N_("File contains all selections from current turn")
                    : N_("File contains one selection from current turn");
            } else {
                result = st.fileFlags.contains(AcceptAllFlag)
                    ? N_("File contains all selections from a different turn; timestamp is %s")
                    : N_("File contains one selection from a different turn; timestamp is %s");
            }
        } else {
            result = st.fileFlags.contains(AcceptAllFlag)
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
game::interface::IFSelectionSave(Session& session, interpreter::Process& /*proc*/, interpreter::Arguments& args)
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
    if (layer < -1 || layer >= int32_t(g.selections().getNumLayers())) {
        throw Error::rangeError();
    }

    saveSelection(*tf, g, layer, flags != 0);
}
