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
        { "COMMENT",           game::parser::ms_DrawingComment          },
        { "FCODE",             game::parser::ms_FriendlyCode            },
        { "INFO1",             game::parser::ms_UfoInfo1                },
        { "INFO2",             game::parser::ms_UfoInfo2                },
        { "NAME",              game::parser::ms_Name                    },
        { "TAG",               game::parser::ms_DrawingTag              },
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
        { "BIDIR",             game::parser::mi_WormholeBidirFlag       }, // Not in scripting language
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
        { "COLOR",             game::parser::mi_Color                   }, // COLOR.EGA in scripting language for Ufos
        { "CREW",              game::parser::mi_ShipCrew                },
        { "DAMAGE",            game::parser::mi_Damage                  },
        { "DEFENSE",           game::parser::mi_PlanetDefense           },
        { "DENSITY.D",         game::parser::mi_PlanetDensityD          },
        { "DENSITY.M",         game::parser::mi_PlanetDensityM          },
        { "DENSITY.N",         game::parser::mi_PlanetDensityN          },
        { "DENSITY.T",         game::parser::mi_PlanetDensityT          },
        { "ENEMY",             game::parser::mi_ShipEnemy               },
        { "ENGINE",            game::parser::mi_ShipEngineType          },
        { "EXPIRE",            game::parser::mi_DrawingExpire           },
        { "FACTORIES",         game::parser::mi_PlanetFactories         },
        { "HEADING",           game::parser::mi_Heading                 },
        { "HULL",              game::parser::mi_ShipHull                },
        { "ID.SHIP",           game::parser::mi_ExplodedShipId          },
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
        { "SHAPE",             game::parser::mi_DrawingShape            },
        { "SPEED",             game::parser::mi_Speed                   },
        { "STABILITY",         game::parser::mi_WormholeStabilityCode   }, // Not in scripting language
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
     case ms_Name:         return tx("Name");
     case ms_FriendlyCode: return tx("FCode");
     case ms_UfoInfo1:     return tx("Info 1");
     case ms_UfoInfo2:     return tx("Info 2");
     case ms_DrawingComment: return tx("Comment");
     case ms_DrawingTag:   return tx("Tag");
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
     case mi_X:                 return tx("X");
     case mi_Y:                 return tx("Y");
     case mi_Radius:            return tx("Radius");
     case mi_Owner:             return tx("Owner");
     case mi_Damage:            return tx("Damage");
     case mi_Heading:           return tx("Heading");
     case mi_Speed:             return tx("Speed");
     case mi_Type:              return tx("Type");
     case mi_Mass:              return tx("Mass");
     case mi_Color:             return tx("Color");
     case mi_EndX:              return tx("End X");
     case mi_EndY:              return tx("End Y");

        /* Minefields: */
     case mi_MineUnits:         return tx("Mine Units");
     case mi_MineScanReason:    return tx("Mine Scan Reason");
     case mi_MineUnitsRemoved:  return tx("Mine Units Removed");

        /* Ships: */
     case mi_ShipHull:          return tx("Hull Type");
     case mi_ShipFuel:          return tx("Fuel");
     case mi_ShipRemoteFlag:    return tx("Remote-control flag");
     case mi_ShipWaypointDX:    return tx("Waypoint DX");
     case mi_ShipWaypointDY:    return tx("Waypoint DY");
     case mi_ShipEngineType:    return tx("Engine Type");
     case mi_ShipBeamType:      return tx("Beam Type");
     case mi_ShipNumBeams:      return tx("Number of Beams");
     case mi_ShipNumBays:       return tx("Number of Fighter Bays");
     case mi_ShipLauncherType:  return tx("Torpedo Type");
     case mi_ShipAmmo:          return tx("Ammo");
     case mi_ShipNumLaunchers:  return tx("Number of Torpedo Launchers");
     case mi_ShipMission:       return tx("Mission");
     case mi_ShipTow:           return tx("Mission Tow");
     case mi_ShipIntercept:     return tx("Mission Intercept");
     case mi_ShipEnemy:         return tx("Primary Enemy");
     case mi_ShipCrew:          return tx("Crew");
     case mi_ShipColonists:     return tx("Colonists aboard");
     case mi_ShipSupplies:      return tx("Supplies aboard");
     case mi_ShipCargoT:        return tx("Tritanium aboard");
     case mi_ShipCargoD:        return tx("Duranium aboard");
     case mi_ShipCargoM:        return tx("Molybdenum aboard");
     case mi_ShipMoney:         return tx("Money aboard");

        /* Planets: */
     case mi_PlanetTotalN:      return tx("Total Neutronium");
     case mi_PlanetTotalT:      return tx("Total Tritanium");
     case mi_PlanetTotalD:      return tx("Total Duranium");
     case mi_PlanetTotalM:      return tx("Total Molybdenum");
     case mi_PlanetAddedN:      return tx("Added Neutronium");
     case mi_PlanetAddedT:      return tx("Added Tritanium");
     case mi_PlanetAddedD:      return tx("Added Duranium");
     case mi_PlanetAddedM:      return tx("Added Molybdenum");
     case mi_PlanetMinedN:      return tx("Mined Neutronium");
     case mi_PlanetMinedT:      return tx("Mined Tritanium");
     case mi_PlanetMinedD:      return tx("Mined Duranium");
     case mi_PlanetMinedM:      return tx("Mined Molybdenum");
     case mi_PlanetDensityN:    return tx("Neutronium Density");
     case mi_PlanetDensityT:    return tx("Tritanium Density");
     case mi_PlanetDensityD:    return tx("Duranium Density");
     case mi_PlanetDensityM:    return tx("Molybdenum Density");
     case mi_PlanetCash:        return tx("Money");
     case mi_PlanetSupplies:    return tx("Supplies");
     case mi_PlanetHasBase:     return tx("Has Base");
     case mi_PlanetMines:       return tx("Mineral Mines");
     case mi_PlanetFactories:   return tx("Factories");
     case mi_PlanetDefense:     return tx("Defense Posts");
     case mi_PlanetTemperature: return tx("Temperature");
     case mi_PlanetColonists:   return tx("Colonist Clans");
     case mi_PlanetColonistHappiness: return tx("Colonist Happiness");
     case mi_PlanetColonistTax: return tx("Colonist Tax");
     case mi_PlanetActivity:    return tx("Industrial activity");
     case mi_PlanetNativeRace:  return tx("Native Race");
     case mi_PlanetNativeGov:   return tx("Native Government");
     case mi_PlanetNativeHappiness: return tx("Native Happiness");
     case mi_PlanetNativeTax:   return tx("Native Tax");
     case mi_PlanetNatives:     return tx("Native Population");
     case mi_PlanetHasNatives:  return tx("Natives present");

        /* Bases: */
     case mi_BaseQueuePos:      return tx("Build Queue Position");
     case mi_BaseQueuePriority: return tx("Build Priority");

        /* Score: */
     case mi_ScoreWinLimit:     return tx("Score Win Limit");
     case mi_ScoreTurnLimit:    return tx("Score Turn Limit");

        /* Ion Storm: */
     case mi_IonVoltage:        return tx("Voltage");
     case mi_IonStatus:         return tx("Storm Status");

        /* Ufo: */
     case mi_UfoRealId:         return tx("Real ID");
     case mi_UfoSpeedX:         return tx("X Movement");
     case mi_UfoSpeedY:         return tx("Y Movement");
     case mi_UfoPlanetRange:    return tx("Visibility Range from Planet");
     case mi_UfoShipRange:      return tx("Visibility Range from Ship");

        /* Wormhole: */
     case mi_WormholeStabilityCode: return tx("Stability");
     case mi_WormholeBidirFlag: return tx("Bidirectional");

        /* Explosion: */
     case mi_ExplodedShipId:    return tx("Exploded Ship Id");

        /* Drawing: */
     case mi_DrawingShape:      return tx("Shape");
     case mi_DrawingExpire:     return tx("Expiration turn");

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
