/**
  *  \file game/interface/globalproperty.cpp
  */

#include "game/interface/globalproperty.hpp"
#include "interpreter/values.hpp"
#include "game/root.hpp"
#include "interpreter/error.hpp"
#include "version.hpp"
#include "game/game.hpp"
#include "game/turn.hpp"
#include "interpreter/arguments.hpp"
#include "game/registrationkey.hpp"

using interpreter::makeStringValue;
using interpreter::makeIntegerValue;
using interpreter::makeBooleanValue;

afl::data::Value*
game::interface::getGlobalProperty(GlobalProperty igp, Session& session)
{
    // ex int/if/globalif.h:getGlobalProperty
    switch (igp) {
// FIXME: port (property)
//      case igpFileFormatLocal:
//         /* @q System.Local:Str (Global Property)
//            Local file format.
//            Reports the file format PCC uses to store your player files:
//            - <tt>"DOS"</tt> (same as planets.exe)
//            - <tt>"Windows"</tt> (same as Winplan) */

//         /* In PCC 1.x, this value determines the format of control and message outbox files.
//            In PCC2, those are orthogonal. We can, however, comparatively easily determine the mailbox format. */
        // FIXME: should get this value from TurnLoader
//         if (haveDisplayedTurn()) {
//             if (getCurrentTurn().getOutbox(getPlayerId()).getFormat() == GOutbox::fDos)
//                 return makeStringValue("DOS");
//             else
//                 return makeStringValue("Windows");
//         } else {
//             return 0;
//         }
     case igpFileFormatRemote:
        /* @q System.Remote:Str (Global Property)
           Remote file format.
           Reports the file format PCC uses for your turn files, i.e. what the "remote" host system sees:
           - <tt>"DOS"</tt> (same as planets.exe)
           - <tt>"Windows"</tt> (same as Winplan) */

        /* In PCC 1.x, this value is 0 for Dosplan, 1 for Winplan, and
           determines the TRN format. This is the config option. As of
           20110206, PCC2 has no such config option and always produces
           Winplan format. */
        // FIXME: should get this value from TurnLoader
        return makeStringValue("Windows");
     case igpGameDirectory:
        /* @q System.GameDirectory:Str (Global Property)
           Game directory. EMPTY when no game loaded.

           The game directory is the directory containing the current game's files.

           @diff In PCC 1.x, it is possible to concatenate this property with a file name
           to access a file in the game directory.
           This does no longer work in PCC2.
           Use the {MakeFileName} function, as in
           | Open MakeFileName(System.GameDirectory, "file.txt") For Input As #1
           to access files in the game directory.

           @diff In c2ng, this value may be the empty string "" if a game is loaded,
           but the game directory is a virtual directory (e.g. network game). */
        if (Root* root = session.getRoot().get()) {
            return makeStringValue(root->gameDirectory().getDirectoryName());
        } else {
            return 0;
        }
     case igpMyInMsgs:
        /* @q My.InMsgs:Int (Global Property)
           Number of incoming (received) messages this turn. */
        if (Game* game = session.getGame().get()) {
            return makeIntegerValue(game->currentTurn().inbox().getNumMessages());
        } else {
            return 0;
        }
// FIXME: port (property)
//      case igpMyOutMsgs:
//         /* @q My.OutMsgs:Int (Global Property)
//            Number of outgoing (sent) messages this turn. */
//         if (haveDisplayedTurn())
//             return makeIntValue(getDisplayedTurn().getOutbox(getPlayerId()).getCount());
//         else
//             return 0;
     case igpMyVCRs:
        /* @q My.VCRs:Int (Global Property)
           Number of incoming combat recordings this turn. */
        if (Game* game = session.getGame().get()) {
            if (game::vcr::Database* db = game->currentTurn().getBattles().get()) {
                return makeIntegerValue(db->getNumBattles());
            } else {
                return makeIntegerValue(0);
            }
        } else {
            return 0;
        }
// FIXME: port (property). Note that this is NOT specificationDirectory()!
//      case igpRootDirectory:
//         /* @q System.RootDirectory:Str (Global Property)
//            Root directory.

//            The root directory is the directory within the program installation directory
//            containing the default specification files.
//            If a specification file cannot be found in the {System.GameDirectory|game directory},
//            it is looked for in the root directory.
//            This directory typically is one of
//            - /usr/local/share/planets
//            - C:\Programs\PCC2\specs

//            @diff In PCC 1.x, it is possible to concatenate this property with a file name
//            to access a file in the root directory.
//            This does no longer work in PCC2.
//            Use the {MakeFileName} function, as in
//            | Open MakeFileName(System.RootDirectory, "file.txt") For Input As #1
//            to access files in the root directory. */
        // FIXME: should get this value from TurnLoader
//         if (root_dir_name.size())
//             return makeStringValue(root_dir_name);
//         else
//             return 0;
// FIXME: port (property)
//      case igpSelectionLayer:
//         /* @q Selection.Layer:Int (Global Property)
//            Current selection layer.
//            A number from 0 to 7.
//            @assignable */
//         return makeIntValue(GMultiSelection::getCurrentSelectionLayer());
// FIXME: port (property)
//      case igpSystemLanguage:
//         /* @q System.Language:Str (Global Property)
//            Language code.
//            This is the language the user wants to use,
//            usually in the form of a two-letter ISO 639 code ("en" = English).
//            @since PCC2 1.99.25 */
//         return makeStringValue(getLanguageCode());
     case igpSystemProgram:
        /* @q System.Program:Str (Global Property)
           Name of the program executing the script.
           Values in use so far:
           - <tt>"PCC"</tt> (PCC 1.x or PCC2 desktop application)
           - <tt>"PCC (Standalone Interpreter)"</tt> (PCC 1.x standalone interpreter) */
        return makeStringValue("PCC");
     case igpSystemVersion:
        /* @q System.Version:Str (Global Property)
           Version number of the program executing the script.
           For example, "1.1.18", or "1.99.20". */
        return makeStringValue(PCC2_VERSION);
     case igpSystemVersionCode:
        /* @q System.Version$:Int (Global Property)
           Version number of the program executing the script.
           The version number has three digits for the "patchlevel" field, and two digits for the "minor" field.
           For example, "101018" or "199020" for "1.1.18" and "1.99.20", respectively.

           Ranges are:
           - starting at 100000 for PCC 1.x (DOS version)
           - starting at 199000 for PCC2 (32-bit version)
           - starting at 204000 for c2ng

           @since PCC 1.0.14 */
        return makeIntegerValue(PCC2_VERSION_CODE);
     case igpSystemHost:
        /* @q System.Host:Str (Global Property)
           Name of Host program.
           Values in use so far:
           - <tt>"PHost"</tt>
           - <tt>"SRace"</tt>
           - <tt>"Host"</tt>
           - <tt>"NuHost"</tt> */
        if (Root* root = session.getRoot().get()) {
            switch (root->hostVersion().getKind()) {
             case HostVersion::PHost:
                return makeStringValue("PHost");
             case HostVersion::SRace:
                return makeStringValue("SRace");
             case HostVersion::Host:
                return makeStringValue("Host");
             case HostVersion::NuHost:
                return makeStringValue("NuHost");
             case HostVersion::Unknown:
                break;
            }
        }
        return 0;
     case igpSystemHostCode:
        /* @q System.Host$:Int (Global Property)
           Name of Host program.
           Values in use so far:
           <table>
            <tr><th width="7" align="left">System.Host</th><th width="8" align="left">System.Host$</th></tr>
            <tr><td width="7">Host</td>       <td width="8">0</td></tr>
            <tr><td width="7">SRace</td>      <td width="8">1</td></tr>
            <tr><td width="7">PHost</td>      <td width="8">2</td></tr>
            <tr><td width="7">NuHost</td>     <td width="8">3</td></tr>
           </table>
           You should prefer using {System.Host} instead. */
        if (Root* root = session.getRoot().get()) {
            switch (root->hostVersion().getKind()) {
             case HostVersion::PHost:
                return makeIntegerValue(2);
             case HostVersion::SRace:
                return makeIntegerValue(1);
             case HostVersion::Host:
                return makeIntegerValue(0);
             case HostVersion::NuHost:
                return makeIntegerValue(3);
             case HostVersion::Unknown:
                break;
            }
        }
        return 0;
     case igpSystemHostVersion:
        /* @q System.HostVersion:Int (Global Property)
           Host version number.
           The version number is converted to a number, with three digits fo the patchlevel and two for the minor version.
           If the respective host version uses letters to specify the patchlevel, "a" is 1, "b" is 2, and so on.
           PCC2 also knows that some host versions use "3.1" to actually mean "3.10".
           Examples:
           <table>
            <tr><th width="5">Version</th><th width="5">Value</th></tr>
            <tr><td width="5">3.22.20</td><td width="5">322020</td></tr>
            <tr><td width="5">3.15</td>   <td width="5">315000</td></tr>
            <tr><td width="5">3.5c</td>   <td width="5">305003</td></tr>
           </table> */
        if (Root* root = session.getRoot().get()) {
            return makeIntegerValue(root->hostVersion().getVersion());
        } else {
            return 0;
        }
     case igpRegSharewareFlag:
        /* @q System.GameType$:Bool (Global Property)
           Registration flag.
           %True if you use a shareware key (Tech 6 limit), %False if you use a full version. */
        if (Root* root = session.getRoot().get()) {
            return makeBooleanValue(root->registrationKey().getStatus() == RegistrationKey::Unregistered);
        } else {
            return 0;
        }
     case igpRegSharewareText:
        /* @q System.GameType:Str (Global Property)
           Registration flag.
           One of <tt>"Shareware"</tt> or <tt>"Registered"</tt>. */
        if (Root* root = session.getRoot().get()) {
            return makeStringValue(root->registrationKey().getStatus() == RegistrationKey::Unregistered ? "Shareware" : "Registered");
        } else {
            return 0;
        }
     case igpRandomSeed:
        /* @q System.RandomSeed:Int (Global Property)
           Random number generator seed.
           Using the same seed, you can reproduce the same random number sequence.
           The seed is a full 32-bit value.

           The underlying random number generator is undocumented as of now,
           and has nothing to do with the random number generators used in VCR/PVCR.
           It is not guaranteed that the same random number generator will be used throughout all versions of PCC.
           @see Random
           @assignable */
        return makeIntegerValue(session.rng().getSeed());
     case igpRegStr1:
        /* @q System.RegStr1:Str (Global Property)
           Your registration key.
           This is the first line (name or registration number) of the key. */
        if (Root* root = session.getRoot().get()) {
            return makeStringValue(root->registrationKey().getLine(RegistrationKey::Line1));
        } else {
            return 0;
        }
     case igpRegStr2:
        /* @q System.RegStr2:Str (Global Property)
           Your registration key.
           This is the second line (registration number or date) of the key. */
        if (Root* root = session.getRoot().get()) {
            return makeStringValue(root->registrationKey().getLine(RegistrationKey::Line2));
        } else {
            return 0;
        }
     case igpTurnNumber:
        /* @q Turn:Int (Global Property)
           Turn number. */
        if (Game* game = session.getGame().get()) {
            return makeIntegerValue(game->currentTurn().universe().getTurnNumber());
        } else {
            return 0;
        }
// FIXME: port (property)
//      case igpTurnIsNew:
//         /* @q Turn.IsNew:Bool (Global Property)
//            New-turn flag.
//            True if this is a new turn, false if you have opened PCC for the second time this turn. */
//         if (haveDisplayedTurn())
//             return makeBoolValue(getDisplayedTurn().getDatabaseTurn() < getDisplayedTurn().getTurnNumber());
//         else
//             return 0;
// FIXME: port (property)
     case igpTurnTime:
        /* @q Turn.Time:Str (Global Property)
           Turn time.
           Time of last host run, in <tt>hh:mm:ss</tt> format,
           using the host's timezone and 24-hour format. */
        if (Game* game = session.getGame().get()) {
            const Timestamp& ts = game->currentTurn().universe().getTimestamp();
            if (ts.isValid()) {
                return makeStringValue(ts.getTimeAsString());
            } else {
                return 0;
            }
        } else {
            return 0;
        }
     case igpTurnDate:
        /* @q Turn.Date:Str (Global Property)
           Turn date.
           Date of last host run, in <tt>mm-dd-yyyy</tt> format,
           using the host's timezone. */
        if (Game* game = session.getGame().get()) {
            const Timestamp& ts = game->currentTurn().universe().getTimestamp();
            if (ts.isValid()) {
                return makeStringValue(ts.getDateAsString());
            } else {
                return 0;
            }
        } else {
            return 0;
        }
    }
    return 0;
}

void
game::interface::setGlobalProperty(GlobalProperty igp, Session& session, afl::data::Value* value)
{
    // ex int/if/globalif.h:setGlobalProperty
    int32_t iv;
    switch (igp) {
    // FIXME: port (assignable property)
    //  case igpSelectionLayer:
    //     if (checkIntArg(iv, value, 0, GMultiSelection::NUM_SELECTION_LAYERS-1))
    //         GMultiSelection::setCurrentSelectionLayer(iv);
    //     break;
     case igpRandomSeed:
        if (interpreter::checkIntegerArg(iv, value)) {
            session.rng().setSeed(iv);
        }
        break;
     default:
        throw interpreter::Error::notAssignable();
    }
}
