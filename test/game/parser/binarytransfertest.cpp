/**
  *  \file test/game/parser/binarytransfertest.cpp
  *  \brief Test for game::parser::BinaryTransfer
  */

#include "game/parser/binarytransfer.hpp"

#include "afl/charset/codepage.hpp"
#include "afl/charset/codepagecharset.hpp"
#include "afl/test/testrunner.hpp"
#include "game/map/drawing.hpp"
#include "game/map/minefield.hpp"
#include "game/map/planet.hpp"
#include "game/parser/messagetemplate.hpp"
#include "game/score/scoreid.hpp"

using game::Element;
using game::map::Minefield;
using game::map::Drawing;
using game::map::Point;
using game::parser::MessageInformation;
using afl::container::PtrVector;

namespace {
    template<typename Index> struct Ret;
    template<> struct Ret<game::parser::MessageIntegerIndex> { typedef int32_t Result_t; };
    template<> struct Ret<game::parser::MessageStringIndex>  { typedef String_t Result_t; };

    class Finder {
     public:
        Finder(const PtrVector<MessageInformation>& info, MessageInformation::Type type, int id, int turnNumber)
            : m_info(info), m_type(type), m_id(id), m_turnNumber(turnNumber)
            { }

        const MessageInformation* find() const
            {
                for (size_t i = 0; i < m_info.size(); ++i) {
                    if (match(m_info[i])) {
                        return m_info[i];
                    }
                }
                return 0;
            }

        size_t count() const
            {
                size_t n = 0;
                for (size_t i = 0; i < m_info.size(); ++i) {
                    if (match(m_info[i])) {
                        n += (m_info[i]->end() - m_info[i]->begin());
                    }
                }
                return n;
            }

        template<typename Index>
        afl::base::Optional<typename Ret<Index>::Result_t> getValue(Index idx)
            {
                for (size_t i = 0; i < m_info.size(); ++i) {
                    if (match(m_info[i])) {
                        afl::base::Optional<typename Ret<Index>::Result_t> v = m_info[i]->getValue(idx);
                        if (v.isValid()) {
                            return v;
                        }
                    }
                }
                return afl::base::Nothing;
            }

        bool getScoreValue(int player, int& result)
            {
                for (size_t i = 0; i < m_info.size(); ++i) {
                    if (match(m_info[i])) {
                        for (MessageInformation::Iterator_t it = m_info[i]->begin(); it != m_info[i]->end(); ++it) {
                            if (game::parser::MessageScoreValue_t* scv = dynamic_cast<game::parser::MessageScoreValue_t*>(*it)) {
                                if (scv->getIndex() == player) {
                                    result = scv->getValue();
                                    return true;
                                }
                            }
                        }
                    }
                }
                return false;
            }

     private:
        bool match(const MessageInformation* p) const
            { return p->getObjectType() == m_type && p->getObjectId() == m_id && p->getTurnNumber() == m_turnNumber; }

        const PtrVector<MessageInformation>& m_info;
        MessageInformation::Type m_type;
        int m_id;
        int m_turnNumber;
    };

}


/** Test packBinaryMinefield(). */
AFL_TEST("game.parser.BinaryTransfer:packBinaryMinefield", a)
{
    Minefield mf(61);
    mf.addReport(Point(2635, 1818),             // center
                 3,                             // owner
                 Minefield::IsMine,             // type report
                 Minefield::UnitsKnown,         // size report
                 11416, 46,                     // size value
                 Minefield::MinefieldScanned);  // reason

    a.checkEqual("01", game::parser::packBinaryMinefield(mf),
                 "<<< VPA Data Transmission >>>\n\n"
                 "OBJECT: Mine field 61\n"
                 "DATA: 2094989326\n"
                 "ocaalekakbhadaaaijmcaaaaaaaa\n");
}

/** Test packBinaryDrawing(), marker. */
AFL_TEST("game.parser.BinaryTransfer:packBinaryDrawing:MarkerDrawing", a)
{
    Drawing d(Point(2060, 1934), Drawing::MarkerDrawing);
    d.setColor(11);             // blue, serialized as color #1
    d.setMarkerKind(1);         // flag
    d.setComment("flag");

    afl::charset::CodepageCharset cs(afl::charset::g_codepageLatin1);

    a.checkEqual("01", game::parser::packBinaryDrawing(d, cs),
                 "<<< VPA Data Transmission >>>\n\n"
                 "OBJECT: Marker\n"
                 "DATA: -1748500463\n"
                 "babamaiaoihaaaaagaaabacaeaggmgbghg\n");

    // This one exercises the line length limit:
    d.setComment("comment");
    a.checkEqual("02", game::parser::packBinaryDrawing(d, cs),
                 "<<< VPA Data Transmission >>>\n\n"
                 "OBJECT: Marker\n"
                 "DATA: -1792344044\n"
                 "babamaiaoihaaaaagaaabacahadgpgngngfgogeh\n");
}

/** Test packBinaryDrawing(), line. */
AFL_TEST("game.parser.BinaryTransfer:packBinaryDrawing:LineDrawing", a)
{
    Drawing d(Point(1304, 1794), Drawing::LineDrawing);
    d.setColor(21);             // light blue, serialized as color #9
    d.setPos2(Point(1359, 1744));

    afl::charset::CodepageCharset cs(afl::charset::g_codepageLatin1);

    a.checkEqual("01", game::parser::packBinaryDrawing(d, cs),
                 "<<< VPA Data Transmission >>>\n\n"
                 "OBJECT: Marker\n"
                 "DATA: 887422989\n"
                 "iajaibfacahaaaaahdaaomppaa\n");
}

/** Test packBinaryDrawing(), circle. */
AFL_TEST("game.parser.BinaryTransfer:packBinaryDrawing:CircleDrawing", a)
{
    Drawing d(Point(1876, 2575), Drawing::CircleDrawing);
    d.setColor(24);             // light red, serialized as color #12
    d.setCircleRadius(50);

    afl::charset::CodepageCharset cs(afl::charset::g_codepageLatin1);

    a.checkEqual("01", game::parser::packBinaryDrawing(d, cs),
                 "<<< VPA Data Transmission >>>\n\n"
                 "OBJECT: Marker\n"
                 "DATA: -861470707\n"
                 "hamaefhapakaaaaaaaaacdaaaa\n");
}

/** Test packBinaryDrawing(), rectangle (transmitted as dotted-line). */
AFL_TEST("game.parser.BinaryTransfer:packBinaryDrawing:RectangleDrawing", a)
{
    Drawing d(Point(2336, 2328), Drawing::RectangleDrawing);
    d.setColor(2);             // light gray, serialized as color #7
    d.setPos2(Point(2432, 2391));

    afl::charset::CodepageCharset cs(afl::charset::g_codepageLatin1);

    a.checkEqual("01", game::parser::packBinaryDrawing(d, cs),
                 "<<< VPA Data Transmission >>>\n\n"
                 "OBJECT: Marker\n"
                 "DATA: 291176461\n"
                 "jahaacjaibjaaaaaagaapdaaaa\n");
}

/** Test packBinaryPlanet(). */
AFL_TEST("game.parser.BinaryTransfer:packBinaryPlanet", a)
{
    game::map::Planet pl(402);

    afl::charset::CodepageCharset cs(afl::charset::g_codepageLatin1);
    game::HostVersion host(game::HostVersion::PHost, MKVERSION(4,0,0));

    // Feed in the turn numbers using message information
    game::parser::MessageInformation info(game::parser::MessageInformation::Planet, 402, 46);
    info.addValue(game::parser::mi_Owner, 6);          // sets ColonistTime
    info.addValue(game::parser::mi_PlanetMinedN, 59);  // sets MineralTime
    info.addValue(game::parser::mi_PlanetCash, 0);     // sets CashTime
    pl.addMessageInformation(info);

    // Populate object normally
    pl.setOwner(6);
    pl.setFriendlyCode(String_t("f*p"));
    pl.setNumBuildings(game::MineBuilding, 16);
    pl.setNumBuildings(game::FactoryBuilding, 16);
    pl.setNumBuildings(game::DefenseBuilding, 15);
    pl.setCargo(Element::Neutronium, 59);
    pl.setCargo(Element::Tritanium, 6);
    pl.setCargo(Element::Duranium, 23);
    pl.setCargo(Element::Molybdenum, 20);
    pl.setCargo(Element::Colonists, 17);
    pl.setCargo(Element::Supplies, 22);
    pl.setCargo(Element::Money, 0);
    pl.setOreGround(Element::Neutronium, 235);
    pl.setOreGround(Element::Tritanium, 2711);
    pl.setOreGround(Element::Duranium, 321);
    pl.setOreGround(Element::Molybdenum, 479);
    pl.setOreDensity(Element::Neutronium, 93);
    pl.setOreDensity(Element::Tritanium, 21);
    pl.setOreDensity(Element::Duranium, 75);
    pl.setOreDensity(Element::Molybdenum, 65);
    pl.setColonistTax(0);
    pl.setColonistHappiness(100);
    pl.setNativeTax(0);
    pl.setNativeHappiness(100);
    pl.setNativeGovernment(0);
    pl.setNatives(0);
    pl.setNativeRace(0);
    pl.setTemperature(54);
    pl.setBuildBaseFlag(false);

    a.checkEqual("01", game::parser::packBinaryPlanet(pl, cs, host),
                 // Original testcase generated with VPA.
                 // Turns out we're smarter populating the EPln section, so we're not binary identical.
                 // "<<< VPA Data Transmission >>>\n\n"
                 // "OBJECT: Planet 402\n"
                 // "DATA: -1515519909\n"
                 // "ocaaocaagaaaggkcahabaaabaapaaaldaaaaaaga\n"
                 // "aaaaaahbaaaaaaebaaaaaabbaaaaaagbaaaaaaaa\n"
                 // "aaaaaaloaaaaaahjkaaaaabebaaaaapnbaaaaanf\n"
                 // "aafbaaleaabeaaaaaaaaaaegaaegaaaaaaaaaaaa\n"
                 // "aaaaaaocaaaaaaaaaagaaa\n"

                 // Updated test-case:
                 "<<< VPA Data Transmission >>>\n\n"
                 "OBJECT: Planet 402\n"
                 "DATA: -1172504485\n"
                 "ocaaocaagaaaggkcahabaaabaapaaaldaaaaaaga\n"
                 "aaaaaahbaaaaaaebaaaaaabbaaaaaagbaaaaaaaa\n"
                 "aaaaaaloaaaaaahjkaaaaabebaaaaapnbaaaaanf\n"
                 "aafbaaleaabeaaaaaaaaaaegaaegaaaaaaaaaaaa\n"
                 "aaaaaaocaaaaaaocaagace\n"
                 //             ^^^^  ^^ difference in scanTurn, flags
        );
}

/** Test unpackBinaryMessage(), minefield. */
AFL_TEST("game.parser.BinaryTransfer:unpackBinaryMessage:Minefield", a)
{
    // Message from testPackBinaryMinefield(): 2635,1818; 11416 units, turn 46
    const String_t msg[] = {
        "<<< VPA Data Transmission >>>",
        "",
        "OBJECT: Mine field 61",
        "DATA: 2094989326",
        "ocaalekakbhadaaaijmcaaaaaaaa"
    };
    afl::charset::CodepageCharset cs(afl::charset::g_codepageLatin1);
    PtrVector<MessageInformation> info;
    a.check("01. unpackBinaryMessage", game::parser::unpackBinaryMessage(msg, 99, info, cs) == std::make_pair(game::parser::UnpackSuccess, game::parser::MinefieldMessage));

    // Must have produced at least one result
    Finder f(info, MessageInformation::Minefield, 61, 46 /* min(46,99) */);
    a.checkNonNull("11. find", f.find());

    // Verify values
    a.checkEqual("21. mi_X",         f.getValue(game::parser::mi_X).orElse(-1), 2635);
    a.checkEqual("22. mi_Y",         f.getValue(game::parser::mi_Y).orElse(-1), 1818);
    a.checkEqual("23. mi_MineUnits", f.getValue(game::parser::mi_MineUnits).orElse(-1), 11416);
    a.checkEqual("24. mi_Owner",     f.getValue(game::parser::mi_Owner).orElse(-1), 3);
    a.checkEqual("25. mi_Type",      f.getValue(game::parser::mi_Type).orElse(-1), 0);
}

/** Test unpackBinaryMessage(), planet. */
AFL_TEST("game.parser.BinaryTransfer:unpackBinaryMessage:Planet", a)
{
    // Original message from testPackBinaryPlanet()
    const String_t msg[] = {
        "<<< VPA Data Transmission >>>",
        "",
        "OBJECT: Planet 402",
        "DATA: -1515519909",
        "ocaaocaagaaaggkcahabaaabaapaaaldaaaaaaga",
        "aaaaaahbaaaaaaebaaaaaabbaaaaaagbaaaaaaaa",
        "aaaaaaloaaaaaahjkaaaaabebaaaaapnbaaaaanf",
        "aafbaaleaabeaaaaaaaaaaegaaegaaaaaaaaaaaa",
        "aaaaaaocaaaaaaaaaagaaa",
    };
    afl::charset::CodepageCharset cs(afl::charset::g_codepageLatin1);
    afl::container::PtrVector<game::parser::MessageInformation> info;
    a.check("01. unpackBinaryMessage", game::parser::unpackBinaryMessage(msg, 99, info, cs) == std::make_pair(game::parser::UnpackSuccess, game::parser::PlanetMessage));

    // Must have produced at least one result
    Finder f(info, MessageInformation::Planet, 402, 46 /* min(46,99) */);
    a.checkNonNull("11. find", f.find());

    // Verify values
    a.checkEqual("21. mi_Owner",                   f.getValue(game::parser::mi_Owner).orElse(-1), 6);
    a.checkEqual("22. ms_FriendlyCode",            f.getValue(game::parser::ms_FriendlyCode).orElse(""), "f*p");
    a.checkEqual("23. mi_PlanetMines",             f.getValue(game::parser::mi_PlanetMines).orElse(-1), 16);
    a.checkEqual("24. mi_PlanetFactories",         f.getValue(game::parser::mi_PlanetFactories).orElse(-1), 16);
    a.checkEqual("25. mi_PlanetDefense",           f.getValue(game::parser::mi_PlanetDefense).orElse(-1), 15);
    a.checkEqual("26. mi_PlanetMinedN",            f.getValue(game::parser::mi_PlanetMinedN).orElse(-1), 59);
    a.checkEqual("27. mi_PlanetMinedT",            f.getValue(game::parser::mi_PlanetMinedT).orElse(-1), 6);
    a.checkEqual("28. mi_PlanetMinedD",            f.getValue(game::parser::mi_PlanetMinedD).orElse(-1), 23);
    a.checkEqual("29. mi_PlanetMinedM",            f.getValue(game::parser::mi_PlanetMinedM).orElse(-1), 20);
    a.checkEqual("30. mi_PlanetColonists",         f.getValue(game::parser::mi_PlanetColonists).orElse(-1), 17);
    a.checkEqual("31. mi_PlanetSupplies",          f.getValue(game::parser::mi_PlanetSupplies).orElse(-1), 22);
    a.checkEqual("32. mi_PlanetCash",              f.getValue(game::parser::mi_PlanetCash).orElse(-1), 0);
    a.checkEqual("33. mi_PlanetTotalN",            f.getValue(game::parser::mi_PlanetTotalN).orElse(-1), 235);
    a.checkEqual("34. mi_PlanetTotalT",            f.getValue(game::parser::mi_PlanetTotalT).orElse(-1), 2711);
    a.checkEqual("35. mi_PlanetTotalD",            f.getValue(game::parser::mi_PlanetTotalD).orElse(-1), 321);
    a.checkEqual("36. mi_PlanetTotalM",            f.getValue(game::parser::mi_PlanetTotalM).orElse(-1), 479);
    a.checkEqual("37. mi_PlanetDensityN",          f.getValue(game::parser::mi_PlanetDensityN).orElse(-1), 93);
    a.checkEqual("38. mi_PlanetDensityT",          f.getValue(game::parser::mi_PlanetDensityT).orElse(-1), 21);
    a.checkEqual("39. mi_PlanetDensityD",          f.getValue(game::parser::mi_PlanetDensityD).orElse(-1), 75);
    a.checkEqual("40. mi_PlanetDensityM",          f.getValue(game::parser::mi_PlanetDensityM).orElse(-1), 65);
    a.checkEqual("41. mi_PlanetColonistTax",       f.getValue(game::parser::mi_PlanetColonistTax).orElse(-1), 0);
    a.checkEqual("42. mi_PlanetColonistHappiness", f.getValue(game::parser::mi_PlanetColonistHappiness).orElse(-1), 100);
    a.checkEqual("43. mi_PlanetNativeTax",         f.getValue(game::parser::mi_PlanetNativeTax).orElse(-1), 0);
    a.checkEqual("44. mi_PlanetNativeGov",         f.getValue(game::parser::mi_PlanetNativeGov).orElse(-1), 0);
    a.checkEqual("45. mi_PlanetNatives",           f.getValue(game::parser::mi_PlanetNatives).orElse(-1), 0);
    a.checkEqual("46. mi_PlanetNativeRace",        f.getValue(game::parser::mi_PlanetNativeRace).orElse(-1), 0);
    a.checkEqual("47. mi_PlanetNativeHappiness",   f.getValue(game::parser::mi_PlanetNativeHappiness).orElse(-1), 100);
    a.checkEqual("48. mi_PlanetTemperature",       f.getValue(game::parser::mi_PlanetTemperature).orElse(-1), 54);

    // No information about base in report
    a.check("51. mi_PlanetHasBase", !f.getValue(game::parser::mi_PlanetHasBase).isValid());
}

/** Test unpackBinaryMessage(), planet which has only sensor sweep. */
AFL_TEST("game.parser.BinaryTransfer:unpackBinaryMessage:Planet:sensor-sweep", a)
{
    const String_t msg[] = {
        "<<< VPA Data Transmission >>>",
        "",
        "OBJECT: Planet 305",
        "DATA: -1070989221",
        "PPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPP",
        "PPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPP",
        "PPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPP",
        "PPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPP",
        "PPPPPPPPPPPPPPocaadafa",
    };
    afl::charset::CodepageCharset cs(afl::charset::g_codepageLatin1);
    afl::container::PtrVector<game::parser::MessageInformation> info;
    a.check("01. unpackBinaryMessage", game::parser::unpackBinaryMessage(msg, 99, info, cs) == std::make_pair(game::parser::UnpackSuccess, game::parser::PlanetMessage));

    // Must have produced at least one result
    Finder f(info, MessageInformation::Planet, 305, 46 /* min(46,99) */);
    a.checkNonNull("11. find", f.find());

    // Verify values
    a.checkEqual("21. mi_Owner", f.getValue(game::parser::mi_Owner).orElse(-1), 3);
    a.checkEqual("22. mi_PlanetActivity", f.getValue(game::parser::mi_PlanetActivity).orElse(-1), 4);

    // Nothing else
    a.checkEqual("31", f.count(), 2U);

    // Therefore everything else reports not-found
    a.check("41. ms_FriendlyCode", !f.getValue(game::parser::ms_FriendlyCode).isValid());
    a.check("42. mi_PlanetMines", !f.getValue(game::parser::mi_PlanetMines).isValid());
    a.check("43. mi_PlanetColonists", !f.getValue(game::parser::mi_PlanetColonists).isValid());
    a.check("44. mi_PlanetMinedM", !f.getValue(game::parser::mi_PlanetMinedM).isValid());
    a.check("45. mi_PlanetHasBase", !f.getValue(game::parser::mi_PlanetHasBase).isValid());
}

/** Test unpackBinaryMessage(), marker. */
AFL_TEST("game.parser.BinaryTransfer:unpackBinaryMessage:MarkerDrawing", a)
{
    const String_t msg[] = {
        "<<< VPA Data Transmission >>>",
        "",
        "OBJECT: Marker",
        "DATA: -1748500463",
        "babamaiaoihaaaaagaaabacaeaggmgbghg"
    };
    afl::charset::CodepageCharset cs(afl::charset::g_codepageLatin1);
    PtrVector<MessageInformation> info;
    a.check("01. unpackBinaryMessage", game::parser::unpackBinaryMessage(msg, 99, info, cs) == std::make_pair(game::parser::UnpackSuccess, game::parser::DrawingMessage));

    // Must have produced exactly one result (otherwise it will create multiple markers)
    a.checkEqual("11. size", info.size(), 1U);
    a.checkNonNull("12. info", info[0]);
    a.checkEqual("13. getObjectType", info[0]->getObjectType(), MessageInformation::MarkerDrawing);

    // Verify
    a.checkEqual("21. mi_X",              info[0]->getValue(game::parser::mi_X).orElse(-1), 2060);
    a.checkEqual("22. mi_Y",              info[0]->getValue(game::parser::mi_Y).orElse(-1), 1934);
    a.checkEqual("23. mi_Color",          info[0]->getValue(game::parser::mi_Color).orElse(-1), 11);
    a.checkEqual("24. mi_DrawingShape",   info[0]->getValue(game::parser::mi_DrawingShape).orElse(-1), 1);
    a.checkEqual("25. mi_DrawingExpire",  info[0]->getValue(game::parser::mi_DrawingExpire).orElse(-99), -1);

    a.checkEqual("31. ms_DrawingComment", info[0]->getValue(game::parser::ms_DrawingComment).orElse(""), "flag");
}

/** Test unpackBinaryMessage(), line. */
AFL_TEST("game.parser.BinaryTransfer:unpackBinaryMessage:LineDrawing", a)
{
    const String_t msg[] = {
        "<<< VPA Data Transmission >>>",
        "",
        "OBJECT: Marker",
        "DATA: 887422989",
        "iajaibfacahaaaaahdaaomppaa",
    };
    afl::charset::CodepageCharset cs(afl::charset::g_codepageLatin1);
    PtrVector<MessageInformation> info;
    a.check("01. unpackBinaryMessage", game::parser::unpackBinaryMessage(msg, 99, info, cs) == std::make_pair(game::parser::UnpackSuccess, game::parser::DrawingMessage));

    // Must have produced exactly one result (otherwise it will create multiple markers)
    a.checkEqual("11. size", info.size(), 1U);
    a.checkNonNull("12. info", info[0]);
    a.checkEqual("13. getObjectType", info[0]->getObjectType(), MessageInformation::LineDrawing);

    // Verify
    a.checkEqual("21. mi_X",     info[0]->getValue(game::parser::mi_X).orElse(-1), 1304);
    a.checkEqual("22. mi_Y",     info[0]->getValue(game::parser::mi_Y).orElse(-1), 1794);
    a.checkEqual("23. mi_EndX",  info[0]->getValue(game::parser::mi_EndX).orElse(-1), 1359);
    a.checkEqual("24. mi_EndY",  info[0]->getValue(game::parser::mi_EndY).orElse(-1), 1744);
    a.checkEqual("25. mi_Color", info[0]->getValue(game::parser::mi_Color).orElse(-1), 21);

    a.check("31. ms_DrawingComment", !info[0]->getValue(game::parser::ms_DrawingComment).isValid());
}

/** Test unpackBinaryMessage(), circle. */
AFL_TEST("game.parser.BinaryTransfer:unpackBinaryMessage:CircleDrawing", a)
{
    const String_t msg[] = {
        "<<< VPA Data Transmission >>>",
        "",
        "OBJECT: Marker",
        "DATA: -861470707",
        "hamaefhapakaaaaaaaaacdaaaa",
    };
    afl::charset::CodepageCharset cs(afl::charset::g_codepageLatin1);
    PtrVector<MessageInformation> info;
    a.check("01. unpackBinaryMessage", game::parser::unpackBinaryMessage(msg, 99, info, cs) == std::make_pair(game::parser::UnpackSuccess, game::parser::DrawingMessage));

    // Must have produced exactly one result (otherwise it will create multiple markers)
    a.checkEqual("11. size", info.size(), 1U);
    a.checkNonNull("12. info", info[0]);
    a.checkEqual("13. getObjectType", info[0]->getObjectType(), MessageInformation::CircleDrawing);

    // Verify
    a.checkEqual("21. mi_X",      info[0]->getValue(game::parser::mi_X).orElse(-1), 1876);
    a.checkEqual("22. mi_Y",      info[0]->getValue(game::parser::mi_Y).orElse(-1), 2575);
    a.checkEqual("23. mi_Radius", info[0]->getValue(game::parser::mi_Radius).orElse(-1), 50);
    a.checkEqual("24. mi_Color",  info[0]->getValue(game::parser::mi_Color).orElse(-1), 24);

    a.check("31. ms_DrawingComment", !info[0]->getValue(game::parser::ms_DrawingComment).isValid());
}

/** Test unpackBinaryMessage(), rectangle. */
AFL_TEST("game.parser.BinaryTransfer:unpackBinaryMessage:RectangleDrawing", a)
{
    const String_t msg[] = {
        "<<< VPA Data Transmission >>>",
        "",
        "OBJECT: Marker",
        "DATA: 291176461",
        "jahaacjaibjaaaaaagaapdaaaa",
    };
    afl::charset::CodepageCharset cs(afl::charset::g_codepageLatin1);
    PtrVector<MessageInformation> info;
    a.check("01. unpackBinaryMessage", game::parser::unpackBinaryMessage(msg, 99, info, cs) == std::make_pair(game::parser::UnpackSuccess, game::parser::DrawingMessage));

    // Must have produced exactly one result (otherwise it will create multiple markers)
    a.checkEqual("11. size", info.size(), 1U);
    a.checkNonNull("12. info", info[0]);
    a.checkEqual("13. getObjectType", info[0]->getObjectType(), MessageInformation::RectangleDrawing);

    // Verify
    a.checkEqual("21. mi_X",     info[0]->getValue(game::parser::mi_X).orElse(-1), 2336);
    a.checkEqual("22. mi_Y",     info[0]->getValue(game::parser::mi_Y).orElse(-1), 2328);
    a.checkEqual("23. mi_EndX",  info[0]->getValue(game::parser::mi_EndX).orElse(-1), 2432);
    a.checkEqual("24. mi_EndY",  info[0]->getValue(game::parser::mi_EndY).orElse(-1), 2391);
    a.checkEqual("25. mi_Color", info[0]->getValue(game::parser::mi_Color).orElse(-1), 2);

    a.check("31. ms_DrawingComment", !info[0]->getValue(game::parser::ms_DrawingComment).isValid());
}

/** Test that we can correctly transmit all drawing colors. */
AFL_TEST("game.parser.BinaryTransfer:drawing-colors", a)
{
    afl::charset::CodepageCharset cs(afl::charset::g_codepageLatin1);
    for (int i = 0; i <= Drawing::NUM_USER_COLORS; ++i) {
        // Drawing
        Drawing d(Point(1000, 1000), Drawing::MarkerDrawing);
        d.setMarkerKind(2);
        d.setColor(static_cast<uint8_t>(i));

        // Encode
        String_t msg = game::parser::packBinaryDrawing(d, cs);

        // Decode
        game::parser::MessageLines_t msgLines;
        game::parser::splitMessage(msgLines, msg);
        PtrVector<MessageInformation> info;
        a.check("01. unpackBinaryMessage", game::parser::unpackBinaryMessage(msgLines, 99, info, cs) == std::make_pair(game::parser::UnpackSuccess, game::parser::DrawingMessage));

        // Verify
        a.checkEqual("11. size", info.size(), 1U);
        a.checkNonNull("12. info", info[0]);
        a.checkEqual("13. getObjectType", info[0]->getObjectType(), MessageInformation::MarkerDrawing);
        a.checkEqual("14. mi_Color", info[0]->getValue(game::parser::mi_Color).orElse(-1), i);
    }
}

/** Test that we can correctly transmit all marker shapes. */
AFL_TEST("game.parser.BinaryTransfer:drawing-shapes", a)
{
    afl::charset::CodepageCharset cs(afl::charset::g_codepageLatin1);
    for (int i = 0; i < Drawing::NUM_USER_MARKERS; ++i) {
        // Drawing
        Drawing d(Point(1000, 1000), Drawing::MarkerDrawing);
        d.setMarkerKind(i);

        // Encode
        String_t msg = game::parser::packBinaryDrawing(d, cs);

        // Decode
        game::parser::MessageLines_t msgLines;
        game::parser::splitMessage(msgLines, msg);
        PtrVector<MessageInformation> info;
        a.check("01. unpackBinaryMessage", game::parser::unpackBinaryMessage(msgLines, 99, info, cs) == std::make_pair(game::parser::UnpackSuccess, game::parser::DrawingMessage));

        // Verify
        a.checkEqual("11. size", info.size(), 1U);
        a.checkNonNull("12. info", info[0]);
        a.checkEqual("13. getObjectType", info[0]->getObjectType(), MessageInformation::MarkerDrawing);
        a.checkEqual("14. mi_DrawingShape", info[0]->getValue(game::parser::mi_DrawingShape).orElse(-1), i);
    }
}

/** Test VPA marker: pink "o" (translated to type 3, color 15). */
AFL_TEST("game.parser.BinaryTransfer:unpackBinaryMessage:MarkerDrawing:vpa-type-circle", a)
{
    const String_t msg[] = {
        "<<< VPA Data Transmission >>>",
        "",
        "OBJECT: Marker",
        "DATA: -1680801779",
        "cafaokjapjiaaaaaaaaaljdkaa",
    };
    afl::charset::CodepageCharset cs(afl::charset::g_codepageLatin1);
    PtrVector<MessageInformation> info;
    a.check("01. unpackBinaryMessage", game::parser::unpackBinaryMessage(msg, 99, info, cs) == std::make_pair(game::parser::UnpackSuccess, game::parser::DrawingMessage));

    // Must have produced exactly one result (otherwise it will create multiple markers)
    a.checkEqual("11. size", info.size(), 1U);
    a.checkNonNull("12. info", info[0]);
    a.checkEqual("13. getObjectType", info[0]->getObjectType(), MessageInformation::MarkerDrawing);

    // Verify
    a.checkEqual("21. mi_X", info[0]->getValue(game::parser::mi_X).orElse(-1), 2478);
    a.checkEqual("22. mi_Y", info[0]->getValue(game::parser::mi_Y).orElse(-1), 2207);
    a.checkEqual("23. mi_Color", info[0]->getValue(game::parser::mi_Color).orElse(-1), 15);
    a.checkEqual("24. mi_DrawingShape", info[0]->getValue(game::parser::mi_DrawingShape).orElse(-1), 3);
    a.checkEqual("25. mi_DrawingExpire", info[0]->getValue(game::parser::mi_DrawingExpire).orElse(-1), -1);

    a.check("31. ms_DrawingComment", !info[0]->getValue(game::parser::ms_DrawingComment).isValid());
}

/** Test VPA marker: brown "Ne" (translated to type 2, color 17, with comment). */
AFL_TEST("game.parser.BinaryTransfer:unpackBinaryMessage:MarkerDrawing:vpa-type-ne", a)
{
    const String_t msg[] = {
        "<<< VPA Data Transmission >>>",
        "",
        "OBJECT: Marker",
        "DATA: -657391603",
        "hbgajkjailiaaaaaaaaaljdkaa",
        "",  // cover the "ignore trailing lines" branch because why not
    };
    afl::charset::CodepageCharset cs(afl::charset::g_codepageLatin1);
    PtrVector<MessageInformation> info;
    a.check("01. unpackBinaryMessage", game::parser::unpackBinaryMessage(msg, 99, info, cs) == std::make_pair(game::parser::UnpackSuccess, game::parser::DrawingMessage));

    // Must have produced exactly one result (otherwise it will create multiple markers)
    a.checkEqual("11. size", info.size(), 1U);
    a.checkNonNull("12. info", info[0]);
    a.checkEqual("13. getObjectType", info[0]->getObjectType(), MessageInformation::MarkerDrawing);

    // Verify
    a.checkEqual("21. mi_X", info[0]->getValue(game::parser::mi_X).orElse(-1), 2473);
    a.checkEqual("22. mi_Y", info[0]->getValue(game::parser::mi_Y).orElse(-1), 2232);
    a.checkEqual("23. mi_Color", info[0]->getValue(game::parser::mi_Color).orElse(-1), 16);
    a.checkEqual("24. mi_DrawingShape", info[0]->getValue(game::parser::mi_DrawingShape).orElse(-1), 2);
    a.checkEqual("25. mi_DrawingExpire", info[0]->getValue(game::parser::mi_DrawingExpire).orElse(-1), -1);

    a.checkEqual("31. ms_DrawingComment", info[0]->getValue(game::parser::ms_DrawingComment).orElse(""), "Ne");
}

/** Test VPA marker: brown "Tr" (translated to type 2, color 17; comment is preserved). */
AFL_TEST("game.parser.BinaryTransfer:unpackBinaryMessage:MarkerDrawing:vpa-type-tr", a)
{
    const String_t msg[] = {
        "<<< VPA Data Transmission >>>",
        "",
        "OBJECT: Marker",
        "DATA: -31653869",
        "ibgakljamliaaaaaoaaabacagaehchjhacjgeh",
    };
    afl::charset::CodepageCharset cs(afl::charset::g_codepageLatin1);
    PtrVector<MessageInformation> info;
    a.check("01. unpackBinaryMessage", game::parser::unpackBinaryMessage(msg, 99, info, cs) == std::make_pair(game::parser::UnpackSuccess, game::parser::DrawingMessage));

    // Must have produced exactly one result (otherwise it will create multiple markers)
    a.checkEqual("11. size", info.size(), 1U);
    a.checkNonNull("12. info", info[0]);
    a.checkEqual("13. getObjectType", info[0]->getObjectType(), MessageInformation::MarkerDrawing);

    // Verify
    a.checkEqual("21. mi_X", info[0]->getValue(game::parser::mi_X).orElse(-1), 2490);
    a.checkEqual("22. mi_Y", info[0]->getValue(game::parser::mi_Y).orElse(-1), 2236);
    a.checkEqual("23. mi_Color", info[0]->getValue(game::parser::mi_Color).orElse(-1), 16);
    a.checkEqual("24. mi_DrawingShape", info[0]->getValue(game::parser::mi_DrawingShape).orElse(-1), 2);
    a.checkEqual("25. mi_DrawingExpire", info[0]->getValue(game::parser::mi_DrawingExpire).orElse(-99), -1);

    a.checkEqual("31. ms_DrawingComment", info[0]->getValue(game::parser::ms_DrawingComment).orElse(""), "try it");
}

/** Test unpacking a Statistic entry. */
AFL_TEST("game.parser.BinaryTransfer:unpackBinaryMessage:PlayerScore", a)
{
    const String_t msg[] = {
        "<<< VPA Data Transmission >>>",
        "",
        "OBJECT: Statistic T46",
        "DATA: -1883438996",
        "aaaaaaaaaaaaaaaagaaaocaajndhpaaafapfaaaa",
        "nikkdaaaoljlppppfooabfccbacbkecnaaaahjcc",
        "aaaamanabaaaefnbaaaalmgdaaaaeicaaaaamlfl",
        "aaaapiddaaaaiccaaaaafjmhaaaaeamfaaaaobda",
        "aaaaglpfaaaacehdaaaapjcaaaaaligfaaaancaa",
        "oaaaaaaaccaahaaa",
        "",
        "",
        "",
    };
    afl::charset::CodepageCharset cs(afl::charset::g_codepageLatin1);
    PtrVector<MessageInformation> info;
    a.check("01. unpackBinaryMessage", game::parser::unpackBinaryMessage(msg, 99, info, cs) == std::make_pair(game::parser::UnpackSuccess, game::parser::StatisticMessage));

    // Must have produced ScoreId_Planets
    int score;
    Finder fp(info, MessageInformation::PlayerScore, game::score::ScoreId_Planets, 46);
    a.checkNonNull("11. find", fp.find());
    a.check("12. getScoreValue", fp.getScoreValue(6, score));
    a.checkEqual("13. score", score, 45);

    // Must have produced ScoreId_Bases
    Finder fb(info, MessageInformation::PlayerScore, game::score::ScoreId_Bases, 46);
    a.checkNonNull("21", fb.find());
    a.check("22", fb.getScoreValue(6, score));
    a.checkEqual("23. score", score, 14);
}

/*
 *  Decoder errors
 */

// Totally unspecial
AFL_TEST("game.parser.BinaryTransfer:unpackBinaryMessage:error:not-a-transmission", a)
{
    afl::charset::CodepageCharset cs(afl::charset::g_codepageLatin1);
    PtrVector<MessageInformation> info;
    const String_t msg[] = {
        "hi there",
    };
    a.checkEqual("01. unpackBinaryMessage", game::parser::unpackBinaryMessage(msg, 99, info, cs).first, game::parser::UnpackUnspecial);
}

// Missing DATA
AFL_TEST("game.parser.BinaryTransfer:unpackBinaryMessage:error:missing-data", a)
{
    afl::charset::CodepageCharset cs(afl::charset::g_codepageLatin1);
    PtrVector<MessageInformation> info;
    const String_t msg[] = {
        "<<< VPA Data Transmission >>>",
        "",
        "OBJECT: Marker",
        "jahaacjaibjaaaaaagaapdaaaa",
    };
    a.checkEqual("11. unpackBinaryMessage", game::parser::unpackBinaryMessage(msg, 99, info, cs).first, game::parser::UnpackUnspecial);
}

// Truncated text
AFL_TEST("game.parser.BinaryTransfer:unpackBinaryMessage:error:truncated", a)
{
    afl::charset::CodepageCharset cs(afl::charset::g_codepageLatin1);
    PtrVector<MessageInformation> info;
    const String_t msg[] = {
        "<<< VPA Data Transmission >>>",
        "",
        "OBJECT: Marker",
        "DATA: 291176461",
        "jah",
    };
    a.checkEqual("21. unpackBinaryMessage", game::parser::unpackBinaryMessage(msg, 99, info, cs).first, game::parser::UnpackUnspecial);
}

// Bad encoding
AFL_TEST("game.parser.BinaryTransfer:unpackBinaryMessage:error:bad-encoding", a)
{
    afl::charset::CodepageCharset cs(afl::charset::g_codepageLatin1);
    PtrVector<MessageInformation> info;
    const String_t msg[] = {
        "<<< VPA Data Transmission >>>",
        "",
        "OBJECT: Marker",
        "DATA: 291176461",
        "jahaacjaibjaaaaaagaapdzzaa",
    };
    a.checkEqual("31. unpackBinaryMessage", game::parser::unpackBinaryMessage(msg, 99, info, cs).first, game::parser::UnpackUnspecial);
}

// Wrong checksum
AFL_TEST("game.parser.BinaryTransfer:unpackBinaryMessage:error:checksum", a)
{
    afl::charset::CodepageCharset cs(afl::charset::g_codepageLatin1);
    PtrVector<MessageInformation> info;
    const String_t msg[] = {
        "<<< VPA Data Transmission >>>",
        "",
        "OBJECT: Marker",
        "DATA: 191176461",
        "jahaacjaibjaaaaaagaapdaaaa",
    };
    a.checkEqual("41. unpackBinaryMessage", game::parser::unpackBinaryMessage(msg, 99, info, cs).first, game::parser::UnpackChecksumError);
}

// Unknown object type
AFL_TEST("game.parser.BinaryTransfer:unpackBinaryMessage:error:unknown-type", a)
{
    afl::charset::CodepageCharset cs(afl::charset::g_codepageLatin1);
    PtrVector<MessageInformation> info;
    const String_t msg[] = {
        "<<< VPA Data Transmission >>>",
        "",
        "OBJECT: Macguffin",
        "DATA: 291176461",
        "jahaacjaibjaaaaaagaapdaaaa",
    };
    a.checkEqual("51. unpackBinaryMessage", game::parser::unpackBinaryMessage(msg, 99, info, cs).first, game::parser::UnpackUnspecial);
}

// Missing Id for Planet
AFL_TEST("game.parser.BinaryTransfer:unpackBinaryMessage:error:missing-planet-id", a)
{
    afl::charset::CodepageCharset cs(afl::charset::g_codepageLatin1);
    PtrVector<MessageInformation> info;
    const String_t msg[] = {
        "<<< VPA Data Transmission >>>",
        "",
        "OBJECT: Planet",
        "DATA: -1515519909",
        "ocaaocaagaaaggkcahabaaabaapaaaldaaaaaaga",
        "aaaaaahbaaaaaaebaaaaaabbaaaaaagbaaaaaaaa",
        "aaaaaaloaaaaaahjkaaaaabebaaaaapnbaaaaanf",
        "aafbaaleaabeaaaaaaaaaaegaaegaaaaaaaaaaaa",
        "aaaaaaocaaaaaaaaaagaaa",
    };
    a.checkEqual("61. unpackBinaryMessage", game::parser::unpackBinaryMessage(msg, 99, info, cs).first, game::parser::UnpackUnspecial);
}

// Missing Id for Minefield
AFL_TEST("game.parser.BinaryTransfer:unpackBinaryMessage:error:missing-ship-id", a)
{
    afl::charset::CodepageCharset cs(afl::charset::g_codepageLatin1);
    PtrVector<MessageInformation> info;
    const String_t msg[] = {
        "<<< VPA Data Transmission >>>",
        "",
        "OBJECT: Mine field",
        "DATA: 2094989326",
        "ocaalekakbhadaaaijmcaaaaaaaa"
    };
    a.checkEqual("71. unpackBinaryMessage", game::parser::unpackBinaryMessage(msg, 99, info, cs).first, game::parser::UnpackUnspecial);
}

// Planet too short
AFL_TEST("game.parser.BinaryTransfer:unpackBinaryMessage:error:planet-too-short", a)
{
    afl::charset::CodepageCharset cs(afl::charset::g_codepageLatin1);
    PtrVector<MessageInformation> info;
    const String_t msg[] = {
        "<<< VPA Data Transmission >>>",
        "",
        "OBJECT: Planet 15",
        "DATA: 291176461",
        "jahaacjaibjaaaaaagaapdaaaa",
    };
    a.checkEqual("81. unpackBinaryMessage", game::parser::unpackBinaryMessage(msg, 99, info, cs).first, game::parser::UnpackFailed);
}

// Minefield too short
AFL_TEST("game.parser.BinaryTransfer:unpackBinaryMessage:error:minefield-too-short", a)
{
    afl::charset::CodepageCharset cs(afl::charset::g_codepageLatin1);
    PtrVector<MessageInformation> info;
    const String_t msg[] = {
        "<<< VPA Data Transmission >>>",
        "",
        "OBJECT: Mine field 10",
        "DATA: 291176461",
        "jahaacjaibjaaaaaagaapdaaaa",
    };
    a.checkEqual("91. unpackBinaryMessage", game::parser::unpackBinaryMessage(msg, 99, info, cs).first, game::parser::UnpackFailed);
}

// Bad turn for statistic: cannot get turn 46 statistic in turn 45
AFL_TEST("game.parser.BinaryTransfer:unpackBinaryMessage:error:bad-turn", a)
{
    afl::charset::CodepageCharset cs(afl::charset::g_codepageLatin1);
    PtrVector<MessageInformation> info;
    const String_t msg[] = {
        "<<< VPA Data Transmission >>>",
        "",
        "OBJECT: Statistic T46",
        "DATA: -1883438996",
        "aaaaaaaaaaaaaaaagaaaocaajndhpaaafapfaaaa",
        "nikkdaaaoljlppppfooabfccbacbkecnaaaahjcc",
        "aaaamanabaaaefnbaaaalmgdaaaaeicaaaaamlfl",
        "aaaapiddaaaaiccaaaaafjmhaaaaeamfaaaaobda",
        "aaaaglpfaaaacehdaaaapjcaaaaaligfaaaancaa",
        "oaaaaaaaccaahaaa"
    };
    a.checkEqual("101. unpackBinaryMessage", game::parser::unpackBinaryMessage(msg, 45, info, cs).first, game::parser::UnpackUnspecial);
}

// Statistic too short
AFL_TEST("game.parser.BinaryTransfer:unpackBinaryMessage:error:statistic-too-short", a)
{
    afl::charset::CodepageCharset cs(afl::charset::g_codepageLatin1);
    PtrVector<MessageInformation> info;
    const String_t msg[] = {
        "<<< VPA Data Transmission >>>",
        "",
        "OBJECT: Statistic T46",
        "DATA: 291176461",
        "jahaacjaibjaaaaaagaapdaaaa",
    };
    a.checkEqual("111. unpackBinaryMessage", game::parser::unpackBinaryMessage(msg, 99, info, cs).first, game::parser::UnpackFailed);
}
