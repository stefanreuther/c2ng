/**
  *  \file game/interface/globalproperty.cpp
  *  \brief Enum game::interface::GlobalProperty
  */

#include "game/interface/globalproperty.hpp"
#include "game/game.hpp"
#include "game/registrationkey.hpp"
#include "game/root.hpp"
#include "game/turn.hpp"
#include "game/turnloader.hpp"
#include "game/v3/genextra.hpp"
#include "game/v3/genfile.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/error.hpp"
#include "interpreter/values.hpp"
#include "version.hpp"

using interpreter::makeStringValue;
using interpreter::makeIntegerValue;
using interpreter::makeSizeValue;
using interpreter::makeBooleanValue;
using game::TurnLoader;

namespace {
    afl::data::Value* getTurnLoaderPropery(TurnLoader::Property p, game::Session& session)
    {
        if (game::Root* pRoot = session.getRoot().get()) {
            if (TurnLoader* pTL = pRoot->getTurnLoader().get()) {
                String_t value = pTL->getProperty(p);
                if (!value.empty()) {
                    return makeStringValue(value);
                }
            }
        }
        return 0;
    }

    String_t getLanguageCode(afl::string::Translator& tx)
    {
        // Translators: translate this as {lang}de, {lang}es, etc.
        // FIXME: We're stripping the tag manually here.
        // The idea is to move this stripping into the actual translator implementation
        // to also allow context-dependant translations such as for example {mission}none, {owner}none
        // that require different word forms in some languages.
        String_t text = tx.translateString("{lang}en");
        String_t::size_type n = text.find('}');
        if (n != String_t::npos) {
            text.erase(0, n+1);
        }
        return text;
    }
}


afl::data::Value*
game::interface::getGlobalProperty(GlobalProperty igp, Session& session)
{
    // ex int/if/globalif.h:getGlobalProperty
    switch (igp) {
     case igpFileFormatLocal:
        /* @q System.Local:Str (Global Property)
           Local file format.
           Reports the file format PCC uses to store your player files:
           - <tt>"DOS"</tt> (same as planets.exe)
           - <tt>"Windows"</tt> (same as Winplan)
           - <tt>"RST"</tt> (not-unpacked result file, c2nu only)
           - <tt>"Nu"</tt> (planets.nu, c2ng only) */
        return getTurnLoaderPropery(TurnLoader::LocalFileFormatProperty, session);
     case igpFileFormatRemote:
        /* @q System.Remote:Str (Global Property)
           Remote file format.
           Reports the file format PCC uses for your turn files, i.e. what the "remote" host system sees:
           - <tt>"DOS"</tt> (same as planets.exe, PCC 1.x only)
           - <tt>"Windows"</tt> (same as Winplan)
           - <tt>"Nu"</tt> (planets.nu, c2nu only) */

        /* In PCC 1.x, this value is 0 for Dosplan, 1 for Winplan, and
           determines the TRN format. This is the config option. As of
           20110206, PCC2 has no such config option and always produces
           Winplan format. */
        return getTurnLoaderPropery(TurnLoader::RemoteFileFormatProperty, session);
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

           @diff In c2ng, this value may be EMPTY if a game is loaded,
           but the game directory is a virtual directory (e.g. network game). */
        if (Root* root = session.getRoot().get()) {
            String_t dirName = root->gameDirectory().getDirectoryName();
            if (dirName.empty()) {
                return 0;
            } else {
                return makeStringValue(dirName);
            }
        } else {
            return 0;
        }
     case igpMyInMsgs:
        /* @q My.InMsgs:Int (Global Property)
           Number of incoming (received) messages this turn. */
        if (const Game* game = session.getGame().get()) {
            return makeSizeValue(game->viewpointTurn().inbox().getNumMessages());
        } else {
            return 0;
        }
     case igpMyOutMsgs:
        /* @q My.OutMsgs:Int (Global Property)
           Number of outgoing (sent) messages this turn. */
        if (const Game* game = session.getGame().get()) {
            return makeSizeValue(game->viewpointTurn().outbox().getNumMessages());
        } else {
            return 0;
        }
     case igpMyVCRs:
        /* @q My.VCRs:Int (Global Property)
           Number of incoming combat recordings this turn. */
        if (const Game* game = session.getGame().get()) {
            if (const game::vcr::Database* db = game->viewpointTurn().getBattles().get()) {
                return makeSizeValue(db->getNumBattles());
            } else {
                return makeSizeValue(0);
            }
        } else {
            return 0;
        }
     case igpRootDirectory:
        /* @q System.RootDirectory:Str (Global Property)
           Root directory.

           The root directory is the directory within the program installation directory
           containing the default specification files.
           If a specification file cannot be found in the {System.GameDirectory|game directory},
           it is looked for in the root directory.
           This directory typically is one of
           - /usr/local/share/planets
           - C:\Programs\PCC2\specs

           @diff In PCC 1.x, it is possible to concatenate this property with a file name
           to access a file in the root directory.
           This does no longer work in PCC2.
           Use the {MakeFileName} function, as in
           | Open MakeFileName(System.RootDirectory, "file.txt") For Input As #1
           to access files in the root directory.

           @change In PCC2ng, it is possible for this property to be empty.
           In network play, a root specification directory may not be used. */
        return getTurnLoaderPropery(TurnLoader::RootDirectoryProperty, session);
     case igpSelectionLayer:
        /* @q Selection.Layer:Int (Global Property)
           Current selection layer.
           A number from 0 to 7.
           @assignable */
        if (const Game* game = session.getGame().get()) {
            return makeSizeValue(game->selections().getCurrentLayer());
        } else {
            return 0;
        }
     case igpSystemLanguage:
        /* @q System.Language:Str (Global Property)
           Language code.
           This is the language the user wants to use,
           usually in the form of a two-letter ISO 639 code ("en" = English).
           @since PCC2 1.99.25 */
        return makeStringValue(getLanguageCode(session.translator()));
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
     case igpSystemHasPassword:
        /* @q System.HasPassword:Bool (Global Property)
           Result file password status.
           This property is True if there is a result file password, False if there is none.
           If the game does not support result file passwords, the value is EMPTY.
           @since PCC2 2.41 */
        if (const Game* g = session.getGame().get()) {
            // We intentionally use currentTurn here. There's no point in asking the password status of a history RST.
            const Turn& t = g->currentTurn();
            if (const game::v3::GenFile* p = game::v3::GenExtra::get(t, g->getViewpointPlayer())) {
                return makeBooleanValue(p->hasPassword());
            }
        }
        return 0;
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
            return makeBooleanValue(root->registrationKey().getStatus() != RegistrationKey::Registered);
        } else {
            return 0;
        }
     case igpRegSharewareText:
        /* @q System.GameType:Str (Global Property)
           Registration flag.
           One of <tt>"Shareware"</tt> or <tt>"Registered"</tt>. */
        if (Root* root = session.getRoot().get()) {
            return makeStringValue(root->registrationKey().getStatus() == RegistrationKey::Registered ? "Registered" : "Shareware");
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
        if (const Game* game = session.getGame().get()) {
            return makeIntegerValue(game->viewpointTurn().getTurnNumber());
        } else {
            return 0;
        }
     case igpTurnIsNew:
        /* @q Turn.IsNew:Bool (Global Property)
           New-turn flag.
           True if this is a new turn, false if you have opened PCC for the second time this turn. */
        if (const Game* game = session.getGame().get()) {
            return makeBooleanValue(game->viewpointTurn().getDatabaseTurnNumber() < game->viewpointTurn().getTurnNumber());
        } else {
            return 0;
        }
     case igpTurnTime:
        /* @q Turn.Time:Str (Global Property)
           Turn time.
           Time of last host run, in <tt>hh:mm:ss</tt> format,
           using the host's timezone and 24-hour format. */
        if (const Game* game = session.getGame().get()) {
            const Timestamp& ts = game->viewpointTurn().getTimestamp();
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
        if (const Game* game = session.getGame().get()) {
            const Timestamp& ts = game->viewpointTurn().getTimestamp();
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
game::interface::setGlobalProperty(GlobalProperty igp, Session& session, const afl::data::Value* value)
{
    // ex int/if/globalif.h:setGlobalProperty
    int32_t iv;
    switch (igp) {
     case igpSelectionLayer:
        if (Game* game = session.getGame().get()) {
            if (interpreter::checkIntegerArg(iv, value, 0, int(game->selections().get(game::map::Selections::Ship).size()) - 1)) {
                game->selections().setCurrentLayer(iv, game->viewpointTurn().universe());
            }
        } else {
            throw interpreter::Error::notAssignable();
        }
        break;
     case igpRandomSeed:
        if (interpreter::checkIntegerArg(iv, value)) {
            session.rng().setSeed(iv);
        }
        break;
     default:
        throw interpreter::Error::notAssignable();
    }
}
