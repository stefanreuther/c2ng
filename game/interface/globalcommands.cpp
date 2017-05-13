/**
  *  \file game/interface/globalcommands.cpp
  */

#include "game/interface/globalcommands.hpp"
#include "game/exception.hpp"
#include "game/game.hpp"
#include "game/turn.hpp"
#include "game/root.hpp"
#include "game/turnloader.hpp"
#include "game/actions/preconditions.hpp"
#include "game/config/stringoption.hpp"
#include "game/config/integeroption.hpp"
#include "game/config/integervalueparser.hpp"
#include "game/config/booleanvalueparser.hpp"

using interpreter::checkIntegerArg;
using interpreter::checkStringArg;

namespace {
    void drawLineOrRectangle(game::Session& session,
                             interpreter::Arguments& args,
                             game::map::Drawing::Type drawingType,
                             bool normalizeCoords)
    {
        // ex int/if/globalif.cc:doLineRectangle
        // <command> x1,y1,x2,y2[,color,tag,expire]
        using game::map::Drawing;

        args.checkArgumentCount(4, 7);

        int32_t x1, y1, x2, y2;
        int32_t color = 9;
        int32_t tag = 0;
        int32_t expire = -1;

        if (!checkIntegerArg(x1, args.getNext(), 0, 10000)
            || !checkIntegerArg(y1, args.getNext(), 0, 10000)
            || !checkIntegerArg(x2, args.getNext(), 0, 10000)
            || !checkIntegerArg(y2, args.getNext(), 0, 10000))
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

        // Normalize coordinates if needed
        game::map::Point a(x1, y1);
        game::map::Point b(x2, y2);
        if (normalizeCoords) {
            b = g.currentTurn().universe().config().getSimpleNearestAlias(b, a);
        }

        // Draw it
        std::auto_ptr<Drawing> drawing(new Drawing(a, drawingType));
        drawing->setPos2(b);
        drawing->setColor(uint8_t(color));
        drawing->setTag(tag);
        drawing->setExpire(expire);

        g.currentTurn().universe().drawings().addNew(drawing.release());
    }

    void createConfigOption(game::config::Configuration& config, interpreter::Arguments& args)
    {
        // Parse args
        String_t key;
        String_t type;
        if (!checkStringArg(key, args.getNext()) || !checkStringArg(type, args.getNext())) {
            return;
        }

        // Create the option by indexing with an appropriate descriptor
        if (type == "str" || type == "string") {
            game::config::StringOptionDescriptor desc;
            desc.m_name = key.c_str();
            config[desc];
        } else if (type == "int" || type == "integer") {
            game::config::IntegerOptionDescriptor desc;
            desc.m_name = key.c_str();
            desc.m_parser = &game::config::IntegerValueParser::instance;
            config[desc];
        } else if (type == "bool" || type == "boolean") {
            game::config::IntegerOptionDescriptor desc;
            desc.m_name = key.c_str();
            desc.m_parser = &game::config::BooleanValueParser::instance;
            config[desc];
        } else {
            throw interpreter::Error::rangeError();
        }
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

   FIXME: add and document a command to define new configuration options.

   @since PCC 1.1.4, PCC2 1.99.25, PCC2 2.40.1 */
void
game::interface::IFAddConfig(interpreter::Process& /*proc*/, game::Session& session, interpreter::Arguments& args)
{
    // ex int/if/globalif.h:IFAddConfig
    // Parse args
    args.checkArgumentCount(1);
    String_t text;
    if (!checkStringArg(text, args.getNext())) {
        return;
    }

    // Must have a turn
    Root& r = game::actions::mustHaveRoot(session);

    // Parse
    String_t::size_type n = text.find('=');
    if (n == String_t::npos) {
        throw interpreter::Error("Invalid configuration setting");
    }

    // Assign the option.
    // We need not verify that this option exists, it will be created.
    r.hostConfiguration().setOption(afl::string::strTrim(String_t(text, 0, n)),
                                    afl::string::strTrim(String_t(text, n+1)),
                                    game::config::ConfigurationOption::User);
    r.hostConfiguration().setDependantOptions();
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
game::interface::IFAddFCode(interpreter::Process& /*proc*/, game::Session& session, interpreter::Arguments& args)
{
    // ex int/if/globalif.h:IFAddFCode

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
                                                              afl::string::strTrim(String_t(text, n+1))));
}

/* @q AddPref line:Str (Global Command)

   FIXME

   @since PCC2 2.40.1 */
void
game::interface::IFAddPref(interpreter::Process& /*proc*/, game::Session& session, interpreter::Arguments& args)
{
    // Parse args
    args.checkArgumentCount(1);
    String_t text;
    if (!checkStringArg(text, args.getNext())) {
        return;
    }

    // Must have a turn
    Root& r = game::actions::mustHaveRoot(session);

    // Parse
    String_t::size_type n = text.find('=');
    if (n == String_t::npos) {
        throw interpreter::Error("Invalid configuration setting");
    }

    // Assign the option.
    // We need not verify that this option exists, it will be created.
    r.userConfiguration().setOption(afl::string::strTrim(String_t(text, 0, n)),
                                    afl::string::strTrim(String_t(text, n+1)),
                                    game::config::ConfigurationOption::User);
}

/* @q CreateConfigOption key:Str, type:Str (Global Command)

   FIXME

   @since PCC2 2.40.1 */
void
game::interface::IFCreateConfigOption(interpreter::Process& /*proc*/, game::Session& session, interpreter::Arguments& args)
{
    args.checkArgumentCount(2);
    createConfigOption(game::actions::mustHaveRoot(session).hostConfiguration(), args);
}

/* @q CreatePrefOption key:Str, type:Str (Global Command)

   FIXME

   @since PCC2 2.40.1 */
void
game::interface::IFCreatePrefOption(interpreter::Process& /*proc*/, game::Session& session, interpreter::Arguments& args)
{
    args.checkArgumentCount(2);
    createConfigOption(game::actions::mustHaveRoot(session).userConfiguration(), args);
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
game::interface::IFNewCircle(interpreter::Process& /*proc*/, game::Session& session, interpreter::Arguments& args)
{
    // ex int/if/globalif.h:IFNewCircle
    // NewCircle x,y,radius,[color,tag,expire]
    using game::map::Drawing;

    args.checkArgumentCount(3, 6);

    int32_t x, y, radius;
    int32_t color = 9;
    int32_t tag = 0;
    int32_t expire = -1;
    if (!checkIntegerArg(x, args.getNext(), 0, 10000)
        || !checkIntegerArg(y, args.getNext(), 0, 10000)
        || !checkIntegerArg(radius, args.getNext(), 1, 5000))
        return;
    checkIntegerArg(color, args.getNext(), 0, Drawing::NUM_USER_COLORS);
    checkIntegerArg(tag, args.getNext(), 0, 0xFFFF);
    checkIntegerArg(expire, args.getNext(), -1, 0x7FFF);

    // Context check
    Game& g = game::actions::mustHaveGame(session);

    // Do it
    std::auto_ptr<Drawing> drawing(new Drawing(game::map::Point(x, y), Drawing::CircleDrawing));
    drawing->setCircleRadius(radius);
    drawing->setColor(uint8_t(color));
    drawing->setTag(tag);
    drawing->setExpire(expire);

    g.currentTurn().universe().drawings().addNew(drawing.release());
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
game::interface::IFNewRectangle(interpreter::Process& /*proc*/, game::Session& session, interpreter::Arguments& args)
{
    // ex int/if/globalif.h:IFNewRectangle
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
game::interface::IFNewRectangleRaw(interpreter::Process& /*proc*/, game::Session& session, interpreter::Arguments& args)
{
    // ex int/if/globalif.h:IFNewRectangleRaw
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
game::interface::IFNewLine(interpreter::Process& /*proc*/, game::Session& session, interpreter::Arguments& args)
{
    // ex int/if/globalif.h:IFNewLine
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
game::interface::IFNewLineRaw(interpreter::Process& /*proc*/, game::Session& session, interpreter::Arguments& args)
{
    // ex int/if/globalif.h:IFNewLineRaw
    drawLineOrRectangle(session, args, game::map::Drawing::LineDrawing, false);
}

/* @q NewMarker x:Int, y:Int, type:Int, Optional color:Int, tag:Int, expire:Int (Global Command)
   Create new marker drawing.
   The %type selects the marker shape.

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
game::interface::IFNewMarker(interpreter::Process& /*proc*/, game::Session& session, interpreter::Arguments& args)
{
    // ex int/if/globalif.h:IFNewMarker
    // NewMarker x,y,typ[,color,text,tag,expire]
    using game::map::Drawing;

    args.checkArgumentCount(3, 7);

    int32_t x, y, type;
    int32_t color = 9;
    int32_t tag = 0;
    int32_t expire = -1;
    String_t text;
    if (!checkIntegerArg(x, args.getNext(), 0, 10000)
        || !checkIntegerArg(y, args.getNext(), 0, 10000)
        || !checkIntegerArg(type, args.getNext(), 0, 7 /*FIXME: NUM_USER_MARKERS-1*/)) {
        return;
    }
    checkIntegerArg(color, args.getNext(), 0, Drawing::NUM_USER_COLORS);
    checkStringArg(text, args.getNext());
    checkIntegerArg(tag, args.getNext(), 0, 0xFFFF);
    checkIntegerArg(expire, args.getNext(), -1, 0x7FFF);

    // Context check
    Game& g = game::actions::mustHaveGame(session);

    // Draw it
    std::auto_ptr<Drawing> drawing(new Drawing(game::map::Point(x, y), Drawing::MarkerDrawing));
    drawing->setMarkerKind(type);
    drawing->setColor(uint8_t(color));
    drawing->setTag(tag);
    drawing->setExpire(expire);
    drawing->setComment(text);

    g.currentTurn().universe().drawings().addNew(drawing.release());
}

/* @q History.ShowTurn nr:Int (Global Command)
   Show turn from history database.

   The parameter specifies the turn number to load.
   The special case "0" will load the current turn.
   PCC2 will load the specified turn's result file, if available.

   @since PCC2 2.40 */
void
game::interface::IFHistoryShowTurn(interpreter::Process& /*proc*/, game::Session& session, interpreter::Arguments& args)
{
    // Check parameters
    args.checkArgumentCount(1);
    int32_t turn;
    if (!interpreter::checkIntegerArg(turn, args.getNext(), 0, 10000)) {
        return;
    }

    // Do we have a game loaded?
    Root* r = session.getRoot().get();
    Game* g = session.getGame().get();
    if (g == 0 || r == 0) {
        throw Exception(Exception::eUser, Exception::eUser);
    }

    // Check turn number
    int currentTurn = g->currentTurn().getTurnNumber();
    if (turn == 0) {
        turn = currentTurn;
    }
    if (turn <= 0 || turn > currentTurn) {
        throw interpreter::Error::rangeError();
    }

    // Load if required
    if (turn < currentTurn) {
        // If turn is not known at all, update metainformation
        if (g->previousTurns().getTurnStatus(turn) == HistoryTurn::Unknown) {
            g->previousTurns().initFromTurnScores(g->scores(), turn, 1);
            if (TurnLoader* tl = r->getTurnLoader().get()) {
                g->previousTurns().initFromTurnLoader(*tl, *r, g->getViewpointPlayer(), turn, 1);
            }
        }

        // If turn is loadable, load
        HistoryTurn* ht = g->previousTurns().get(turn);
        if (ht != 0 && ht->isLoadable() && r->getTurnLoader().get() != 0) {
            // FIXME: code duplication to turnlistdialog.cpp
            try {
                afl::base::Ref<Turn> t = *new Turn();
                int player = g->getViewpointPlayer();
                r->getTurnLoader()->loadHistoryTurn(*t, *g, player, turn, *r);
                t->universe().postprocess(game::PlayerSet_t(player), game::PlayerSet_t(player), game::map::Object::ReadOnly,
                                          r->hostVersion(), r->hostConfiguration(),
                                          turn,
                                          session.translator(), session.log());
                ht->handleLoadSucceeded(t);
            }
            catch (std::exception& e) {
                ht->handleLoadFailed();
            }
        }

        if (ht == 0 || ht->getStatus() != HistoryTurn::Loaded) {
            throw interpreter::Error("Turn not available");
        }
    }

    // Do it
    g->setViewpointTurnNumber(turn);
}
