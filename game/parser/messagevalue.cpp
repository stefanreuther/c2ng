/**
  *  \file game/parser/messagevalue.cpp
  *  \brief Template class game::parser::MessageValue and related functions
  *
  *  This contains the integer-to-name and name-to-integer mapping functions for MessageStringIndex and MessageIntegerIndex.
  *
  *  Note that getNameFromIndex and getStringIndexFromKeyword/getIntegerIndexFromKeyword are not inverse to each other!
  */

#include "game/parser/messagevalue.hpp"

namespace {
    struct Keyword {
        uint16_t index;
        const char* word;
    };

    const Keyword stringNames[] = {
        { game::parser::ms_FriendlyCode,      "FCODE"          },
        { game::parser::ms_Name,              "NAME"           },
    };

    const Keyword intNames[] = {
        { game::parser::mi_PlanetAddedD,      "ADDED.D"        },
        { game::parser::mi_PlanetAddedM,      "ADDED.M"        },
        { game::parser::mi_PlanetAddedN,      "ADDED.N"        },
        { game::parser::mi_PlanetAddedT,      "ADDED.T"        },
        { game::parser::mi_PlanetHasBase,     "BASE"           },
        { game::parser::mi_BaseQueuePriority, "BUILD.PRIORITY" }, // Not in scripting language
        { game::parser::mi_BaseQueuePos,      "BUILD.QPOS"     },
        { game::parser::mi_ShipFuel,          "CARGO.N"        },
        { game::parser::mi_PlanetColonists,   "COLONISTS"      },
        { game::parser::mi_Damage,            "DAMAGE"         },
        { game::parser::mi_PlanetDefense,     "DEFENSE"        },
        { game::parser::mi_PlanetFactories,   "FACTORIES"      },
        { game::parser::mi_Heading,           "HEADING"        },
        { game::parser::mi_ShipHull,          "HULL"           },
        { game::parser::mi_PlanetActivity,    "INDUSTRY"       },
        { game::parser::mi_PlanetMinedD,      "MINED.D"        },
        { game::parser::mi_PlanetMinedM,      "MINED.M"        },
        { game::parser::mi_PlanetMinedN,      "MINED.N"        },
        { game::parser::mi_PlanetMinedT,      "MINED.T"        },
        { game::parser::mi_PlanetMines,       "MINES"          },
        { game::parser::mi_PlanetCash,        "MONEY"          },
        { game::parser::mi_PlanetNatives,     "NATIVES"        },
        { game::parser::mi_PlanetNativeGov,   "NATIVES.GOV"    },
        { game::parser::mi_PlanetNativeRace,  "NATIVES.RACE"   },
        { game::parser::mi_PlanetHasNatives,  "NATIVES.YESNO"  }, // Not in scripting language
        { game::parser::mi_Owner,             "OWNER"          },
        { game::parser::mi_Radius,            "RADIUS"         },
        { game::parser::mi_MineScanReason,    "REASON"         }, // SCANNED in the scripting language
        { game::parser::mi_Speed,             "SPEED"          },
        { game::parser::mi_IonStatus,         "STATUS"         },
        { game::parser::mi_PlanetSupplies,    "SUPPLIES"       },
        { game::parser::mi_PlanetTemperature, "TEMP"           },
        { game::parser::mi_PlanetTotalD,      "TOTAL.D"        },
        { game::parser::mi_PlanetTotalM,      "TOTAL.M"        },
        { game::parser::mi_PlanetTotalN,      "TOTAL.N"        },
        { game::parser::mi_PlanetTotalT,      "TOTAL.T"        },
        { game::parser::mi_ScoreTurnLimit,    "TURNLIMIT"      },
        { game::parser::mi_MineType,          "TYPE"           },
        { game::parser::mi_MineUnits,         "UNITS"          },
        { game::parser::mi_MineUnitsRemoved,  "UNITS.REMOVED"  }, // Not in scripting language
        { game::parser::mi_IonVoltage,        "VOLTAGE"        },
        { game::parser::mi_ScoreWinLimit,     "WINLIMIT"       },
        { game::parser::mi_X,                 "X"              },
        { game::parser::mi_Y,                 "Y"              },
    };

    int lookupKeyword(const String_t& kw, afl::base::Memory<const Keyword> defs, int defaultValue)
    {
        // Slow and simple
        while (const Keyword* p = defs.eat()) {
            if (p->word == kw) {
                return p->index;
            }
        }
        return defaultValue;
    }

}

// Get human-readable name, given a string index.
String_t
game::parser::getNameFromIndex(MessageStringIndex si, afl::string::Translator& tx)
{
    // ex game/parser.h:getNameForItem
    switch (si) {
     case ms_Name:         return tx.translateString("Name");
     case ms_FriendlyCode: return tx.translateString("FCode");
     case ms_Max:          return "?";
    }
    return "?";
}

// Get human-readable name, given an integer index.
String_t
game::parser::getNameFromIndex(MessageIntegerIndex ii, afl::string::Translator& tx)
{
    // ex game/parser.h:getNameForItem
    switch (ii) {
     case mi_X:                 return tx.translateString("X");
     case mi_Y:                 return tx.translateString("Y");
     case mi_Radius:            return tx.translateString("Radius");
     case mi_Owner:             return tx.translateString("Owner");
     case mi_Damage:            return tx.translateString("Damage");
     case mi_Heading:           return tx.translateString("Heading");
     case mi_Speed:             return tx.translateString("Speed");

        /* Minefields: */
     case mi_MineUnits:         return tx.translateString("Mine Units");
     case mi_MineScanReason:    return tx.translateString("Mine Scan Reason");
     case mi_MineType:          return tx.translateString("Mine Type");
     case mi_MineUnitsRemoved:  return tx.translateString("Mine Units Removed");

        /* Ships: */
     case mi_ShipHull:          return tx.translateString("Hull Type");
     case mi_ShipFuel:          return tx.translateString("Fuel");
     case mi_ShipRemoteFlag:    return tx.translateString("Remote-control flag");

        /* Planets: */
     case mi_PlanetTotalN:      return tx.translateString("Total Neutronium");
     case mi_PlanetTotalT:      return tx.translateString("Total Tritanium");
     case mi_PlanetTotalD:      return tx.translateString("Total Duranium");
     case mi_PlanetTotalM:      return tx.translateString("Total Molybdenum");
     case mi_PlanetAddedN:      return tx.translateString("Added Neutronium");
     case mi_PlanetAddedT:      return tx.translateString("Added Tritanium");
     case mi_PlanetAddedD:      return tx.translateString("Added Duranium");
     case mi_PlanetAddedM:      return tx.translateString("Added Molybdenum");
     case mi_PlanetMinedN:      return tx.translateString("Mined Neutronium");
     case mi_PlanetMinedT:      return tx.translateString("Mined Tritanium");
     case mi_PlanetMinedD:      return tx.translateString("Mined Duranium");
     case mi_PlanetMinedM:      return tx.translateString("Mined Molybdenum");
     case mi_PlanetCash:        return tx.translateString("Money");
     case mi_PlanetSupplies:    return tx.translateString("Supplies");
     case mi_PlanetHasBase:     return tx.translateString("Has Base");
     case mi_PlanetMines:       return tx.translateString("Mineral Mines");
     case mi_PlanetFactories:   return tx.translateString("Factories");
     case mi_PlanetDefense:     return tx.translateString("Defense Posts");
     case mi_PlanetTemperature: return tx.translateString("Temperature");
     case mi_PlanetColonists:   return tx.translateString("Colonist Clans");
     case mi_PlanetActivity:    return tx.translateString("Industrial activity");
     case mi_PlanetNativeRace:  return tx.translateString("Native Race");
     case mi_PlanetNativeGov:   return tx.translateString("Native Government");
     case mi_PlanetNatives:     return tx.translateString("Native Population");
     case mi_PlanetHasNatives:  return tx.translateString("Natives present");

        /* Bases: */
     case mi_BaseQueuePos:      return tx.translateString("Build Queue Position");
     case mi_BaseQueuePriority: return tx.translateString("Build Priority");

        /* Score: */
     case mi_ScoreWinLimit:     return tx.translateString("Score Win Limit");
     case mi_ScoreTurnLimit:    return tx.translateString("Score Turn Limit");

        /* Ion Storm: */
     case mi_IonVoltage:        return tx.translateString("Voltage");
     case mi_IonStatus:         return tx.translateString("Storm Status");


     case mi_Max:               return "?";
    }
    return "?";
}

// Get string index, given a keyword.
game::parser::MessageStringIndex
game::parser::getStringIndexFromKeyword(String_t kw)
{
    // ex game/parser.h:getStringIndexFromKeyword
    return MessageStringIndex(lookupKeyword(kw, stringNames, ms_Max));
}

// Get integer index, given a keyword.
game::parser::MessageIntegerIndex
game::parser::getIntegerIndexFromKeyword(String_t kw)
{
    // ex game/parser.h:getIntIndexFromKeyword
    return MessageIntegerIndex(lookupKeyword(kw, intNames, mi_Max));
}
