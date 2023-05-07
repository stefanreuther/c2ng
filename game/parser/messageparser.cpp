/**
  *  \file game/parser/messageparser.cpp
  *  \brief Class game::parser::MessageParser
  */

#include <algorithm>
#include "game/parser/messageparser.hpp"
#include "afl/io/textfile.hpp"
#include "afl/string/format.hpp"
#include "afl/string/parse.hpp"
#include "game/alliance/offer.hpp"
#include "game/parser/datainterface.hpp"
#include "game/parser/messageinformation.hpp"
#include "game/parser/messagetemplate.hpp"
#include "game/parser/messagevalue.hpp"
#include "util/string.hpp"

using afl::string::strTrim;
using game::alliance::Offer;

namespace {
    const char LOG_NAME[] = "game.parser.msgparser";

    /*
     *  load() helpers
     */

    /** Parse a Match instruction. Extracts an optional scope modifier to build the final opcode, and generates the instruction.
        - "+n,text": search n-th line below
        - "-n,text": search n-th line above
        - "=n,text": search line n
        \param tpl Message template
        \param opcode Base opcode
        \param line Line to process */
    void parseCheckInstruction(game::parser::MessageTemplate& tpl, uint8_t opcode, String_t line)
    {
        uint8_t scope = game::parser::MessageTemplate::sAny;
        int offset = 0;
        int scale  = 1;
        if (!line.empty()) {
            if (line[0] == '+') {
                scope = game::parser::MessageTemplate::sRelative;
            } else if (line[0] == '-') {
                scope = game::parser::MessageTemplate::sRelative;
                scale = -1;
            } else if (line[0] == '=') {
                scope = game::parser::MessageTemplate::sFixed;
            }
            if (scope != game::parser::MessageTemplate::sAny) {
                String_t::size_type i = 1;
                while (i < line.size() && line[i] >= '0' && line[i] <= '9') {
                    offset = 10*offset + line[i] - '0';
                    if (offset > 127) {
                        /* Overflow; go into "invalid" branch below */
                        i = line.size();
                        break;
                    }
                    ++i;
                }
                if (i < line.size() && line[i] == ',') {
                    /* It's valid, so finish it */
                    offset *= scale;
                    ++i;
                    while (i < line.size() && line[i] == ' ') {
                        ++i;
                    }
                    line.erase(0, i);
                } else {
                    /* Invalid, back out */
                    scope = game::parser::MessageTemplate::sAny;
                }
            }
            tpl.addCheckInstruction(uint8_t(opcode + scope), int8_t(offset), line);
        }
    }

    /** Check that the template so far is sensible and generate warnings.
        \param tpl Template
        \param tf  Text file we're reading from (used to obtain identification information for warnings) */
    void checkTemplate(const game::parser::MessageTemplate* tpl, afl::io::TextFile& tf, int startingLine, afl::string::Translator& tx, afl::sys::LogListener& log)
    {
        /* No problem if there is no current template */
        if (tpl == 0) {
            return;
        }

        /* Check number of variables */
        size_t nvar  = tpl->getNumVariables();
        size_t nwild = tpl->getNumWildcards();
        if (nvar != nwild) {
            log.write(log.Error, LOG_NAME, tf.getName(), startingLine,
                      afl::string::Format(tx.translateString("number of variables (%d) does not match number of produced values (%d)").c_str(),
                                          nvar, nwild));
        }

        /* Check number of restrictions */
        if (tpl->getNumRestrictions() == 0) {
            log.write(log.Error, LOG_NAME, tf.getName(), startingLine, tx.translateString("template will match every message"));
        }
    }

    /*
     *  parseMessage() helpers
     */

    /** Convert pre-parsed yes/no array into an array of offers. */
    void generateSimpleAllies(game::PlayerArray<Offer::Type>& out, String_t value)
    {
        int playerNr = 0;
        do {
            ++playerNr;

            String_t item = afl::string::strTrim(afl::string::strFirst(value, ","));
            if (util::stringMatch("Yes", item)) {
                out.set(playerNr, Offer::Yes);
            } else if (util::stringMatch("No", item)) {
                out.set(playerNr, Offer::No);
            } else if (util::stringMatch("Conditional", item)) {
                out.set(playerNr, Offer::Conditional);
            } else {
                // ignore (in particular, this branch is taken when the field is empty)
            }
        } while (afl::string::strRemove(value, ","));
    }

    /** Convert pre-parsed yes/no array into an array of offers, for FF allies.. */
    void generateFFAllies(game::PlayerArray<Offer::Type>& out, String_t value)
    {
        int playerNr = 0;
        do {
            ++playerNr;
            String_t item = afl::string::strTrim(afl::string::strFirst(value, ","));
            if (item == "YES") {
                out.set(playerNr, Offer::Yes);
            } else if (!item.empty()) {
                out.set(playerNr, Offer::No);
            } else {
                // ignore (in particular, this branch is taken when the field is empty)
            }
        } while (afl::string::strRemove(value, ","));
    }

    /** Generate allies from an array of "Race+!" elements. */
    void generateFlagAllies(Offer& out, String_t value, const game::parser::DataInterface& iface)
    {
        do {
            String_t item = afl::string::strFirst(value, ",");

            // Parse off the alliance markers
            bool excl = false;
            bool plus = false;
            size_t n = item.size();
            while (n > 0) {
                if (item[n-1] == '+') {
                    plus = true;
                    --n;
                } else if (item[n-1] == '!') {
                    excl = true;
                    --n;
                } else if (item[n-1] == ' ' || item[n-1] == ':') {
                    --n;
                } else {
                    break;
                }
            }

            // What remains is a race name, I hope.
            //   !     => this race has offered something to us
            //   +     => we have offered something
            item.erase(n);

            if (int player = iface.parseName(game::parser::DataInterface::AdjectiveRaceName, item)) {
                out.theirOffer.set(player, excl ? Offer::Yes : Offer::No);
                out.oldOffer.set(player,   plus ? Offer::Yes : Offer::No);
            }
        } while (afl::string::strRemove(value, ","));
    }


    /** Generate output for one matching message template.
        This generates a new MessageInformation record, or extends an existing one.
        \param values     [in] values produced by the template
        \param tpl        [in] template which matched
        \param turnNr     [in] message turn number
        \param info       [out] Information will be appended here */
    void generateOutput(const std::vector<String_t>& values,
                        const game::parser::MessageTemplate& tpl,
                        const game::parser::DataInterface& iface,
                        const int turnNr,
                        afl::container::PtrVector<game::parser::MessageInformation>& info,
                        afl::string::Translator& tx,
                        afl::sys::LogListener& log)
    {
        // ex game/msgparse.cc:generateOutput
        using namespace game::parser;

        /* Figure out process limit.
           We cannot process values that have no variable, nor variables without values. */
        const size_t processLimit = std::min(values.size(), tpl.getNumVariables());

        /* Figure out Id number. */
        int32_t id = 0;
        bool mergeable = false;
        size_t skipSlot = values.size()+1;
        switch (tpl.getMessageType()) {
         case MessageInformation::Ship:
         case MessageInformation::Minefield:
         case MessageInformation::Planet:
         case MessageInformation::Starbase:
         case MessageInformation::IonStorm:
         case MessageInformation::Ufo:
         case MessageInformation::Wormhole:
         case MessageInformation::ExtraShip:
         case MessageInformation::ExtraMinefield:
         case MessageInformation::ExtraPlanet:
            /* Those are identified by a mandatory Id */
            if (tpl.getVariableSlotByName("ID").get(skipSlot) && skipSlot < processLimit) {
                id = parseIntegerValue(values[skipSlot]);
            }
            if (!id) {
                /* Only complain about missing Id when we actually produced some data.
                   Some templates produce just an Id, "just in case", to associate the
                   message with an object or produce a marker. */
                if (values.size() > (skipSlot < values.size())) {
                    log.write(log.Error, LOG_NAME, afl::string::Format(tx.translateString("Message template \"%s\" did not produce Id number").c_str(), tpl.getTemplateName()));
                }
                return;
            }
            if (id <= 0) {
                // Change: PCC2 would have checked for upper bound ("ship Id > 999") as well
                log.write(log.Error, LOG_NAME, afl::string::Format(tx.translateString("Message template \"%s\" produced out-of-range Id %d, ignoring").c_str(), tpl.getTemplateName(), id));
                return;
            }
            mergeable = true;
            break;

         case MessageInformation::PlayerScore:
         case MessageInformation::Explosion:
            /* These can have an optional Id */
            if (tpl.getVariableSlotByName("ID").get(skipSlot) && skipSlot < processLimit) {
                id = parseIntegerValue(values[skipSlot]);
            }
            mergeable = (id != 0);
            break;

         case MessageInformation::Configuration:
            /* Always mergeable */
            mergeable = true;
            break;

         case MessageInformation::MarkerDrawing:
         case MessageInformation::CircleDrawing:
         case MessageInformation::LineDrawing:
         case MessageInformation::RectangleDrawing:
            /* Never mergeable */
            break;

         case MessageInformation::Alliance:
            /* This one is special, see below */
            break;

         case MessageInformation::NoObject:
            break;
        }

        /* Find out whether we can merge this item with the previous one. We can merge when
           the object kind permits merging, and they actually describe the same object. */
        MessageInformation* pInfo;
        if (mergeable
            && !info.empty()
            && info.back()->getObjectType() == tpl.getMessageType()
            && info.back()->getObjectId() == id
            && info.back()->getTurnNumber() == turnNr)
        {
            /* Merge this */
            pInfo = info.back();
        } else {
            /* Make new object */
            pInfo = info.pushBackNew(new MessageInformation(tpl.getMessageType(), id, turnNr));
        }

        /* Now produce the values */
        if (tpl.getMessageType() == MessageInformation::Alliance) {
            // Alliance case. Produce one alliance offer and a name.
            Offer offer;
            String_t idStr;

            for (size_t i = 0; i < processLimit; ++i) {
                const String_t varName = tpl.getVariableName(i);
                if (values[i].empty() || varName == "_" || varName.empty()) {
                    continue;
                }

                if (varName == "NAME") {
                    idStr = values[i];
                } else if (varName == "FROM") {
                    generateSimpleAllies(offer.theirOffer, values[i]);
                } else if (varName == "TO") {
                    generateSimpleAllies(offer.oldOffer, values[i]);
                } else if (varName == "FROMFF") {
                    generateFFAllies(offer.theirOffer, values[i]);
                } else if (varName == "TOFF") {
                    generateFFAllies(offer.oldOffer, values[i]);
                } else if (varName == "FLAGS") {
                    generateFlagAllies(offer, values[i], iface);
                } else {
                    log.write(log.Error, LOG_NAME, afl::string::Format(tx.translateString("Message template \"%s\" generates unknown value \"%s\"").c_str(),
                                                                       tpl.getTemplateName(), tpl.getVariableName(i)));
                }
            }

            if (idStr.empty()) {
                log.write(log.Error, LOG_NAME, afl::string::Format(tx.translateString("Message template \"%s\" did not produce name, ignoring").c_str(), tpl.getTemplateName()));
            } else {
                pInfo->addAllianceValue(idStr, offer);
            }
        } else {
            /* Regular case */
            for (size_t i = 0; i < processLimit; ++i) {
                /* Do not process empty values. Those are generated, in particular,
                   by array items. Also skip the Id field processed above. Finally,
                   skip values named "_" (match placeholders). */
                const String_t varName = tpl.getVariableName(i);
                if (values[i].empty() || i == skipSlot || varName == "_" || varName.empty()) {
                    continue;
                }

                MessageStringIndex msi;
                MessageIntegerIndex mii;
                if (tpl.getMessageType() == MessageInformation::Configuration) {
                    /* Configuration produces naked key/value pairs */
                    pInfo->addConfigurationValue(varName, values[i]);
                } else if (tpl.getMessageType() == MessageInformation::PlayerScore && varName == "SCORE") {
                    /* Score, this is an 11-element array */
                    String_t all = values[i];
                    int pl = 1;
                    String_t::size_type n;
                    while ((n = all.find(',')) != String_t::npos) {
                        if (n != 0) {
                            pInfo->addScoreValue(pl, parseIntegerValue(all.substr(0, n)));
                        }
                        all.erase(0, n+1);
                        ++pl;
                    }
                    if (!all.empty()) {
                        pInfo->addScoreValue(pl, parseIntegerValue(all));
                    }
                } else if ((msi = getStringIndexFromKeyword(varName)) != ms_Max) {
                    /* String value */
                    pInfo->addValue(msi, values[i]);
                } else if ((mii = getIntegerIndexFromKeyword(varName)) != mi_Max) {
                    /* Integer value */
                    pInfo->addValue(mii, parseIntegerValue(values[i]));
                } else if ((varName[0] == '+' || varName[0] == '-') && (mii = getIntegerIndexFromKeyword(varName.substr(1))) != mi_Max) {
                    /* Relative integer */
                    bool ok = false;
                    for (MessageInformation::Iterator_t it = pInfo->begin(); it != pInfo->end(); ++it) {
                        if (MessageIntegerValue_t* iv = dynamic_cast<MessageIntegerValue_t*>(*it)) {
                            if (iv->getIndex() == mii) {
                                int32_t delta = parseIntegerValue(values[i]);
                                if (varName[0] == '-') {
                                    delta = -delta;
                                }
                                iv->setValue(iv->getValue() + delta);
                                ok = true;
                                break;
                            }
                        }
                    }
                    if (!ok) {
                        log.write(log.Error, LOG_NAME, afl::string::Format(tx.translateString("Message template \"%s\" modifies value \"%s\" which does not exist").c_str(),
                                                                           tpl.getTemplateName(), tpl.getVariableName(i).substr(1)));
                    }
                } else {
                    /* What? */
                    log.write(log.Error, LOG_NAME, afl::string::Format(tx.translateString("Message template \"%s\" generates unknown value \"%s\"").c_str(),
                                                                       tpl.getTemplateName(), tpl.getVariableName(i)));
                }
            }
        }
    }
}

/*
 *  Class MessageParser
 */

// Default constructor.
game::parser::MessageParser::MessageParser()
    : m_templates()
{
    // ex GMessageParser::GMessageParser
}

// Destructor.
game::parser::MessageParser::~MessageParser()
{ }

// Load definitions from file.
void
game::parser::MessageParser::load(afl::io::Stream& file, afl::string::Translator& tx, afl::sys::LogListener& log)
{
    // ex GMessageParser::load
    afl::io::TextFile tf(file);
    String_t line;
    MessageTemplate* currentTemplate = 0;
    int currentTemplateLine = 0;

    while (tf.readLine(line)) {
        line = strTrim(line);
        if (line.empty() || line[0] == ';') {
            continue;
        }

        String_t::size_type p = line.find_first_of("=,");
        if (p == String_t::npos) {
            log.write(log.Error, LOG_NAME, file.getName(), tf.getLineNumber(), tx.translateString("missing delimiter"));
        } else if (line[p] == ',') {
            /* Check old template */
            checkTemplate(currentTemplate, tf, currentTemplateLine, tx, log);

            /* Start a new template */
            MessageInformation::Type mo;
            String_t kind = strTrim(line.substr(0, p));
            if (util::stringMatch("Minefield", kind)) {
                mo = MessageInformation::Minefield;
            } else if (util::stringMatch("Planet", kind)) {
                mo = MessageInformation::Planet;
            } else if (util::stringMatch("Base", kind)) {
                mo = MessageInformation::Starbase;
            } else if (util::stringMatch("PLAYerscore", kind)) {
                mo = MessageInformation::PlayerScore;
            } else if (util::stringMatch("Ship", kind)) {
                mo = MessageInformation::Ship;
            } else if (util::stringMatch("Ionstorm", kind)) {
                mo = MessageInformation::IonStorm;
            } else if (util::stringMatch("Configuration", kind)) {
                mo = MessageInformation::Configuration;
            } else if (util::stringMatch("Explosion", kind)) {
                mo = MessageInformation::Explosion;
            } else if (util::stringMatch("Alliance", kind)) {
                mo = MessageInformation::Alliance;
            } else if (util::stringMatch("Wormhole", kind)) {
                mo = MessageInformation::Wormhole;
            } else if (util::stringMatch("Ufo", kind)) {
                mo = MessageInformation::Ufo;
            } else if (util::stringMatch("MArker", kind)) {
                mo = MessageInformation::MarkerDrawing;
            } else if (util::stringMatch("Line", kind)) {
                mo = MessageInformation::LineDrawing;
            } else if (util::stringMatch("Rectangle", kind)) {
                mo = MessageInformation::RectangleDrawing;
            } else if (util::stringMatch("CIrcle", kind)) {
                mo = MessageInformation::CircleDrawing;
            } else if (util::stringMatch("EXTRAShip", kind)) {
                mo = MessageInformation::ExtraShip;
            } else if (util::stringMatch("EXTRAPlanet", kind)) {
                mo = MessageInformation::ExtraPlanet;
            } else if (util::stringMatch("EXTRAMinefield", kind)) {
                mo = MessageInformation::ExtraMinefield;
            } else {
                mo = MessageInformation::NoObject;
            }

            if (mo == MessageInformation::NoObject) {
                log.write(log.Error, LOG_NAME, file.getName(), tf.getLineNumber(), tx.translateString("unknown object kind"));
                currentTemplate = 0;
            } else {
                currentTemplate = m_templates.pushBackNew(new MessageTemplate(mo, strTrim(line.substr(p+1))));
                currentTemplateLine = tf.getLineNumber();
            }
        } else {
            /* Assignments */
            if (!currentTemplate) {
                continue;
            }

            String_t lhs = strTrim(line.substr(0, p));
            line = strTrim(line.substr(p+1));
            if (util::stringMatch("KInd", lhs)) {
                if (line.size()) {
                    currentTemplate->addMatchInstruction(MessageTemplate::iMatchKind, line[0]);
                }
            } else if (util::stringMatch("SUbid", lhs)) {
                if (line.size()) {
                    currentTemplate->addMatchInstruction(MessageTemplate::iMatchSubId, line[0]);
                }
            } else if (util::stringMatch("BIgid", lhs)) {
                uint16_t n;
                if (afl::string::strToInteger(line, n)) {
                    currentTemplate->addMatchInstruction(MessageTemplate::iMatchBigId, n);
                } else {
                    log.write(log.Error, LOG_NAME, file.getName(), tf.getLineNumber(), tx.translateString("invalid number"));
                }
            } else if (util::stringMatch("CHeck", lhs)) {
                parseCheckInstruction(*currentTemplate, MessageTemplate::iCheck, line);
            } else if (util::stringMatch("FAil", lhs)) {
                parseCheckInstruction(*currentTemplate, MessageTemplate::iFail, line);
            } else if (util::stringMatch("FInd", lhs)) {
                parseCheckInstruction(*currentTemplate, MessageTemplate::iFind, line);
            } else if (util::stringMatch("PArse", lhs)) {
                parseCheckInstruction(*currentTemplate, MessageTemplate::iParse, line);
            } else if (util::stringMatch("ARray", lhs)) {
                parseCheckInstruction(*currentTemplate, MessageTemplate::iArray, line);
            } else if (util::stringMatch("VAlues", lhs)) {
                currentTemplate->addValueInstruction(MessageTemplate::iValue, line);
            } else if (util::stringMatch("ASsign", lhs)) {
                currentTemplate->addVariables(line);
            } else if (util::stringMatch("COntinue", lhs)) {
                currentTemplate->setContinueFlag(util::stringMatch("Yes", line));
            } else {
                log.write(log.Warn, LOG_NAME, file.getName(), tf.getLineNumber(), tx.translateString("unknown keyword"));
            }
        }
    }
    checkTemplate(currentTemplate, tf, currentTemplateLine, tx, log);
}

// Parse a message, main entry point.
void
game::parser::MessageParser::parseMessage(String_t theMessage, const DataInterface& iface, int turnNr, afl::container::PtrVector<MessageInformation>& info,
                                          afl::string::Translator& tx, afl::sys::LogListener& log)
{
    // ex GMessageParser::parseMessage
    // Split message into lines
    MessageLines_t lines;
    splitMessage(lines, theMessage);

    // Parse all templates and gather information
    for (afl::container::PtrVector<MessageTemplate>::iterator i = m_templates.begin(); i != m_templates.end(); ++i) {
        std::vector<String_t> values;
        if ((*i)->match(lines, iface, values)) {
            // Matches. Produce output.
            generateOutput(values, **i, iface, turnNr - getMessageHeaderInformation(lines, MsgHdrAge), info, tx, log);
            if (!(*i)->getContinueFlag()) {
                break;
            }
        }
    }
}
