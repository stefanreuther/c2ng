/**
  *  \file game/parser/binarytransfer.cpp
  *  \brief Binary Message Transfer (VPA Data Transmission)
  */

#include "game/parser/binarytransfer.hpp"

#include "afl/base/countof.hpp"
#include "afl/base/growablememory.hpp"
#include "afl/base/memory.hpp"
#include "afl/base/staticassert.hpp"
#include "afl/base/types.hpp"
#include "afl/bits/fixedstring.hpp"
#include "afl/bits/int16le.hpp"
#include "afl/bits/int32le.hpp"
#include "afl/bits/value.hpp"
#include "afl/string/char.hpp"
#include "afl/string/format.hpp"
#include "game/limits.hpp"
#include "game/score/scoreid.hpp"
#include "game/v3/structures.hpp"
#include "util/string.hpp"
#include "util/stringparser.hpp"

using game::map::Drawing;
using game::map::Point;
using game::map::Planet;
using game::parser::MessageInformation;

namespace {

    const char*const TRANSFER_SIGNATURE = "<<< VPA Data Transmission >>>";

    typedef afl::bits::Value<afl::bits::Int16LE> Int16_t;
    typedef afl::bits::Value<afl::bits::Int32LE> Int32_t;
    typedef afl::bits::Value<afl::bits::FixedString<3> > String3_t;

    const int16_t UnknownInteger = -1;
    const int16_t UnknownNegative = int16_t(0x8000);
    const int32_t UnknownLong = -1;


    /*
     *  Structure Packing
     */

    /** Planet in a binary transmission. */
    struct BinaryPlanet {
        // VPA "PRec"
        Int16_t     earliestTurn;                               ///< Turn of oldest information.
        Int16_t     latestTurn;                                 ///< Turn of newest information.
        Int16_t     owner;                                      ///< Planet owner.
        String3_t   friendlyCode;                               ///< Friendly code.
        Int16_t     numMines;                                   ///< Mineral mines.
        Int16_t     numFactories;                               ///< Factories.
        Int16_t     numDefensePosts;                            ///< Defense posts.
        Int32_t     minedOre[4];                                ///< Mined ore. @see Ore.
        Int32_t     colonists;                                  ///< Colonist clans.
        Int32_t     supplies;                                   ///< Supplies.
        Int32_t     money;                                      ///< Money.
        Int32_t     groundOre[4];                               ///< Ground ore. @see Ore.
        Int16_t     oreDensity[4];                              ///< Density of ground ore. @see Ore.
        Int16_t     colonistTax;                                ///< Colonist tax rate.
        Int16_t     nativeTax;                                  ///< Native tax rate.
        Int16_t     colonistHappiness;                          ///< Colonist happiness.
        Int16_t     nativeHappiness;                            ///< Native happiness.
        Int16_t     nativeGovernment;                           ///< Native government.
        Int32_t     natives;                                    ///< Native clans.
        Int16_t     nativeRace;                                 ///< Native race.
        Int16_t     temperatureCode;                            ///< 100-temp, actually.
        Int16_t     baseFlag;                                   ///< EP_Base, EP_NoBase, EP_Activity.
        // VPA "EPln"
        Int16_t     scanTurn;                                   ///< Turn of sensor sweep.
        uint8_t     owner2;                                     ///< Owner during sensor sweep.
        uint8_t     flags;                                      ///< Flags.
    };
    static_assert(sizeof(BinaryPlanet) == 91, "sizeof BinaryPlanet");

    const uint8_t EP_Base     = 0x80;                   ///< Set if planet has a base.
    const uint8_t EP_NoBase   = 0x40;                   ///< Set if planet does not have a base.
    const uint8_t EP_Activity = 0x07;                   ///< Industry level, starting with 1=MinimalIndustry.


    /** Minefield in a binary transmission. */
    struct BinaryMinefield {
        // VPA "MRec"
        Int16_t     turnNumber;                                 ///< Time when last seen.
        Int16_t     x, y;                                       ///< Position.
        Int16_t     owner;                                      ///< Owner.
        Int32_t     units;                                      ///< Units.
        Int16_t     type;                                       ///< Minefield type. Bit 0=normal/web, bit 1=from KORE file
    };
    static_assert(sizeof(BinaryMinefield) == 14, "sizeof BinaryMinefield");

    /** Mineral in a statistics transmission. */
    struct BinaryStatisticMineral {
        Int32_t     available;                                  ///< Available amount (mined).
        Int32_t     production;                                 ///< Production (amount extracted).
        Int32_t     ground;                                     ///< Amount in ground.
    };

    /** Resource in a statistics transmission. */
    struct BinaryStatisticResource {
        Int32_t     available;                                  ///< Available amount.
        Int32_t     production;                                 ///< Production/growth.
    };

    /** Statistics in a binary transmission. */
    struct BinaryStatistics {
        Int32_t     unused[2];                                  ///< Unused (next/prev links in VPA).
        Int16_t     playerNr;                                   ///< Player number.
        Int16_t     turnNumber;                                 ///< Turn number.
        BinaryStatisticResource colonists;                      ///< Colonists/growth.
        BinaryStatisticResource natives;                        ///< Natives/growth.
        Int16_t     totalMines;                                 ///< Total mines.
        Int16_t     totalFactories;                             ///< Total factories.
        Int16_t     totalDefense;                               ///< Total defense.
        BinaryStatisticResource supplies;                       ///< Supplies/production.
        BinaryStatisticResource money;                          ///< Money/production.
        BinaryStatisticMineral neutronium;                      ///< Neutronium/production/ground.
        BinaryStatisticMineral tritanium;                       ///< Tritanium/production/ground.
        BinaryStatisticMineral duranium;                        ///< Duranium/production/ground.
        BinaryStatisticMineral molybdenum;                      ///< Molybdenum/production/ground.
        Int16_t     numPlanets;                                 ///< Number of owned planets.
        Int16_t     numBases;                                   ///< Number of starbases.
        Int16_t     numBasesBeingBuilt;                         ///< Number of starbases being built.
        Int16_t     numShips;                                   ///< Number of controlled ships.
        Int16_t     numShipsBeingBuilt;                         ///< Number of ships being built.
    };
    static_assert(sizeof(BinaryStatistics) == 108, "sizeof BinaryStatistics");

    /** Drawing in a binary transmission.
        Parameters:
        - for lines: dx, dy
        - for circles: 0, radius
        - for icons (everything else): internal text address flag (not relevant),
          x-align + 256*y-align

        For icons, the data is followed by a pascal string with the comment.
        VPA limits that to 20 characters. */
    struct BinaryDrawing {
        // VPA "MapMark"
        uint8_t     type;                                       ///< Marker type.
        uint8_t     color;                                      ///< Marker color (EGA, same as Winplan Ufos).
        Int16_t     x, y;                                       ///< Position.
        Int16_t     bind;                                       ///< Association with message (>0) or ship (<0). Missing in filefmt.txt.
        Int16_t     argX;                                       ///< "x" parameter.
        Int16_t     argY;                                       ///< "y" parameter.
        // Followed by Pascal string (length+data) with comment
    };
    static_assert(sizeof(BinaryDrawing) == 12, "sizeof BinaryDrawing");

    /** Maximum length of a drawing comment.
        VPA sends at most 20. Absolute max size is 6*40=240 bytes including header, so let's allow 200. */
    const size_t MAX_COMMENT_LENGTH = 200;

    // Drawing types:
    const uint8_t mrkNone   =  0;
    const uint8_t mrkFlag   =  1;
    const uint8_t mrkCircle =  2;
    const uint8_t mrkCross  =  3;
    const uint8_t mrkSquare =  4;
    const uint8_t mrkRhombe =  5;
    const uint8_t mrkPoint  =  6;
    const uint8_t mrkRCircle=  7;
    const uint8_t mrkLine   =  8;
    const uint8_t mrkDLine  =  9;
    const uint8_t mrkGrave  = 10;
    const uint8_t mrkCactus = 11;
    const uint8_t mrkFlag1  = 12;
    const uint8_t mrkFlag2  = 13;
    const uint8_t mrkFlag3  = 14;
    const uint8_t mrkArrow1 = 15;
    const uint8_t mrkArrow2 = 16;
    const uint8_t mrkArrow3 = 17;
    const uint8_t mrkArrow4 = 18;
    const uint8_t mrkArrow5 = 19;
    const uint8_t mrkArrow6 = 20;
    const uint8_t mrkArrow7 = 21;
    const uint8_t mrkArrow8 = 22;
    const uint8_t mrkNe     = 23;
    const uint8_t mrkTr     = 24;
    const uint8_t mrkDu     = 25;
    const uint8_t mrkMo     = 26;
    const uint8_t mrkSkull  = 27;

    class Packer {
     public:
        Packer();

        Packer& add(afl::base::ConstBytes_t bytes);

        String_t getResult() const;

        int32_t getChecksum() const;

        String_t buildText(const String_t objectName) const;

     private:
        String_t m_accumulator;
        int m_column;
        uint16_t m_length;
        uint16_t m_checksum;
    };

    /*
     *  Marker Type Conversions
     */

    uint8_t getExternalMarkerKind(int k)
    {
        switch (k) {
         case 0:
            // "plus" -> VPA does not have that, convert to square
            return mrkSquare;
         case 1:
            // "!" -> VPA does not have that, convert to flag
            return mrkFlag;
         case 2:
            // "x"
            return mrkCross;
         case 3:
            // "<>"
            return mrkRhombe;
         case 4:
            // "P" flag
            return mrkFlag1;
         case 5:
            // "X" -> VPA does not have that, convert to up-arrow
            return mrkArrow1;
         case 6:
            // "><" -> VPA does not have that, convert to right arrow
            return mrkArrow3;
         case 7:
            // "cactus"
            return mrkCactus;
        }
        return mrkNone;
    }

    uint8_t getExternalMarkerType(const Drawing& d)
    {
        switch (d.getType()) {
         case Drawing::LineDrawing:
            return mrkLine;
         case Drawing::RectangleDrawing:
            return mrkDLine;
         case Drawing::CircleDrawing:
            return mrkRCircle;
         case Drawing::MarkerDrawing:
            return getExternalMarkerKind(d.getMarkerKind());
        }
        return mrkNone;
    }


    /*
     *  Color Mapping
     *
     *  VPA uses the plain VGA palette whereas we use a custom palette.
     *  This mapping tries to preserve color meanings, i.e. yellow appears yellow on both sides.
     *  The mapping also needs to be reversible in case of a PCC user talking to a PCC user.
     *  Given that VPA has 15 colors while we have 30, we use VGA colors 16-31 as well
     *  (which appear the same as 0-15 due to VGA having only 4 bit color).
     */

    const uint8_t DEFAULT_COLOR = 9;

    // Marker colors to VPA colors
    static const uint8_t COLOR_EXPORT_MAP[] = {
        0,
        8,  7, 26, 28, 18, 17, 25, 24, 30, 15,    // 1-10
        1,  2,  3,  4,  5,  6, 20, 23, 19, 21,    // 11-20
        9, 10, 11, 12, 13, 14, 22, 31, 27, 29,    // 21-30
    };

    // VPA colors to marker colors
    static const uint8_t COLOR_IMPORT_MAP[] = {
        0, 11, 12, 13, 14, 15, 16,  2,            // 0-7
        1, 21, 22, 23, 24, 25, 26, 10,            // 8-15
        9,  6,  5, 19, 17, 20, 27, 18,            // 16-23 [16 not used, would appear black]
        8,  7,  3, 29,  4, 30,  9, 28,            // 24-31
    };

    uint8_t getExternalColor(uint8_t c)
    {
        if (c < countof(COLOR_EXPORT_MAP)) {
            return COLOR_EXPORT_MAP[c];
        } else {
            return COLOR_EXPORT_MAP[DEFAULT_COLOR];
        }
    }

    uint8_t getInternalColor(uint8_t c)
    {
        if (c < countof(COLOR_IMPORT_MAP)) {
            return COLOR_IMPORT_MAP[c];
        } else {
            return DEFAULT_COLOR;
        }
    }

    /*
     *  Data Packing
     */
    void copyOut(Int16_t& out, game::IntegerProperty_t p)
    {
        out = static_cast<int16_t>(p.orElse(-1));
    }

    void copyOut(Int32_t& out, game::LongProperty_t p)
    {
        out = static_cast<int32_t>(p.orElse(-1));
    }

    void copyOut2(Int16_t& taxOut, Int16_t& happyOut, game::IntegerProperty_t taxIn, game::NegativeProperty_t happyIn)
    {
        // Special case for taxation: VPA will send -1 for all unknown values, including happiness, despite -1 being a valid value.
        // Therefore, we send/receive happiness/tax only if tax is known, using tax as validity marker for both.
        int tax, happy;
        if (taxIn.get(tax) && happyIn.get(happy)) {
            taxOut = static_cast<int16_t>(tax);
            happyOut = static_cast<int16_t>(happy);
        } else {
            taxOut = -1;
            happyOut = -1;
        }
    }


    /*
     *  Data Unpacking
     */

    /* Add integer value */
    void addValue(MessageInformation& info, game::parser::MessageIntegerIndex idx, int32_t value, int32_t unknownMarker)
    {
        if (value != unknownMarker) {
            info.addValue(idx, value);
        }
    }

    /* Add string value */
    void addValue(MessageInformation& info, game::parser::MessageStringIndex idx, const String3_t& value, afl::charset::Charset& cs)
    {
        afl::base::ConstBytes_t data = value.m_bytes;
        if (data.findNot(0xFF) != data.size()) {
            info.addValue(idx, cs.decode(data));
        }
    }

    /* Pick turn number to use for report */
    int pickTurn(int turnNr, int reportedTurn)
    {
        // If no turn is reported (reportedTurn <= 0), just use the message's turn.
        // Otherwise, take oldest of reporter/message turn.
        return (reportedTurn > 0 ? std::min(turnNr, reportedTurn) : turnNr);
    }

    /* Main entry point: unpack minefield */
    game::parser::UnpackResult unpackMinefield(int turnNr, int id, afl::base::ConstBytes_t data, afl::container::PtrVector<MessageInformation>& info)
    {
        BinaryMinefield mf;
        if (data.size() < sizeof(mf)) {
            return game::parser::UnpackFailed;
        }
        afl::base::fromObject(mf).copyFrom(data);

        MessageInformation& mi = *info.pushBackNew(new MessageInformation(MessageInformation::Minefield, id, pickTurn(turnNr, mf.turnNumber)));
        mi.addValue(game::parser::mi_X, mf.x);
        mi.addValue(game::parser::mi_Y, mf.y);
        mi.addValue(game::parser::mi_Owner, mf.owner);
        mi.addValue(game::parser::mi_MineUnits, mf.units);
        mi.addValue(game::parser::mi_Type, mf.type & 1);
        return game::parser::UnpackSuccess;
    }

    /* Main entry point: unpack planet */
    game::parser::UnpackResult unpackPlanet(int turnNr, int id, afl::base::ConstBytes_t data, afl::container::PtrVector<MessageInformation>& info, afl::charset::Charset& cs)
    {
        namespace gt = game::v3::structures;

        BinaryPlanet pl;
        if (data.size() < sizeof(pl)) {
            return game::parser::UnpackFailed;
        }
        afl::base::fromObject(pl).copyFrom(data);

        // Pick turn for majority of values
        int mainTurn     = pickTurn(turnNr, std::max(pl.earliestTurn, pl.latestTurn));
        int colonistTurn = (pl.scanTurn <= 0 ? mainTurn : pickTurn(turnNr, pl.scanTurn));

        // Create MessageInformation objects
        MessageInformation& mainData = *info.pushBackNew(new MessageInformation(MessageInformation::Planet, id, mainTurn));
        MessageInformation& colonistData = *info.pushBackNew(new MessageInformation(MessageInformation::Planet, id, colonistTurn));

        // Owner: make a guess for the best value
        addValue(colonistData, game::parser::mi_Owner, (pl.owner > 0 ? pl.owner : pl.owner2 > 0 ? pl.owner2 : UnknownInteger), UnknownInteger);

        // The following are attributed to ColonistTime, but may be older than owner, so put them to main data.
        addValue(mainData, game::parser::ms_FriendlyCode,    pl.friendlyCode, cs);
        addValue(mainData, game::parser::mi_PlanetMines,     pl.numMines,        UnknownInteger);
        addValue(mainData, game::parser::mi_PlanetFactories, pl.numFactories,    UnknownInteger);
        addValue(mainData, game::parser::mi_PlanetDefense,   pl.numDefensePosts, UnknownInteger);

        // Minerals: report Totals first, so they can be corrected by Mined later
        addValue(mainData, game::parser::mi_PlanetTotalN,   pl.groundOre[gt::Neutronium],  UnknownLong);
        addValue(mainData, game::parser::mi_PlanetTotalT,   pl.groundOre[gt::Tritanium],   UnknownLong);
        addValue(mainData, game::parser::mi_PlanetTotalD,   pl.groundOre[gt::Duranium],    UnknownLong);
        addValue(mainData, game::parser::mi_PlanetTotalM,   pl.groundOre[gt::Molybdenum],  UnknownLong);
        addValue(mainData, game::parser::mi_PlanetMinedN,   pl.minedOre[gt::Neutronium],   UnknownLong);
        addValue(mainData, game::parser::mi_PlanetMinedT,   pl.minedOre[gt::Tritanium],    UnknownLong);
        addValue(mainData, game::parser::mi_PlanetMinedD,   pl.minedOre[gt::Duranium],     UnknownLong);
        addValue(mainData, game::parser::mi_PlanetMinedM,   pl.minedOre[gt::Molybdenum],   UnknownLong);
        addValue(mainData, game::parser::mi_PlanetDensityN, pl.oreDensity[gt::Neutronium], UnknownInteger);
        addValue(mainData, game::parser::mi_PlanetDensityT, pl.oreDensity[gt::Tritanium],  UnknownInteger);
        addValue(mainData, game::parser::mi_PlanetDensityD, pl.oreDensity[gt::Duranium],   UnknownInteger);
        addValue(mainData, game::parser::mi_PlanetDensityM, pl.oreDensity[gt::Molybdenum], UnknownInteger);

        // Colonists
        addValue(colonistData, game::parser::mi_PlanetColonists,         pl.colonists,         UnknownLong);
        addValue(colonistData, game::parser::mi_PlanetColonistTax,       pl.colonistTax,       UnknownInteger);
        if (pl.colonistTax != UnknownInteger) {
            colonistData.addValue(game::parser::mi_PlanetColonistHappiness, pl.colonistHappiness);
        }

        // Natives
        addValue(mainData, game::parser::mi_PlanetNatives,         pl.natives,          UnknownLong);
        addValue(mainData, game::parser::mi_PlanetNativeRace,      pl.nativeRace,       UnknownInteger);
        addValue(mainData, game::parser::mi_PlanetNativeGov,       pl.nativeGovernment, UnknownInteger);
        addValue(mainData, game::parser::mi_PlanetNativeTax,       pl.nativeTax,        UnknownInteger);
        if (pl.nativeTax != UnknownInteger) {
            mainData.addValue(game::parser::mi_PlanetNativeHappiness, pl.nativeHappiness);
        }

        // Resources
        addValue(mainData, game::parser::mi_PlanetSupplies, pl.supplies, UnknownLong);
        addValue(mainData, game::parser::mi_PlanetCash,     pl.money,    UnknownLong);

        // Temperature
        int temperatureCode = pl.temperatureCode;
        if (temperatureCode >= 0) {
            mainData.addValue(game::parser::mi_PlanetTemperature, 100 - temperatureCode);
        }

        // Flags
        int flags = pl.flags;
        if ((flags & (EP_Base | EP_NoBase)) != 0) {
            colonistData.addValue(game::parser::mi_PlanetHasBase, (flags & EP_Base) != 0);
        }
        if ((flags & EP_Activity) != 0) {
            colonistData.addValue(game::parser::mi_PlanetActivity, (flags & EP_Activity) - 1);
        }

        return game::parser::UnpackSuccess;
    }

    /* Unpack drawing, base part */
    MessageInformation& unpackDrawingBase(MessageInformation::Type type, int turnNr, const BinaryDrawing& d, afl::container::PtrVector<MessageInformation>& info)
    {
        MessageInformation& mi = *info.pushBackNew(new MessageInformation(type, 0, turnNr));
        mi.addValue(game::parser::mi_X, d.x);
        mi.addValue(game::parser::mi_Y, d.y);
        mi.addValue(game::parser::mi_Color, getInternalColor(d.color));

        // Drawings added by MessageInformation are transient by default.
        // We want explicitly received drawings to be persistent.
        mi.addValue(game::parser::mi_DrawingExpire, -1);
        return mi;
    }

    /* Unpack circle drawing */
    game::parser::UnpackResult unpackCircle(int turnNr, const BinaryDrawing& d, afl::container::PtrVector<MessageInformation>& info)
    {
        MessageInformation& mi = unpackDrawingBase(MessageInformation::CircleDrawing, turnNr, d, info);
        mi.addValue(game::parser::mi_Radius, d.argY);
        return game::parser::UnpackSuccess;
    }

    /* Unpack line/rectangle drawing */
    game::parser::UnpackResult unpackLineOrRectangle(MessageInformation::Type type, int turnNr, const BinaryDrawing& d, afl::container::PtrVector<MessageInformation>& info)
    {
        MessageInformation& mi = unpackDrawingBase(type, turnNr, d, info);
        mi.addValue(game::parser::mi_EndX, d.x + d.argX);
        mi.addValue(game::parser::mi_EndY, d.y + d.argY);
        return game::parser::UnpackSuccess;
    }

    /* Unpack marker */
    game::parser::UnpackResult unpackMarker(int shape, int turnNr, const BinaryDrawing& d, const String_t& comment, afl::container::PtrVector<MessageInformation>& info)
    {
        MessageInformation& mi = unpackDrawingBase(MessageInformation::MarkerDrawing, turnNr, d, info);
        mi.addValue(game::parser::mi_DrawingShape, shape);
        if (!comment.empty()) {
            mi.addValue(game::parser::ms_DrawingComment, comment);
        }
        return game::parser::UnpackSuccess;
    }

    /* Main entry point: unpack drawing */
    game::parser::UnpackResult unpackDrawing(int turnNr, afl::base::ConstBytes_t data, afl::container::PtrVector<MessageInformation>& info, afl::charset::Charset& cs)
    {
        // Main data
        BinaryDrawing d;
        if (data.size() < sizeof(d)) {
            return game::parser::UnpackFailed;
        }
        afl::base::fromObject(d).copyFrom(data);

        // Comment: may be absent [won't be with VPA or PCC2], but if it's present, it must not be truncated
        String_t comment;
        data.split(sizeof(d));
        if (const uint8_t* len = data.eat()) {
            if (data.size() < *len) {
                return game::parser::UnpackFailed;
            }
            comment = cs.decode(data.split(*len));
        }

        // Dispatch on type
        switch (d.type) {
         case mrkNone:
            return game::parser::UnpackFailed;
         case mrkFlag:
            // type 1 "!"
            return unpackMarker(1, turnNr, d, comment, info);
         case mrkCircle:
            // unmapped, map to 3 "<>"
            return unpackMarker(3, turnNr, d, comment, info);
         case mrkCross:
            // type 2 "x"
            return unpackMarker(2, turnNr, d, comment, info);
         case mrkSquare:
            // type 0 "+"
            return unpackMarker(0, turnNr, d, comment, info);
         case mrkRhombe:
            // type 3 "<>
            return unpackMarker(3, turnNr, d, comment, info);
         case mrkPoint:
            // unmapped, map to "+"
            return unpackMarker(0, turnNr, d, comment, info);
         case mrkRCircle:
            // normal circle
            return unpackCircle(turnNr, d, info);
         case mrkLine:
            // normal line
            return unpackLineOrRectangle(MessageInformation::LineDrawing, turnNr, d, info);
         case mrkDLine:
            // dashed line, mapped to rectangle both ways
            return unpackLineOrRectangle(MessageInformation::RectangleDrawing, turnNr, d, info);
         case mrkGrave:
            // unmapped, map to 1 "!"
            return unpackMarker(1, turnNr, d, comment, info);
         case mrkCactus:
            // type 7 "cactus"
            return unpackMarker(7, turnNr, d, comment, info);
         case mrkFlag1:
         case mrkFlag2:
         case mrkFlag3:
            // Flag1 mapped to type 4 "P" flag, map the others as well
            return unpackMarker(4, turnNr, d, comment, info);
         case mrkArrow1:
         case mrkArrow2:
         case mrkArrow7:
         case mrkArrow8:
            // Arrow1 mapped to type 5 "X", map others as well
            return unpackMarker(5, turnNr, d, comment, info);
         case mrkArrow3:
         case mrkArrow4:
         case mrkArrow5:
         case mrkArrow6:
            // Arrow3 mapped to type 6 "><", map others as well
            return unpackMarker(6, turnNr, d, comment, info);
         case mrkNe:
            // Map to 2 "x" with text, unless separate comment given
            return unpackMarker(2, turnNr, d, comment.empty() ? "Ne" : comment, info);
         case mrkTr:
            return unpackMarker(2, turnNr, d, comment.empty() ? "Tr" : comment, info);
         case mrkDu:
            return unpackMarker(2, turnNr, d, comment.empty() ? "Du" : comment, info);
         case mrkMo:
            return unpackMarker(2, turnNr, d, comment.empty() ? "Mo" : comment, info);
         case mrkSkull:
            // unmapped, map to 2 "x"
            return unpackMarker(2, turnNr, d, comment, info);
         default:
            return game::parser::UnpackFailed;
        }
    }

    void unpackStatisticScore(afl::container::PtrVector<MessageInformation>& info, const game::score::ScoreId_t scoreId, int turnNr, int playerNr, int score)
    {
        if (score >= 0) {
            MessageInformation& mi = *info.pushBackNew(new MessageInformation(MessageInformation::PlayerScore, scoreId, turnNr));
            mi.addScoreValue(playerNr, score);
        }
    }

    /* Main entry point: unpack statistic */
    game::parser::UnpackResult unpackStatistic(int turnNr, afl::base::ConstBytes_t data, afl::container::PtrVector<MessageInformation>& info)
    {
        // Main data
        BinaryStatistics d;
        if (data.size() < sizeof(d)) {
            return game::parser::UnpackFailed;
        }
        afl::base::fromObject(d).copyFrom(data);

        // Turn number must match
        if (d.turnNumber != turnNr) {
            return game::parser::UnpackFailed;
        }
        const int playerNr = d.playerNr;

        // For now, we don't have an exact equivalent to storing statistics.
        // We can get number of planets/bases, though. This might help when score blanking is used.
        unpackStatisticScore(info, game::score::ScoreId_Planets, turnNr, playerNr, d.numPlanets);
        unpackStatisticScore(info, game::score::ScoreId_Bases,   turnNr, playerNr, d.numBases);
        return game::parser::UnpackSuccess;
    }
}

Packer::Packer()
    : m_accumulator(),
      m_column(0),
      m_length(),
      m_checksum()
{ }

Packer&
Packer::add(afl::base::ConstBytes_t bytes)
{
    while (const uint8_t* p = bytes.eat()) {
        uint8_t ch1 = uint8_t('a' + (*p & 15));
        uint8_t ch2 = uint8_t('a' + (*p >> 4));
        m_accumulator += char(ch1);
        m_accumulator += char(ch2);
        if (++m_column >= 20) {
            m_accumulator += '\n';
            m_column = 0;
        }
        ++m_length;
        m_checksum = uint16_t(2*m_checksum + ch1 + 256*ch2);
    }
    return *this;
}

String_t
Packer::getResult() const
{
    if (m_column != 0) {
        return m_accumulator + '\n';
    } else {
        return m_accumulator;
    }
}

int32_t
Packer::getChecksum() const
{
    return int32_t(65536 * m_checksum + m_length);
}

String_t
Packer::buildText(const String_t objectName) const
{
    return afl::string::Format("%s\n\nOBJECT: %s\nDATA: %d\n%s", TRANSFER_SIGNATURE, objectName, getChecksum(), getResult());
}

/*
 *  Public Methods: Packing
 */

String_t
game::parser::packBinaryPlanet(const game::map::Planet& pl, afl::charset::Charset& cs, const HostVersion& host)
{
    namespace gt = game::v3::structures;

    // Determine owner
    int owner = -1;
    pl.getOwner(owner);

    // Determine timestamps
    int earliestTurn = 0, latestTurn = 0;
    for (size_t i = 0; i < Planet::NUM_TIMESTAMPS; ++i) {
        if (int me = pl.getHistoryTimestamp(Planet::Timestamp(i))) {
            if (earliestTurn == 0 || me < earliestTurn) {
                earliestTurn = me;
            }
            if (latestTurn == 0 || me > latestTurn) {
                latestTurn = me;
            }
        }
    }

    // Pack main part
    BinaryPlanet b;
    b.earliestTurn = static_cast<int16_t>(earliestTurn);
    b.latestTurn   = static_cast<int16_t>(latestTurn);
    b.owner        = static_cast<int16_t>(owner);

    StringProperty_t fcOpt = pl.getFriendlyCode();
    if (const String_t* fc = fcOpt.get()) {
        b.friendlyCode = cs.encode(afl::string::toMemory(*fc));
    } else {
        afl::base::Bytes_t(b.friendlyCode.m_bytes).fill(0xFF);
    }
    copyOut(b.numMines,                   pl.getNumBuildings(MineBuilding));
    copyOut(b.numFactories,               pl.getNumBuildings(FactoryBuilding));
    copyOut(b.numDefensePosts,            pl.getNumBuildings(DefenseBuilding));
    copyOut(b.minedOre[gt::Neutronium],   pl.getCargo(Element::Neutronium));
    copyOut(b.minedOre[gt::Tritanium],    pl.getCargo(Element::Tritanium));
    copyOut(b.minedOre[gt::Duranium],     pl.getCargo(Element::Duranium));
    copyOut(b.minedOre[gt::Molybdenum],   pl.getCargo(Element::Molybdenum));
    copyOut(b.colonists,                  pl.getCargo(Element::Colonists));
    copyOut(b.supplies,                   pl.getCargo(Element::Supplies));
    copyOut(b.money,                      pl.getCargo(Element::Money));
    copyOut(b.groundOre[gt::Neutronium],  pl.getOreGround(Element::Neutronium));
    copyOut(b.groundOre[gt::Tritanium],   pl.getOreGround(Element::Tritanium));
    copyOut(b.groundOre[gt::Duranium],    pl.getOreGround(Element::Duranium));
    copyOut(b.groundOre[gt::Molybdenum],  pl.getOreGround(Element::Molybdenum));
    copyOut(b.oreDensity[gt::Neutronium], pl.getOreDensity(Element::Neutronium));
    copyOut(b.oreDensity[gt::Tritanium],  pl.getOreDensity(Element::Tritanium));
    copyOut(b.oreDensity[gt::Duranium],   pl.getOreDensity(Element::Duranium));
    copyOut(b.oreDensity[gt::Molybdenum], pl.getOreDensity(Element::Molybdenum));
    copyOut2(b.colonistTax, b.colonistHappiness, pl.getColonistTax(), pl.getColonistHappiness());
    copyOut2(b.nativeTax,   b.nativeHappiness,   pl.getNativeTax(),   pl.getNativeHappiness());
    copyOut(b.nativeGovernment,           pl.getNativeGovernment());
    copyOut(b.natives,                    pl.getNatives());
    copyOut(b.nativeRace,                 pl.getNativeRace());

    int temp;
    if (pl.getTemperature().get(temp)) {
        b.temperatureCode = static_cast<int16_t>(100 - temp);
    } else {
        b.temperatureCode = -1;
    }

    b.baseFlag = 0;

    // "EPln" section
    // - scanTurn
    b.scanTurn = static_cast<int16_t>(pl.getHistoryTimestamp(Planet::ColonistTime));

    // - owner2
    b.owner2 = (owner > 0 ? static_cast<uint8_t>(owner) : 0);

    // - flags
    int flags = 0;
    int industryLevel;
    if (pl.getIndustryLevel(host).get(industryLevel)) {
        flags |= (industryLevel+1);
    }
    if (pl.hasBase()) {
        // We know it has a base
        flags |= EP_Base;
    } else {
        // If we play it, we know it has no base
        if (!pl.isPlayable(game::map::Object::ReadOnly)) {
            flags |= EP_NoBase;
        }
    }
    b.flags = static_cast<uint8_t>(flags);

    return Packer().add(afl::base::fromObject(b)).buildText(afl::string::Format("Planet %d", pl.getId()));
}

String_t
game::parser::packBinaryMinefield(const game::map::Minefield& mf)
{
    // ex chartdlg.pas:MailMinefield (sort-of)
    Point pos;
    mf.getPosition(pos);

    int owner = 0;
    mf.getOwner(owner);

    BinaryMinefield b;
    b.turnNumber = static_cast<int16_t>(mf.getTurnLastSeen());
    b.x          = static_cast<int16_t>(pos.getX());
    b.y          = static_cast<int16_t>(pos.getY());
    b.owner      = static_cast<int16_t>(owner);
    b.units      = mf.getUnitsLastSeen();
    b.type       = mf.isWeb() ? 1 : 0;

    return Packer().add(afl::base::fromObject(b)).buildText(afl::string::Format("Mine field %d", mf.getId()));
}

String_t
game::parser::packBinaryDrawing(const game::map::Drawing& d, afl::charset::Charset& cs)
{
    // BinaryDrawing object
    BinaryDrawing b;
    b.type  = getExternalMarkerType(d);
    b.color = getExternalColor(d.getColor());
    b.x     = static_cast<int16_t>(d.getPos().getX());
    b.y     = static_cast<int16_t>(d.getPos().getY());
    b.bind  = 0;                // not relevant outside VPA
    b.argX  = 0;                // default
    b.argY  = 0;                // default
    switch (d.getType()) {
     case Drawing::LineDrawing:
     case Drawing::RectangleDrawing:
        b.argX = static_cast<int16_t>(d.getPos2().getX() - d.getPos().getX());
        b.argY = static_cast<int16_t>(d.getPos2().getY() - d.getPos().getY());
        break;

     case Drawing::CircleDrawing:
        b.argY = static_cast<int16_t>(d.getCircleRadius());
        break;

     case Drawing::MarkerDrawing:
        b.argX = 6;             // text address flag (probably not relevant but happens to be this value in my test case)
        b.argY = 0x201;         // bottom/center alignment
        break;
    }

    // Comment text
    afl::base::GrowableBytes_t encodedComment = cs.encode(afl::string::toMemory(d.getComment()));
    encodedComment.trim(MAX_COMMENT_LENGTH);

    uint8_t commentLength = static_cast<uint8_t>(encodedComment.size());

    // Pack it
    return Packer().add(afl::base::fromObject(b)).add(afl::base::fromObject(commentLength)).add(encodedComment).buildText("Marker");
}

/*
 *  Public Methods: Unpacking
 */

game::parser::UnpackResultPair_t
game::parser::unpackBinaryMessage(afl::base::Memory<const String_t> in, int turnNr, afl::container::PtrVector<MessageInformation>& info, afl::charset::Charset& cs)
{
    // ex EvaluatePlayerMessage (sort-of)
    // TODO: handle Password transmission
    // TODO: handle planet list transmission
    // TODO: (future) VPA seems to have new verbs?

    enum State {
        LookForSignature,
        LookForObject,          // produces objectName
        LookForData,            // produces checksum
        Decoding,               // produces data
        Done
    };
    State state = LookForSignature;
    String_t objectName;
    int32_t combinedChecksum = 0;
    uint16_t computedChecksum = 0;
    afl::base::GrowableBytes_t data;

    // Locate start of data
    while (const String_t* p = in.eat()) {
        switch (state) {
         case LookForSignature:
            // TRANSFER_SIGNATURE can be preceded by anything (e.g. headers, forwarding indicator)
            if (p->find(TRANSFER_SIGNATURE) != String_t::npos) {
                state = LookForObject;
            }
            break;
         case LookForObject:
            // OBJECT: header can be preceded by anything (blank lines, usually)
            if (const char* n = util::strStartsWith(*p, "OBJECT:")) {
                objectName = afl::string::strTrim(n);
                state = LookForData;
            }
            break;
         case LookForData: {
            // DATA: header must immediately follow OBJECT
            util::StringParser parser(*p);
            if (parser.parseString("DATA: ") && parser.parseInt(combinedChecksum) && parser.parseEnd()) {
                state = Decoding;
            } else {
                return std::make_pair(UnpackUnspecial, NoMessage);
            }
            break;
         }
         case Decoding:
            // Decode until we reach an empty line or an error
            if (p->empty()) {
                state = Done;
            } else if (p->size() % 2 != 0) {
                return std::make_pair(UnpackUnspecial, NoMessage);
            } else {
                for (size_t i = 0; i < p->size(); i += 2) {
                    char och1 = (*p)[i];
                    char och2 = (*p)[i+1];
                    uint8_t ch1 = afl::string::charToLower(och1);
                    uint8_t ch2 = afl::string::charToLower(och2);
                    if (ch1 >= 'a' && ch1 <= 'a'+15 && ch2 >= 'a' && ch2 <= 'a'+15) {
                        data.append(static_cast<uint8_t>((ch1 - 'a') + 16*(ch2 - 'a')));
                        computedChecksum = uint16_t(2*computedChecksum + och1 + 256*och2);
                    } else {
                        return std::make_pair(UnpackUnspecial, NoMessage);
                    }
                }
            }
            break;
         case Done:
            break;
        }
    }

    // Final state?
    if (state != Decoding && state != Done) {
        return std::make_pair(UnpackUnspecial, NoMessage);
    }

    // Verify checksum
    int32_t expectedChecksum = int32_t(65536 * computedChecksum + data.size());
    if (expectedChecksum != combinedChecksum) {
        return std::make_pair(UnpackChecksumError, NoMessage);
    }

    util::StringParser p(objectName);
    int id;
    if (p.parseString("Mine field")) {
        if (p.parseInt(id) && p.parseEnd() && id > 0 && id <= MAX_NUMBER) {
            return std::make_pair(unpackMinefield(turnNr, id, data, info), MinefieldMessage);
        } else {
            return std::make_pair(UnpackUnspecial, NoMessage);
        }
    } else if (p.parseString("Planet")) {
        if (p.parseInt(id) && p.parseEnd() && id > 0 && id <= MAX_NUMBER) {
            return std::make_pair(unpackPlanet(turnNr, id, data, info, cs), PlanetMessage);
        } else {
            return std::make_pair(UnpackUnspecial, NoMessage);
        }
    } else if (p.parseString("Marker")) {
        if (p.parseEnd()) {
            return std::make_pair(unpackDrawing(turnNr, data, info, cs), DrawingMessage);
        } else {
            return std::make_pair(UnpackUnspecial, NoMessage);
        }
    } else if (p.parseString("Statistic T")) {
        if (p.parseInt(id) && p.parseEnd() && id > 0 && id <= turnNr) {
            return std::make_pair(unpackStatistic(id, data, info), StatisticMessage);
        } else {
            return std::make_pair(UnpackUnspecial, NoMessage);
        }
    } else {
        return std::make_pair(UnpackUnspecial, NoMessage);
    }
}
