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
        const char* word;
        uint16_t index;
    };

    const Keyword stringNames[] = {
        { "FCODE",             game::parser::ms_FriendlyCode            },
        { "INFO1",             game::parser::ms_UfoInfo1                },
        { "INFO2",             game::parser::ms_UfoInfo2                },
        { "NAME",              game::parser::ms_Name                    },
    };

    const Keyword intNames[] = {
        { "ADDED.D",           game::parser::mi_PlanetAddedD            },
        { "ADDED.M",           game::parser::mi_PlanetAddedM            },
        { "ADDED.N",           game::parser::mi_PlanetAddedN            },
        { "ADDED.T",           game::parser::mi_PlanetAddedT            },
        { "AMMO",              game::parser::mi_ShipAmmo                },
        { "BASE",              game::parser::mi_PlanetHasBase           },
        { "BAYS",              game::parser::mi_ShipNumBays             },
        { "BEAM",              game::parser::mi_ShipBeamType            },
        { "BEAM.COUNT",        game::parser::mi_ShipNumBeams            },
        { "BUILD.PRIORITY",    game::parser::mi_BaseQueuePriority       }, // Not in scripting language
        { "BUILD.QPOS",        game::parser::mi_BaseQueuePos            },
        { "CARGO.COLONISTS",   game::parser::mi_ShipColonists           },
        { "CARGO.D",           game::parser::mi_ShipCargoD              },
        { "CARGO.M",           game::parser::mi_ShipCargoM              },
        { "CARGO.MONEY",       game::parser::mi_ShipMoney               },
        { "CARGO.N",           game::parser::mi_ShipFuel                },
        { "CARGO.SUPPLIES",    game::parser::mi_ShipSupplies            },
        { "CARGO.T",           game::parser::mi_ShipCargoT              },
        { "COLONISTS",         game::parser::mi_PlanetColonists         },
        { "COLONISTS.HAPPY",   game::parser::mi_PlanetColonistHappiness },
        { "COLONISTS.TAX",     game::parser::mi_PlanetColonistTax       },
        { "COLOR",             game::parser::mi_UfoColor                }, // COLOR.EGA in scripting language
        { "CREW",              game::parser::mi_ShipCrew                },
        { "DAMAGE",            game::parser::mi_Damage                  },
        { "DEFENSE",           game::parser::mi_PlanetDefense           },
        { "DENSITY.D",         game::parser::mi_PlanetDensityD          },
        { "DENSITY.M",         game::parser::mi_PlanetDensityM          },
        { "DENSITY.N",         game::parser::mi_PlanetDensityN          },
        { "DENSITY.T",         game::parser::mi_PlanetDensityT          },
        { "ENEMY",             game::parser::mi_ShipEnemy               },
        { "ENGINE",            game::parser::mi_ShipEngineType          },
        { "FACTORIES",         game::parser::mi_PlanetFactories         },
        { "HEADING",           game::parser::mi_Heading                 },
        { "HULL",              game::parser::mi_ShipHull                },
        { "ID2",               game::parser::mi_UfoRealId               },
        { "INDUSTRY",          game::parser::mi_PlanetActivity          },
        { "MASS",              game::parser::mi_Mass                    },
        { "MINED.D",           game::parser::mi_PlanetMinedD            },
        { "MINED.M",           game::parser::mi_PlanetMinedM            },
        { "MINED.N",           game::parser::mi_PlanetMinedN            },
        { "MINED.T",           game::parser::mi_PlanetMinedT            },
        { "MINES",             game::parser::mi_PlanetMines             },
        { "MISSION",           game::parser::mi_ShipMission             },
        { "MISSION.INTERCEPT", game::parser::mi_ShipIntercept           },
        { "MISSION.TOW",       game::parser::mi_ShipTow                 },
        { "MONEY",             game::parser::mi_PlanetCash              },
        { "MOVE.DX",           game::parser::mi_UfoSpeedX               },
        { "MOVE.DY",           game::parser::mi_UfoSpeedY               },
        { "NATIVES",           game::parser::mi_PlanetNatives           },
        { "NATIVES.GOV",       game::parser::mi_PlanetNativeGov         },
        { "NATIVES.HAPPY",     game::parser::mi_PlanetNativeHappiness   },
        { "NATIVES.RACE",      game::parser::mi_PlanetNativeRace        },
        { "NATIVES.TAX",       game::parser::mi_PlanetNativeTax         },
        { "NATIVES.YESNO",     game::parser::mi_PlanetHasNatives        }, // Not in scripting language
        { "OWNER",             game::parser::mi_Owner                   },
        { "RADIUS",            game::parser::mi_Radius                  },
        { "REASON",            game::parser::mi_MineScanReason          }, // SCANNED in the scripting language
        { "SPEED",             game::parser::mi_Speed                   },
        { "STATUS",            game::parser::mi_IonStatus               },
        { "SUPPLIES",          game::parser::mi_PlanetSupplies          },
        { "TEMP",              game::parser::mi_PlanetTemperature       },
        { "TORP",              game::parser::mi_ShipLauncherType        },
        { "TORP.LCOUNT",       game::parser::mi_ShipNumLaunchers        },
        { "TOTAL.D",           game::parser::mi_PlanetTotalD            },
        { "TOTAL.M",           game::parser::mi_PlanetTotalM            },
        { "TOTAL.N",           game::parser::mi_PlanetTotalN            },
        { "TOTAL.T",           game::parser::mi_PlanetTotalT            },
        { "TURNLIMIT",         game::parser::mi_ScoreTurnLimit          },
        { "TYPE",              game::parser::mi_Type                    },
        { "UNITS",             game::parser::mi_MineUnits               },
        { "UNITS.REMOVED",     game::parser::mi_MineUnitsRemoved        }, // Not in scripting language
        { "VISIBLE.PLANET",    game::parser::mi_UfoPlanetRange          },
        { "VISIBLE.SHIP",      game::parser::mi_UfoShipRange            },
        { "VOLTAGE",           game::parser::mi_IonVoltage              },
        { "WAYPOINT.DX",       game::parser::mi_ShipWaypointDX          },
        { "WAYPOINT.DY",       game::parser::mi_ShipWaypointDY          },
        { "WINLIMIT",          game::parser::mi_ScoreWinLimit           },
        { "X",                 game::parser::mi_X                       },
        { "Y",                 game::parser::mi_Y                       },
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
     case ms_UfoInfo1:     return tx.translateString("Info 1");
     case ms_UfoInfo2:     return tx.translateString("Info 2");
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
     case mi_Type:              return tx.translateString("Type");
     case mi_Mass:              return tx.translateString("Mass");

        /* Minefields: */
     case mi_MineUnits:         return tx.translateString("Mine Units");
     case mi_MineScanReason:    return tx.translateString("Mine Scan Reason");
     case mi_MineUnitsRemoved:  return tx.translateString("Mine Units Removed");

        /* Ships: */
     case mi_ShipHull:          return tx.translateString("Hull Type");
     case mi_ShipFuel:          return tx.translateString("Fuel");
     case mi_ShipRemoteFlag:    return tx.translateString("Remote-control flag");
     case mi_ShipWaypointDX:    return tx.translateString("Waypoint DX");
     case mi_ShipWaypointDY:    return tx.translateString("Waypoint DY");
     case mi_ShipEngineType:    return tx.translateString("Engine Type");
     case mi_ShipBeamType:      return tx.translateString("Beam Type");
     case mi_ShipNumBeams:      return tx.translateString("Number of Beams");
     case mi_ShipNumBays:       return tx.translateString("Number of Fighter Bays");
     case mi_ShipLauncherType:  return tx.translateString("Torpedo Type");
     case mi_ShipAmmo:          return tx.translateString("Ammo");
     case mi_ShipNumLaunchers:  return tx.translateString("Number of Torpedo Launchers");
     case mi_ShipMission:       return tx.translateString("Mission");
     case mi_ShipTow:           return tx.translateString("Mission Tow");
     case mi_ShipIntercept:     return tx.translateString("Mission Intercept");
     case mi_ShipEnemy:         return tx.translateString("Primary Enemy");
     case mi_ShipCrew:          return tx.translateString("Crew");
     case mi_ShipColonists:     return tx.translateString("Colonists aboard");
     case mi_ShipSupplies:      return tx.translateString("Supplies aboard");
     case mi_ShipCargoT:        return tx.translateString("Tritanium aboard");
     case mi_ShipCargoD:        return tx.translateString("Duranium aboard");
     case mi_ShipCargoM:        return tx.translateString("Molybdenum aboard");
     case mi_ShipMoney:         return tx.translateString("Money aboard");

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
     case mi_PlanetDensityN:    return tx.translateString("Neutronium Density");
     case mi_PlanetDensityT:    return tx.translateString("Tritanium Density");
     case mi_PlanetDensityD:    return tx.translateString("Duranium Density");
     case mi_PlanetDensityM:    return tx.translateString("Molybdenum Density");
     case mi_PlanetCash:        return tx.translateString("Money");
     case mi_PlanetSupplies:    return tx.translateString("Supplies");
     case mi_PlanetHasBase:     return tx.translateString("Has Base");
     case mi_PlanetMines:       return tx.translateString("Mineral Mines");
     case mi_PlanetFactories:   return tx.translateString("Factories");
     case mi_PlanetDefense:     return tx.translateString("Defense Posts");
     case mi_PlanetTemperature: return tx.translateString("Temperature");
     case mi_PlanetColonists:   return tx.translateString("Colonist Clans");
     case mi_PlanetColonistHappiness: return tx.translateString("Colonist Happiness");
     case mi_PlanetColonistTax: return tx.translateString("Colonist Tax");
     case mi_PlanetActivity:    return tx.translateString("Industrial activity");
     case mi_PlanetNativeRace:  return tx.translateString("Native Race");
     case mi_PlanetNativeGov:   return tx.translateString("Native Government");
     case mi_PlanetNativeHappiness: return tx.translateString("Native Happiness");
     case mi_PlanetNativeTax:   return tx.translateString("Native Tax");
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

        /* Ufo: */
     case mi_UfoRealId:         return tx.translateString("Real ID");
     case mi_UfoSpeedX:         return tx.translateString("X Movement");
     case mi_UfoSpeedY:         return tx.translateString("Y Movement");
     case mi_UfoColor:          return tx.translateString("Color");
     case mi_UfoPlanetRange:    return tx.translateString("Visibility Range from Planet");
     case mi_UfoShipRange:      return tx.translateString("Visibility Range from Ship");

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
