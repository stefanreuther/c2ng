/**
  *  \file game/interface/globalcommands.cpp
  *  \brief Global Commands
  */

#include "game/interface/globalcommands.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/vectorvalue.hpp"
#include "game/actions/preconditions.hpp"
#include "game/config/markeroption.hpp"
#include "game/exception.hpp"
#include "game/game.hpp"
#include "game/interface/configurationcontext.hpp"
#include "game/limits.hpp"
#include "game/root.hpp"
#include "game/turn.hpp"
#include "game/turnloader.hpp"
#include "interpreter/arraydata.hpp"
#include "interpreter/arrayvalue.hpp"
#include "interpreter/exporter/configuration.hpp"
#include "interpreter/indexablevalue.hpp"
#include "interpreter/values.hpp"

using interpreter::checkIntegerArg;
using interpreter::checkStringArg;
using game::MAX_NUMBER;

namespace {
    class PostSaveAction : public afl::base::Closure<void(bool)> {
     public:
        PostSaveAction(interpreter::Process& proc, game::Session& session)
            : m_process(proc), m_session(session)
            { }

        virtual void call(bool flag)
            {
                // continueProcess will delete the task, so save m_session first
                game::Session& s = m_session;
                if (!flag) {
                    s.processList().continueProcessWithFailure(m_process, "Save error");
                } else {
                    s.processList().continueProcess(m_process);
                }
                s.runScripts();
            }

     private:
        interpreter::Process& m_process;
        game::Session& m_session;
    };

    void drawLineOrRectangle(game::Session& session,
                             interpreter::Arguments& args,
                             game::map::Drawing::Type drawingType,
                             bool normalizeCoords)
    {
        // ex int/if/globalif.cc:doLineRectangle, globint.pas:LineOrRectangle
        // <command> x1,y1,x2,y2[,color,tag,expire]
        using game::map::Drawing;

        args.checkArgumentCount(4, 7);

        int32_t x1, y1, x2, y2;
        int32_t color = 9;
        int32_t tag = 0;
        int32_t expire = -1;

        if (!checkIntegerArg(x1, args.getNext(), 0, MAX_NUMBER)
            || !checkIntegerArg(y1, args.getNext(), 0, MAX_NUMBER)
            || !checkIntegerArg(x2, args.getNext(), 0, MAX_NUMBER)
            || !checkIntegerArg(y2, args.getNext(), 0, MAX_NUMBER))
            return;
        checkIntegerArg(color, args.getNext(), 0, Drawing::NUM_USER_COLORS);
        checkIntegerArg(tag, args.getNext(), 0, 0xFFFF);
        checkIntegerArg(expire, args.getNext(), -1, 0x7FFF);

        // Refuse making drawing of a certain size
        if (std::abs(x2-x1) > 5000 || std::abs(y2-y1) > 5000) {
            throw interpreter::Error::rangeError();
        }

        // Context check
        game::Game& g = game::actions::mustHaveGame(session);
        game::Turn& t = game::actions::mustBeLocallyEditable(g.viewpointTurn());

        // Normalize coordinates if needed
        game::map::Point a(x1, y1);
        game::map::Point b(x2, y2);
        if (normalizeCoords) {
            b = g.mapConfiguration().getSimpleNearestAlias(b, a);
        }

        // Draw it
        std::auto_ptr<Drawing> drawing(new Drawing(a, drawingType));
        drawing->setPos2(b);
        drawing->setColor(uint8_t(color));
        drawing->setTag(tag);
        drawing->setExpire(expire);

        t.universe().drawings().addNew(drawing.release());
    }
}


bool
game::interface::checkPlayerSetArg(PlayerSet_t& result, afl::data::Value* value)
{
    result.clear();
    if (interpreter::IndexableValue* iv = dynamic_cast<interpreter::IndexableValue*>(value)) {
        // We need to use IndexableValue because that allows retrieving values without having a process.
        // In contrast, FArrayDim checks CallableValue.
        afl::data::Segment seg;
        iv->getAll(seg, 0);
        for (size_t i = 0, n = seg.size(); i < n; ++i) {
            int playerNr = 0;
            if (interpreter::checkIntegerArg(playerNr, seg[i], 0, MAX_PLAYERS)) {
                result += playerNr;
            }
        }
        return true;
    } else if (afl::data::VectorValue* vv = dynamic_cast<afl::data::VectorValue*>(value)) {
        // We get those if input is JSON, i.e. on the API / c2play
        afl::data::Vector& vec = *vv->getValue();
        for (size_t i = 0, n = vec.size(); i < n; ++i) {
            int playerNr = 0;
            if (interpreter::checkIntegerArg(playerNr, vec[i], 0, MAX_PLAYERS)) {
                result += playerNr;
            }
        }
        return true;
    } else {
        int playerNr = 0;
        if (interpreter::checkIntegerArg(playerNr, value, 0, MAX_PLAYERS)) {
            result += playerNr;
            return true;
        } else {
            return false;
        }
    }
}

afl::data::Value*
game::interface::makePlayerSet(PlayerSet_t set)
{
    if (set.empty()) {
        return 0;
    } else {
        afl::base::Ref<interpreter::ArrayData> ad = *new interpreter::ArrayData();
        ad->addDimension(0);
        for (int i = 0; i <= MAX_PLAYERS; ++i) {
            if (set.contains(i)) {
                ad->pushBackNew(interpreter::makeIntegerValue(i));
            }
        }
        return new interpreter::ArrayValue(ad);
    }
}


/* @q AddConfig line:Str (Global Command)
   Modify the game configuration (PConfig/HConfig).
   %line is a configuration assignment as it could appear in <tt>pconfig.src</tt>.
   This command will process the line, and update the in-memory configuration accordingly
   (it will not update the configuration file!).

   For example,
   | AddConfig "EngineShieldBonusRate = 0"
   will disable the engine-shield bonus. After that command,
   | {Cfg()|Cfg}("EngineShieldBonusRate")
   will return 0.

   You can only modify complete options, there's no way to modify just one slot of an array option.

   With PHost, some host settings can be permanently modified by players by sending
   a command message (for example, the language).
   Use {AddCommand} to send these messages.

   <b>This function is for people who know what they're doing.</b>
   Changing the configuration will not immediately update the screen.
   Some settings known to cause trouble, in increasing order of severity:
   - %ColonistTaxRate and friends: you must call <tt>{UI.Update} 1</tt>
     to update the predictions on control screens;
   - %CPEnableRemote, %CPEnableGive: you may have to exit and re-enter the
     control screen to add/remove the respective buttons;
   - %PlayerRace, %StarbaseCost, etc.: don't even think about modifying that in mid-game.

   @change Whereas PCC and PCC2 only accept options they know in this command, PCC2ng will accept all names.
   A previously-undefined name will produce a new option of type "string".

   This command is the same as
   | Call System.Cfg->Add line

   @see CreateConfigOption, Add (Configuration Command)
   @since PCC 1.1.4, PCC2 1.99.25, PCC2 2.40.1 */
void
game::interface::IFAddConfig(game::Session& session, interpreter::Process& proc, interpreter::Arguments& args)
{
    // ex int/if/globalif.h:IFAddConfig, globint.pas:Global_AddConfig
    IFConfiguration_Add(ConfigurationContext::Data(session, game::actions::mustHaveRoot(session).hostConfiguration()), proc, args);
}

/* @q AddFCode line:Str (Global Command)
   Add a friendly code to the <a href="pcc2:fcode">selection list</a>.
   %line is a text line as it could appear in <tt>fcodes.cc</tt>.

   For example,
   | AddFCode "cln,s-57,Clone this ship"
   will define the "cln" friendly code (this definition already appears in the default <tt>fcodes.cc</tt> by default).

   @diff In PCC 1.x, this command always adds the new code at the end.
   In PCC2, the friendly code list is always sorted alphabetically.

   @since PCC 1.1.4, PCC2 1.99.25, PCC2 2.40.1 */
void
game::interface::IFAddFCode(game::Session& session, interpreter::Process& /*proc*/, interpreter::Arguments& args)
{
    // ex int/if/globalif.h:IFAddFCode, globint.pas:Global_AddFCode

    // Parse args
    args.checkArgumentCount(1);
    String_t text;
    if (!checkStringArg(text, args.getNext())) {
        return;
    }

    // Must have a turn
    game::spec::ShipList& shipList = game::actions::mustHaveShipList(session);

    // Parse
    String_t::size_type n = text.find(',');
    if (n == String_t::npos) {
        throw interpreter::Error("Invalid friendly code");
    }

    // Do it
    shipList.friendlyCodes().addCode(game::spec::FriendlyCode(afl::string::strTrim(String_t(text, 0, n)),
                                                              afl::string::strTrim(String_t(text, n+1)),
                                                              session.translator()));
}

/* @q AddPref line:Str (Global Command)
   Modify the user configuration (preferences/options).
   %line is a configuration assignment as it could appear in <tt>pcc2.ini</tt>.
   This command will process the line, and update the in-memory configuration accordingly.
   The configuration file will be rewritten the next time PCC2 exits the game.

   You can only modify complete options, there's no way to modify just one slot of an array option.

   FIXME: Need to have a way to have configuration without a loaded game

   If the option you're setting has not be defined before, this command will produce a new option of type "string".
   (In PCC2, the command will fail for unknown options.)

   This command is the same as
   | Call System.Pref->Add line

   @see CreatePrefOption, Add (Configuration Command)
   @since PCC2 2.40.1, PCC2 2.0.12 */
void
game::interface::IFAddPref(game::Session& session, interpreter::Process& proc, interpreter::Arguments& args)
{
    IFConfiguration_Add(ConfigurationContext::Data(session, game::actions::mustHaveRoot(session).userConfiguration()), proc, args);
}

/* @q AuthPlayer player:Int, password:Str (Global Command)
   Defines a player password.
   When you load the specified player's data, and the password matches, PCC2 will not ask for the password.
   It is not an error to specify the wrong password with this command.

   This command can be placed in your <a href="int:statement:startup">autoexec.q</a> file in your game directory.
   For example, when you're playing the Feds, you could put the following in the game's <tt>autoexec.q</tt> file:
   |  On BeforeLoad Do AuthPlayer 1, "kirk"     % the Fed password
   This will let you open the Fed RST without being prompted for passwords on your computer
   (but everyone else on other computers without this script will still have to know it).

   Passwords are forgotten whenever you leave the <a href="pcc2:racescreen">race screen</a>,
   so you should regenerate it in the {BeforeLoad} hook.

   @change In PCC2NG (2.40+) and PCC 1.x, {AuthPlayer} commands stack.
   Providing multiple passwords will check all of them.
   In PCC2, only the last {AuthPlayer} command for a player will be effective.

   @since PCC 1.1.1, PCC2 1.99.25, PCC2 2.40.8 */
void
game::interface::IFAuthPlayer(game::Session& session, interpreter::Process& /*proc*/, interpreter::Arguments& args)
{
    // ex int/if/globalif.cc:IFAuthPlayer, globint.pas:Global_AuthPlayer
    // Parse args
    args.checkArgumentCount(2);
    int32_t playerNr;
    String_t password;
    if (!interpreter::checkIntegerArg(playerNr, args.getNext(), 1, MAX_PLAYERS) || !checkStringArg(password, args.getNext())) {
        return;
    }

    // Remember password
    std::auto_ptr<AuthCache::Item> it(new AuthCache::Item());
    it->playerNr = playerNr;
    it->password = password;
    session.authCache().addNew(it.release());
}

/* @q CC$SelectionExec layer:Int, code:Str (Internal)
   Back-end to {SelectionExec}.

   @since PCC2 2.40.3, PCC2 1.99.10 */
void
game::interface::IFCCSelectionExec(game::Session& session, interpreter::Process& /*proc*/, interpreter::Arguments& args)
{
    // ex int/if/globalif.cc:IFCCSelectionExec
    args.checkArgumentCount(2);

    Game& g = game::actions::mustHaveGame(session);

    int32_t layer;
    String_t code;
    if (!interpreter::checkIntegerArg(layer, args.getNext(), 0, int(g.selections().getNumLayers()))) {
        return;
    }
    if (!interpreter::checkStringArg(code, args.getNext())) {
        return;
    }

    size_t effLayer = (layer == 0
                       ? g.selections().getCurrentLayer()
                       : size_t(layer-1));

    g.selections().executeCompiledExpression(code, effLayer, g.viewpointTurn().universe());
}

/* @q CC$History.ShowTurn n:Int (Internal)
   Activate the given turn; back-end to {History.ShowTurn}.

   @since PCC2 2.40.12 */
void
game::interface::IFCCHistoryShowTurn(game::Session& session, interpreter::Process& /*proc*/, interpreter::Arguments& args)
{
    // Split off original IFHistoryShowTurn
    // Check parameters
    args.checkArgumentCount(1);
    int32_t turnNumber;
    if (!interpreter::checkIntegerArg(turnNumber, args.getNext(), 0, MAX_NUMBER)) {
        return;
    }

    // Do we have a game loaded?
    Game& g = game::actions::mustHaveGame(session);

    // Check turn number
    int currentTurn = g.currentTurn().getTurnNumber();
    if (turnNumber == 0) {
        turnNumber = currentTurn;
    }
    if (turnNumber <= 0 || turnNumber > currentTurn) {
        throw interpreter::Error::rangeError();
    }

    // Verify
    if (turnNumber < currentTurn) {
        HistoryTurn* ht = g.previousTurns().get(turnNumber);
        if (ht == 0 || ht->getStatus() != HistoryTurn::Loaded) {
            throw interpreter::Error("Turn not available");
        }
    }

    // Activate
    g.setViewpointTurnNumber(turnNumber);
}

/* @q CreateConfigOption key:Str, type:Str (Global Command)
   Create a new game configuration option (PConfig/HConfig).
   Use this to track configuration options that PCC2 does not support internally.

   %key is the name of the option.

   %type is the type of the value.
   Supported types are:
   - "int"/"integer": a number
   - "str"/"string": a string
   - "bool"/"boolean": a boolean value (yes/no)
   The type affects acceptable values for the option, and the return type produced by {Cfg()}.

   This command is the same as
   | Call System.Cfg->Create key, type

   @see AddConfig, Cfg(), CreatePrefOption, Create (Configuration Command)
   @since PCC2 2.40.1 */
void
game::interface::IFCreateConfigOption(game::Session& session, interpreter::Process& proc, interpreter::Arguments& args)
{
    IFConfiguration_Create(ConfigurationContext::Data(session, game::actions::mustHaveRoot(session).hostConfiguration()), proc, args);
}

/* @q CreatePrefOption key:Str, type:Str (Global Command)
   Create a new user configuration option (pcc2.ini).
   Use this to track configuration options that PCC2 does not support internally.

   %key is the name of the option.

   %type is the type of the value.
   Supported types are:
   - "int"/"integer": a number
   - "str"/"string": a string
   - "bool"/"boolean": a boolean value (yes/no)
   The type affects acceptable values for the option, and the return type produced by {Pref()}.

   This command is the same as
   | Call System.Pref->Create key, type

   @see AddPref, Pref(), CreateConfigOption, Create (Configuration Command)
   @since PCC2 2.40.1 */
void
game::interface::IFCreatePrefOption(game::Session& session, interpreter::Process& proc, interpreter::Arguments& args)
{
    IFConfiguration_Create(ConfigurationContext::Data(session, game::actions::mustHaveRoot(session).userConfiguration()), proc, args);
}

/* @q Export array, fields:Str, file:Str, type:Str, Optional charset:Str (Global Command)
   Export data from an array, into a file.

   The %array must be an object array, such as {Ship|Ship()}, {Planet|Planet()}, {Hull|Hull()}, etc.
   The array must not be empty.

   The %fields is a string containing a list of fields, separated by commas,
   each optionally followed by "@" and a width, for example, "Id@5,Name@-20".
   The widths are used for file formats that support it;
   a positive number produces a right-justified field, a negative number produces a left-justified field.

   Further parameters:
   - %file: name of file to create
   - %type: file type, one of "text", "table", "csv", "tsv", "ssv", "json", "html", "dbf"
   - %charset: character set name; defaults to Latin-1 if none given.

   @since PCC2 2.40.13 */
void
game::interface::IFExport(game::Session& session, interpreter::Process& /*proc*/, interpreter::Arguments& args)
{
    args.checkArgumentCount(4, 5);

    // First parameter
    afl::data::Value* array = args.getNext();
    if (array == 0) {
        return;
    }

    // Further mandatory parameters
    String_t fieldNames, fileName, typeName;
    if (!checkStringArg(fieldNames, args.getNext())
        || !checkStringArg(fileName, args.getNext())
        || !checkStringArg(typeName, args.getNext()))
    {
        return;
    }

    // Create a Configuration
    interpreter::exporter::Configuration config;
    config.fieldList().addList(fieldNames);
    config.setFormatByName(typeName, session.translator());

    // Optional parameter
    String_t charsetName;
    if (checkStringArg(charsetName, args.getNext())) {
        config.setCharsetByName(charsetName, session.translator());
    }

    // Try to export
    interpreter::CallableValue* callable = dynamic_cast<interpreter::CallableValue*>(array);
    if (callable == 0) {
        throw interpreter::Error::typeError(interpreter::Error::ExpectIterable);
    }

    std::auto_ptr<interpreter::Context> ctx(callable->makeFirstContext());
    if (ctx.get() == 0) {
        throw interpreter::Error("Export set is empty");
    }

    afl::base::Ref<afl::io::Stream> file = session.world().fileSystem().openFile(fileName, afl::io::FileSystem::Create);
    config.exportFile(*ctx, *file);
}

/* @q NewCannedMarker x:Int, y:Int, slot:Int, Optional tag:Int, expire:Int (Global Command)
   Create a new canned marker drawing.
   Users can predefine a number of marker shapes/colors.
   The %slot parameter selects which type to create, starting at 0.

   The %tag is a value between 0 and 32767 you can use to identify your drawings,
   usually this value is created using {Atom}.

   %expire defines the time-of-expiry for the game as a turn number:
   if the current turn number is larger than this value, the drawing is automatically deleted.
   Thus, set %expire=0 to make drawings only visible for the current session.
   %expire=-1 is the default, drawings with this value never expire.
   @see NewMarker
   @since PCC2 2.40.10 */
void
game::interface::IFNewCannedMarker(game::Session& session, interpreter::Process& /*proc*/, interpreter::Arguments& args)
{
    // ex createCannedMarker (sort-of)
    using game::map::Drawing;

    args.checkArgumentCount(3, 5);

    // Parse args
    int32_t x, y, shape;
    int32_t tag = 0;
    int32_t expire = -1;
    if (!checkIntegerArg(x, args.getNext(), 0, MAX_NUMBER)
        || !checkIntegerArg(y, args.getNext(), 0, MAX_NUMBER)
        || !checkIntegerArg(shape, args.getNext(), 0, MAX_NUMBER))
    {
        return;
    }
    checkIntegerArg(tag, args.getNext(), 0, 0xFFFF);
    checkIntegerArg(expire, args.getNext(), -1, 0x7FFF);

    // Context check
    Root& r = game::actions::mustHaveRoot(session);
    Game& g = game::actions::mustHaveGame(session);
    Turn& t = game::actions::mustBeLocallyEditable(g.viewpointTurn());

    // Obtain configuration
    const game::config::MarkerOptionDescriptor* opt = game::config::UserConfiguration::getCannedMarker(shape);
    if (opt == 0) {
        throw interpreter::Error::rangeError();
    }

    // Draw it
    std::auto_ptr<Drawing> drawing(new Drawing(game::map::Point(x, y), r.userConfiguration()[*opt]()));
    drawing->setTag(tag);
    drawing->setExpire(expire);

    t.universe().drawings().addNew(drawing.release());
}

/* @q NewCircle x:Int, y:Int, radius:Int, Optional color:Int, tag:Int, expire:Int (Global Command)
   Create new circle drawing.
   The circle will be centered at %x,%y, and have the specified %radius.

   The %color is an integer between 0 and 30, and selects the color.
   The %tag is a value between 0 and 32767 you can use to identify your drawings,
   usually this value is created using {Atom}.

   %expire defines the time-of-expiry for the game as a turn number:
   if the current turn number is larger than this value, the drawing is automatically deleted.
   Thus, set %expire=0 to make drawings only visible for the current session.
   %expire=-1 is the default, drawings with this value never expire.
   @see NewLine, NewRectangle, NewMarker
   @since PCC2 1.99.9, PCC 1.0.5, PCC2 2.40.1 */
void
game::interface::IFNewCircle(game::Session& session, interpreter::Process& /*proc*/, interpreter::Arguments& args)
{
    // ex int/if/globalif.h:IFNewCircle, globint.pas:Global_NewCircle
    // NewCircle x,y,radius,[color,tag,expire]
    using game::map::Drawing;

    args.checkArgumentCount(3, 6);

    int32_t x, y, radius;
    int32_t color = 9;
    int32_t tag = 0;
    int32_t expire = -1;
    if (!checkIntegerArg(x, args.getNext(), 0, MAX_NUMBER)
        || !checkIntegerArg(y, args.getNext(), 0, MAX_NUMBER)
        || !checkIntegerArg(radius, args.getNext(), 1, 5000))
        return;
    checkIntegerArg(color, args.getNext(), 0, Drawing::NUM_USER_COLORS);
    checkIntegerArg(tag, args.getNext(), 0, 0xFFFF);
    checkIntegerArg(expire, args.getNext(), -1, 0x7FFF);

    // Context check
    Game& g = game::actions::mustHaveGame(session);
    Turn& t = game::actions::mustBeLocallyEditable(g.viewpointTurn());

    // Do it
    std::auto_ptr<Drawing> drawing(new Drawing(game::map::Point(x, y), Drawing::CircleDrawing));
    drawing->setCircleRadius(radius);
    drawing->setColor(uint8_t(color));
    drawing->setTag(tag);
    drawing->setExpire(expire);

    t.universe().drawings().addNew(drawing.release());
}

/* @q NewRectangle x1:Int, y1:Int, x2:Int, y2:Int, Optional color:Int, tag:Int, expire:Int (Global Command)
   Create new rectangle drawing.
   On a wrapped map, the coordinates will be adjusted so that the rectangle spans the minimum area,
   possibly by crossing a map seam.

   The %color is an integer between 0 and 30, and selects the color.
   The %tag is a value between 0 and 32767 you can use to identify your drawings,
   usually this value is created using {Atom}.

   %expire defines the time-of-expiry for the game as a turn number:
   if the current turn number is larger than this value, the drawing is automatically deleted.
   Thus, set %expire=0 to make drawings only visible for the current session.
   %expire=-1 is the default, drawings with this value never expire.
   @see NewCircle, NewLine, NewRectangleRaw, NewMarker
   @since PCC2 1.99.9, PCC 1.0.5, PCC2 2.40.1 */
void
game::interface::IFNewRectangle(game::Session& session, interpreter::Process& /*proc*/, interpreter::Arguments& args)
{
    // ex int/if/globalif.h:IFNewRectangle, globint.pas:Global_NewRectangle
    drawLineOrRectangle(session, args, game::map::Drawing::RectangleDrawing, true);
}

/* @q NewRectangleRaw x1:Int, y1:Int, x2:Int, y2:Int, Optional color:Int, tag:Int, expire:Int (Global Command)
   Create new rectangle drawing.
   The coordinates will not be adjusted for wrapped maps.

   The %color is an integer between 0 and 30, and selects the color.
   The %tag is a value between 0 and 32767 you can use to identify your drawings,
   usually this value is created using {Atom}.

   %expire defines the time-of-expiry for the game as a turn number:
   if the current turn number is larger than this value, the drawing is automatically deleted.
   Thus, set %expire=0 to make drawings only visible for the current session.
   %expire=-1 is the default, drawings with this value never expire.
   @see NewCircle, NewLine, NewRectangle, NewMarker
   @since PCC2 1.99.9, PCC 1.1.15, PCC2 2.40.1 */
void
game::interface::IFNewRectangleRaw(game::Session& session, interpreter::Process& /*proc*/, interpreter::Arguments& args)
{
    // ex int/if/globalif.h:IFNewRectangleRaw, globint.pas:Global_NewRectangleRaw
    drawLineOrRectangle(session, args, game::map::Drawing::RectangleDrawing, false);
}

/* @q NewLine x1:Int, y1:Int, x2:Int, y2:Int, Optional color:Int, tag:Int, expire:Int (Global Command)
   Create new line drawing.
   On a wrapped map, the coordinates will be adjusted so that the line covers the minimum distance,
   possibly by crossing a map seam.

   The %color is an integer between 0 and 30, and selects the color.
   The %tag is a value between 0 and 32767 you can use to identify your drawings,
   usually this value is created using {Atom}.

   %expire defines the time-of-expiry for the game as a turn number:
   if the current turn number is larger than this value, the drawing is automatically deleted.
   Thus, set %expire=0 to make drawings only visible for the current session.
   %expire=-1 is the default, drawings with this value never expire.
   @see NewCircle, NewLineRaw, NewRectangle, NewMarker
   @since PCC2 1.99.9, PCC 1.0.5, PCC2 2.40.1 */
void
game::interface::IFNewLine(game::Session& session, interpreter::Process& /*proc*/, interpreter::Arguments& args)
{
    // ex int/if/globalif.h:IFNewLine, globint.pas:Global_NewLine
    drawLineOrRectangle(session, args, game::map::Drawing::LineDrawing, true);
}

/* @q NewLineRaw x1:Int, y1:Int, x2:Int, y2:Int, Optional color:Int, tag:Int, expire:Int (Global Command)
   Create new line drawing.
   The coordinates will not be adjusted for wrapped maps.

   The %color is an integer between 0 and 30, and selects the color.
   The %tag is a value between 0 and 32767 you can use to identify your drawings,
   usually this value is created using {Atom}.

   %expire defines the time-of-expiry for the game as a turn number:
   if the current turn number is larger than this value, the drawing is automatically deleted.
   Thus, set %expire=0 to make drawings only visible for the current session.
   %expire=-1 is the default, drawings with this value never expire.
   @see NewCircle, NewLine, NewRectangle, NewMarker
   @since PCC2 1.99.9, PCC 1.1.15, PCC2 2.40.1 */
void
game::interface::IFNewLineRaw(game::Session& session, interpreter::Process& /*proc*/, interpreter::Arguments& args)
{
    // ex int/if/globalif.h:IFNewLineRaw, globint.pas:Global_NewLineRaw
    drawLineOrRectangle(session, args, game::map::Drawing::LineDrawing, false);
}

/* @q NewMarker x:Int, y:Int, type:Int, Optional color:Int, text:Str, tag:Int, expire:Int (Global Command)
   Create new marker drawing.
   The %type selects the marker shape.

   The %color is an integer between 0 and 30, and selects the color.
   The %text contains the marker's comment shown on the map.
   The %tag is a value between 0 and 32767 you can use to identify your drawings,
   usually this value is created using {Atom}.

   %expire defines the time-of-expiry for the game as a turn number:
   if the current turn number is larger than this value, the drawing is automatically deleted.
   Thus, set %expire=0 to make drawings only visible for the current session.
   %expire=-1 is the default, drawings with this value never expire.

   Note: this command was erroneously documented without the %text argument in some versions,
   but has always accepted the parameters as shown above.

   @see NewCircle, NewLineRaw, NewRectangle, NewMarker, NewCannedMarker
   @since PCC2 1.99.9, PCC 1.0.5, PCC2 2.40.1 */
void
game::interface::IFNewMarker(game::Session& session, interpreter::Process& /*proc*/, interpreter::Arguments& args)
{
    // ex int/if/globalif.h:IFNewMarker, globint.pas:Global_NewMarker
    // NewMarker x,y,typ[,color,text,tag,expire]
    using game::map::Drawing;

    args.checkArgumentCount(3, 7);

    int32_t x, y, type;
    int32_t color = 9;
    int32_t tag = 0;
    int32_t expire = -1;
    String_t text;
    if (!checkIntegerArg(x, args.getNext(), 0, MAX_NUMBER)
        || !checkIntegerArg(y, args.getNext(), 0, MAX_NUMBER)
        || !checkIntegerArg(type, args.getNext(), 0, Drawing::NUM_USER_MARKERS-1)) {
        return;
    }
    checkIntegerArg(color, args.getNext(), 0, Drawing::NUM_USER_COLORS);
    checkStringArg(text, args.getNext());
    checkIntegerArg(tag, args.getNext(), 0, 0xFFFF);
    checkIntegerArg(expire, args.getNext(), -1, 0x7FFF);

    // Context check
    Game& g = game::actions::mustHaveGame(session);
    Turn& t = game::actions::mustBeLocallyEditable(g.viewpointTurn());

    // Draw it
    std::auto_ptr<Drawing> drawing(new Drawing(game::map::Point(x, y), Drawing::MarkerDrawing));
    drawing->setMarkerKind(type);
    drawing->setColor(uint8_t(color));
    drawing->setTag(tag);
    drawing->setExpire(expire);
    drawing->setComment(text);

    t.universe().drawings().addNew(drawing.release());
}

/* @q History.LoadTurn nr:Int (Global Command)
   Load turn from history database.

   The parameter specifies the turn number to load.
   The special case "0" will load the current turn.
   PCC2 will load the specified turn's result file, if available.
   The turn will be loaded but not shown.

   @see History.ShowTurn
   @since PCC2 2.40.12 */
void
game::interface::IFHistoryLoadTurn(game::Session& session, interpreter::Process& proc, interpreter::Arguments& args)
{
    // Check parameters
    args.checkArgumentCount(1);
    int32_t turnNumber;
    if (!interpreter::checkIntegerArg(turnNumber, args.getNext(), 0, MAX_NUMBER)) {
        return;
    }

    // Do we have a game loaded?
    Root* r = session.getRoot().get();
    Game* g = session.getGame().get();
    game::spec::ShipList* sl = session.getShipList().get();
    if (g == 0 || r == 0 || sl == 0) {
        throw Exception(Exception::eUser);
    }

    // Check turn number
    int currentTurn = g->currentTurn().getTurnNumber();
    if (turnNumber == 0) {
        turnNumber = currentTurn;
    }
    if (turnNumber <= 0 || turnNumber > currentTurn) {
        throw interpreter::Error::rangeError();
    }

    // Load if required
    if (turnNumber < currentTurn) {
        // If turn is not known at all, update metainformation
        if (g->previousTurns().getTurnStatus(turnNumber) == HistoryTurn::Unknown) {
            g->previousTurns().initFromTurnScores(g->scores(), turnNumber, 1);
            if (TurnLoader* tl = r->getTurnLoader().get()) {
                g->previousTurns().initFromTurnLoader(*tl, *r, g->getViewpointPlayer(), turnNumber, 1);
            }
        }

        // If turn is loadable, load
        HistoryTurn* ht = g->previousTurns().get(turnNumber);
        if (ht != 0 && ht->isLoadable() && r->getTurnLoader().get() != 0) {
            // Load asynchronously; suspend this task until completion.
            class Task : public StatusTask_t {
             public:
                Task(interpreter::Process& proc, Session& session, const afl::base::Ref<Turn>& turn, int turnNr, HistoryTurn& ht)
                    : m_process(proc), m_session(session), m_turn(turn), m_turnNumber(turnNr), m_historyTurn(ht)
                    { }
                void call(bool flag)
                    {
                        bool ok = flag;
                        if (ok) {
                            try {
                                Game& game = game::actions::mustHaveGame(m_session);

                                int player = game.getViewpointPlayer();
                                m_session.postprocessTurn(*m_turn, game::PlayerSet_t(player), game::PlayerSet_t(player), game::map::Object::ReadOnly);
                                m_historyTurn.handleLoadSucceeded(*m_turn);
                                m_session.processList().continueProcess(m_process);
                            }
                            catch (std::exception& e) {
                                ok = false;
                            }
                        }
                        if (!ok) {
                            m_historyTurn.handleLoadFailed();
                            m_session.processList().continueProcessWithFailure(m_process, "Turn not available");
                        }
                    }
             private:
                interpreter::Process& m_process;
                Session& m_session;
                afl::base::Ref<Turn> m_turn;
                const int m_turnNumber;
                HistoryTurn& m_historyTurn;
            };
            afl::base::Ref<Turn> turn = *new Turn();
            proc.suspend(r->getTurnLoader()->loadHistoryTurn(*turn, *g, g->getViewpointPlayer(), turnNumber, *r, session, std::auto_ptr<StatusTask_t>(new Task(proc, session, turn, turnNumber, *ht))));
            return;
        }

        if (ht == 0 || ht->getStatus() != HistoryTurn::Loaded) {
            throw interpreter::Error("Turn not available");
        }
    }
}

/* @q SaveGame [flags:Str] (Global Command)
   Save current game.
   Depending on the game type, this will create and/or upload the turn file.

   Valid flags:
   - "f": make a final turn file. Default is to mark the turn file temporary if possible.

   The flags parameter is supported since PCC2 2.40.12.

   @since PCC 1.0.17, PCC2 1.99.12, PCC2 2.40.5 */
void
game::interface::IFSaveGame(game::Session& session, interpreter::Process& proc, interpreter::Arguments& args)
{
    // ex int/if/globalif.cc:IFSaveGame, globint.pas:Global_SaveGame
    args.checkArgumentCount(0, 1);

    int32_t flags = 0;
    interpreter::checkFlagArg(flags, 0, args.getNext(), "F");

    // Build flags parameter
    TurnLoader::SaveOptions_t opts;
    if ((flags & 1) == 0) {
        opts += TurnLoader::MarkTurnTemporary;
    }

    // Create deferred save action
    std::auto_ptr<afl::base::Closure<void()> > action =
        session.save(opts, std::auto_ptr<afl::base::Closure<void(bool)> >(new PostSaveAction(proc, session)));

    if (action.get() == 0) {
        throw interpreter::Error("No game loaded");
    }

    // Save configuration and game
    proc.suspend(session.saveConfiguration(action));
}

/* @q SendMessage player:Int, text:Str... (Global Command)
   Send a message.
   The player number can be a single integer to send to one player,
   or an array of integers to send to multiple players.
   For example,
   | SendMessage Array(3,4), "Hi there"
   | SendMessage 7, "Knock knock"
   sends a message to players 3 and 4 and another one to player 7.

   @since PCC2 2.40.8 */
void
game::interface::IFSendMessage(game::Session& session, interpreter::Process& /*proc*/, interpreter::Arguments& args)
{
    // ex globint.pas:Global_SendMessage (sort-of; only a stub in PCC1)
    args.checkArgumentCountAtLeast(2);

    game::PlayerSet_t receivers;
    if (!checkPlayerSetArg(receivers, args.getNext())) {
        return;
    }

    String_t text;
    while (args.getNumArgs() > 0) {
        String_t line;
        if (!interpreter::checkStringArg(line, args.getNext())) {
            return;
        }
        text += line;
        text += '\n';
    }

    Game& g = game::actions::mustHaveGame(session);
    Turn& t = game::actions::mustAllowCommands(g.viewpointTurn(), g.getViewpointPlayer());
    t.outbox().addMessage(g.getViewpointPlayer(), text, receivers);
}
