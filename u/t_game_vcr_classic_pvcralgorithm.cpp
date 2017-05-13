/**
  *  \file u/t_game_vcr_classic_pvcralgorithm.cpp
  *  \brief Test for game::vcr::classic::PVCRAlgorithm
  */

#include "game/vcr/classic/pvcralgorithm.hpp"

#include "t_game_vcr_classic.hpp"
#include "afl/base/countof.hpp"
#include "game/vcr/classic/nullvisualizer.hpp"

namespace {
    struct Cost {
        int mc, t, d, m;
    };
    struct Beam {
        const char* name;
        Cost cost;
        int mass;
        int techLevel;
        int killPower;
        int damagePower;
    };
    const Beam beams[] = {
        {"Laser Cannon",      {1,1,0,1},0,1,1,2},
        {"Kill-O-Zap",        {5,1,2,0},0,2,10,1},
        {"Desintegrator",     {10,3,1,2},1,4,7,10},
        {"Phaser",            {20,5,0,2},1,6,15,25},
        {"Disruptor",         {45,10,5,5},1,7,40,10},
        {"Electron Ram",      {50,15,5,10},2,7,20,40},
        {"Ion Cannon",        {60,5,20,5},1,8,10,45},
        {"Turbolaser Battery",{90,20,5,10},2,9,30,60},
        {"Inpotron Cannon",   {110,10,10,10},3,10,70,35},
        {"Multitraf Spiral",  {130,25,15,10},3,10,40,80}
    };
    struct Torpedo {
        const char* name;
        Cost torpedoCost;
        Cost launcherCost;
        int mass;
        int techLevel;
        int killPower;
        int damagePower;
    };
    const Torpedo torpedoes[] = {
        {"Space Rocket",       {2,1, 1,1},{  5, 1, 0, 0},1,1,3,5},
        {"Fusion Bomb",        {8,1, 1,1},{ 20, 2, 1, 0},1,3,10,10},
        {"Paralyso-Matic Bomb",{10,1,1,1},{ 35, 4, 0, 5},0,5,20,1},
        {"Initial Bomb",       {20,1,1,1},{ 60, 5, 1, 2},2,7,50,15},
        {"Photon Torp",        {30,1,1,1},{ 70, 7, 1, 3},2,7,25,50},
        {"Graviton Bomb",      {35,1,1,1},{ 80, 8, 3, 5},3,8,10,60},
        {"Arkon Bomb",         {50,1,1,1},{100,15,10, 5},4,9,56,55},
        {"Antimatter Bomb",    {55,1,1,1},{105,10, 3,10},2,9,35,75},
        {"Katalysator Bomb",   {65,1,1,1},{130, 5, 1,10},4,10,80,50},
        {"Selphyr-Fataro-Dev.",{80,1,1,1},{150,15, 5,20},7,10,40,99}
    };

    game::spec::Cost convertCost(const Cost& c)
    {
        game::spec::Cost result;
        result.set(result.Duranium, c.d);
        result.set(result.Tritanium, c.t);
        result.set(result.Molybdenum, c.m);
        result.set(result.Money, c.mc);
        return result;
    }

    void initShipList(game::spec::ShipList& list)
    {
        for (int i = 0; i < int(countof(beams)); ++i) {
            const Beam& in = beams[i];
            if (game::spec::Beam* out = list.beams().create(i+1)) {
                out->setKillPower(in.killPower);
                out->setDamagePower(in.damagePower);
                out->setMass(in.mass);
                out->setTechLevel(in.techLevel);
                out->setName(in.name);
                out->cost() = convertCost(in.cost);
            }
        }
        for (int i = 0; i < int(countof(torpedoes)); ++i) {
            const Torpedo& in = torpedoes[i];
            if (game::spec::TorpedoLauncher* out = list.launchers().create(i+1)) {
                out->setKillPower(in.killPower);
                out->setDamagePower(in.damagePower);
                out->setMass(in.mass);
                out->setTechLevel(in.techLevel);
                out->setName(in.name);
                out->cost() = convertCost(in.launcherCost);
                out->torpedoCost() = convertCost(in.torpedoCost);
            }
        }
    }
    
    struct Object {
        int beamType;
        int numBeams;
        int crew;
        int damage;
        int numBays;
        int numFighters;
        int hull;
        int id;
        int image;
        int experienceLevel;
        int mass;
        const char* name;
        int owner;
        int shield;
        int torpedoType;
        int numTorpedoes;
        int numLaunchers;
        int isPlanet;
        int beamKillRate;
        int beamChargeRate;
        int torpMissRate;
        int torpChargeRate;
        int crewDefenseRate;
    };
    struct Battle {
        int magic;
        uint16_t seed;
        int capabilities;
        const char* algorithm;
        Object object[2];
    };

    const Battle battles[] = {
        // tests/pvcr/vcr5.dat battle #1
        {18801,30078,0,"PHost 3",{{6,12,1300,0,14,150,99,143,144,0,751,"Bloody Mary",11,100,0,0,0,0,1,1,35,1,0},
                                  {10,15,787,0,0,0,35,2,68,0,681,"Cyc-9",4,100,10,85,13,0,1,1,35,1,0}}},

        // tests/pvcr/vcr5.dat battle #12
        {0,53958,0,"PHost 3",{{7,12,1249,0,14,129,99,492,144,0,751,"Alvilda the Goth",11,100,0,0,0,0,1,1,35,1,0},
                              {2, 2,   0,0, 3,  3, 0,425,  0,0,110,"Steenrod",        10,100,0,0,0,1,1,1,35,1,0}}},

//         {"MAGIC":0,"SEED":37461,"CAPABILITIES":0,"ALGORITHM":"PHost 3","UNIT":[
//             {"BEAM":7,"BEAM.COUNT":7,"CREW":270,"DAMAGE":0,"FIGHTER.BAYS":12,"FIGHTER.COUNT":100,
//              "HULL":92,"ID":142,"IMAGE":135,"LEVEL":0,"MASS":537,"NAME":"Al Bundy","OWNER":11,
//              "SHIELD":100,"TORP.COUNT":0,"TORP.LCOUNT":0,"ISPLANET":0,"CONFIG.BEAMKILLRATE":1,
//              "CONFIG.BEAMCHARGERATE":1,"CONFIG.TORPMISSRATE":35,"CONFIG.TORPCHARGERATE":1,"CONFIG.CREWDEFENSERATE":0},
//             {"BEAM":10,"BEAM.COUNT":6,"CREW":698,"DAMAGE":61,"FIGHTER.BAYS":0,"FIGHTER.COUNT":0,"HULL":35,
//              "ID":2,"IMAGE":68,"LEVEL":0,"MASS":681,"NAME":"Cyc-9","OWNER":4,"SHIELD":0,"TORP":10,
//              "TORP.COUNT":14,"TORP.LCOUNT":6,"ISPLANET":0,"CONFIG.BEAMKILLRATE":1,"CONFIG.BEAMCHARGERATE":1,
//              "CONFIG.TORPMISSRATE":35,"CONFIG.TORPCHARGERATE":1,"CONFIG.CREWDEFENSERATE":0}]
//         },
//         {"MAGIC":0,"SEED":48940,"CAPABILITIES":0,"ALGORITHM":"PHost 3","UNIT":[
//             {"BEAM":7,"BEAM.COUNT":7,"CREW":270,"DAMAGE":0,"FIGHTER.BAYS":12,
//              "FIGHTER.COUNT":95,"HULL":92,"ID":142,"IMAGE":135,"LEVEL":0,"MASS":537,"NAME":"Al Bundy",
//              "OWNER":11,"SHIELD":26,"TORP.COUNT":0,"TORP.LCOUNT":0,"ISPLANET":0,"CONFIG.BEAMKILLRATE":1,
//              "CONFIG.BEAMCHARGERATE":1,"CONFIG.TORPMISSRATE":35,"CONFIG.TORPCHARGERATE":1,"CONFIG.CREWDEFENSERATE":0},
//             {"BEAM":10,"BEAM.COUNT":15,"CREW":787,"DAMAGE":0,"FIGHTER.BAYS":0,"FIGHTER.COUNT":0,"HULL":35,
//              "ID":149,"IMAGE":68,"LEVEL":0,"MASS":681,"NAME":"Cyc-11","OWNER":4,"SHIELD":100,"TORP":10,
//              "TORP.COUNT":85,"TORP.LCOUNT":13,"ISPLANET":0,"CONFIG.BEAMKILLRATE":1,"CONFIG.BEAMCHARGERATE":1,
//              "CONFIG.TORPMISSRATE":35,"CONFIG.TORPCHARGERATE":1,"CONFIG.CREWDEFENSERATE":0}]
//         },
//         {"MAGIC":0,"SEED":16696,"CAPABILITIES":0,"ALGORITHM":"PHost 3","UNIT":[
//             {"BEAM":10,"BEAM.COUNT":14,"CREW":768,"DAMAGE":13,"FIGHTER.BAYS":0,
//              "FIGHTER.COUNT":0,"HULL":35,"ID":149,"IMAGE":68,"LEVEL":0,"MASS":681,"NAME":"Cyc-11",
//              "OWNER":4,"SHIELD":0,"TORP":10,"TORP.COUNT":43,"TORP.LCOUNT":12,"ISPLANET":0,"CONFIG.BEAMKILLRATE":1,
//              "CONFIG.BEAMCHARGERATE":1,"CONFIG.TORPMISSRATE":35,"CONFIG.TORPCHARGERATE":1,"CONFIG.CREWDEFENSERATE":0},
//             {"BEAM":7,"BEAM.COUNT":12,"CREW":1300,"DAMAGE":0,"FIGHTER.BAYS":14,"FIGHTER.COUNT":240,"HULL":99,
//              "ID":180,"IMAGE":144,"LEVEL":0,"MASS":751,"NAME":"Charlotte de Berry","OWNER":11,
//              "SHIELD":100,"TORP.COUNT":0,"TORP.LCOUNT":0,"ISPLANET":0,"CONFIG.BEAMKILLRATE":1,
//              "CONFIG.BEAMCHARGERATE":1,"CONFIG.TORPMISSRATE":35,"CONFIG.TORPCHARGERATE":1,"CONFIG.CREWDEFENSERATE":0}]
//         },
//         {"MAGIC":0,"SEED":34185,"CAPABILITIES":0,"ALGORITHM":"PHost 3","UNIT":[
//             {"BEAM":10,"BEAM.COUNT":15,
//              "CREW":787,"DAMAGE":0,"FIGHTER.BAYS":0,"FIGHTER.COUNT":0,"HULL":35,"ID":483,"IMAGE":68,
//              "LEVEL":0,"MASS":681,"NAME":"Cyc-10","OWNER":4,"SHIELD":100,"TORP":10,"TORP.COUNT":93,
//              "TORP.LCOUNT":13,"ISPLANET":0,"CONFIG.BEAMKILLRATE":1,"CONFIG.BEAMCHARGERATE":1,"CONFIG.TORPMISSRATE":35,
//              "CONFIG.TORPCHARGERATE":1,"CONFIG.CREWDEFENSERATE":0},
//             {"BEAM":7,"BEAM.COUNT":8,"CREW":1279,"DAMAGE":34,"FIGHTER.BAYS":10,
//              "FIGHTER.COUNT":218,"HULL":99,"ID":180,"IMAGE":144,"LEVEL":0,"MASS":751,"NAME":"Charlotte de Berry",
//              "OWNER":11,"SHIELD":0,"TORP.COUNT":0,"TORP.LCOUNT":0,"ISPLANET":0,"CONFIG.BEAMKILLRATE":1,
//              "CONFIG.BEAMCHARGERATE":1,"CONFIG.TORPMISSRATE":35,"CONFIG.TORPCHARGERATE":1,"CONFIG.CREWDEFENSERATE":0}]
//         },
//         {"MAGIC":0,"SEED":34415,"CAPABILITIES":0,"ALGORITHM":"PHost 3","UNIT":[
//             {"BEAM":10,"BEAM.COUNT":15,
//              "CREW":787,"DAMAGE":0,"FIGHTER.BAYS":0,"FIGHTER.COUNT":0,"HULL":35,"ID":483,"IMAGE":68,
//              "LEVEL":0,"MASS":681,"NAME":"Cyc-10","OWNER":4,"SHIELD":30,"TORP":10,"TORP.COUNT":62,
//              "TORP.LCOUNT":13,"ISPLANET":0,"CONFIG.BEAMKILLRATE":1,"CONFIG.BEAMCHARGERATE":1,"CONFIG.TORPMISSRATE":35,
//              "CONFIG.TORPCHARGERATE":1,"CONFIG.CREWDEFENSERATE":0},
//             {"BEAM":2,"BEAM.COUNT":6,"CREW":334,"DAMAGE":0,"FIGHTER.BAYS":0,
//              "FIGHTER.COUNT":0,"HULL":7,"ID":88,"IMAGE":30,"LEVEL":0,"MASS":261,"NAME":"Bicz na Romana",
//              "OWNER":11,"SHIELD":100,"TORP":10,"TORP.COUNT":20,"TORP.LCOUNT":3,"ISPLANET":0,"CONFIG.BEAMKILLRATE":1,
//              "CONFIG.BEAMCHARGERATE":1,"CONFIG.TORPMISSRATE":35,"CONFIG.TORPCHARGERATE":1,"CONFIG.CREWDEFENSERATE":0}]
//         },
//         {"MAGIC":0,"SEED":56370,"CAPABILITIES":0,"ALGORITHM":"PHost 3","UNIT":[
//             {"BEAM":10,"BEAM.COUNT":15,
//              "CREW":787,"DAMAGE":0,"FIGHTER.BAYS":0,"FIGHTER.COUNT":0,"HULL":35,"ID":183,"IMAGE":68,
//              "LEVEL":0,"MASS":681,"NAME":"Cynik","OWNER":4,"SHIELD":100,"TORP":10,"TORP.COUNT":0,
//              "TORP.LCOUNT":13,"ISPLANET":0,"CONFIG.BEAMKILLRATE":1,"CONFIG.BEAMCHARGERATE":1,"CONFIG.TORPMISSRATE":35,
//              "CONFIG.TORPCHARGERATE":1,"CONFIG.CREWDEFENSERATE":0},
//             {"BEAM":0,"BEAM.COUNT":0,"CREW":0,"DAMAGE":0,"FIGHTER.BAYS":0,
//              "FIGHTER.COUNT":0,"ID":462,"IMAGE":0,"LEVEL":0,"MASS":100,"NAME":"Turing","OWNER":1,
//              "SHIELD":100,"TORP.COUNT":0,"TORP.LCOUNT":0,"ISPLANET":1,"CONFIG.BEAMKILLRATE":1,
//              "CONFIG.BEAMCHARGERATE":1,"CONFIG.TORPMISSRATE":35,"CONFIG.TORPCHARGERATE":1,"CONFIG.CREWDEFENSERATE":0}]
//         },
//         {"MAGIC":0,"SEED":58730,"CAPABILITIES":0,"ALGORITHM":"PHost 3","UNIT":[
//             {"BEAM":10,"BEAM.COUNT":15,"CREW":787,"DAMAGE":0,"FIGHTER.BAYS":0,"FIGHTER.COUNT":0,"HULL":35,"ID":163,"IMAGE":68,
//              "LEVEL":0,"MASS":681,"NAME":"Cyrulik","OWNER":4,"SHIELD":100,"TORP":10,"TORP.COUNT":107,
//              "TORP.LCOUNT":13,"ISPLANET":0,"CONFIG.BEAMKILLRATE":1,"CONFIG.BEAMCHARGERATE":1,"CONFIG.TORPMISSRATE":35,
//              "CONFIG.TORPCHARGERATE":1,"CONFIG.CREWDEFENSERATE":0},
//             {"BEAM":5,"BEAM.COUNT":4,"CREW":0,"DAMAGE":0,"FIGHTER.BAYS":8,
//              "FIGHTER.COUNT":8,"ID":346,"IMAGE":0,"LEVEL":0,"MASS":157,"NAME":"Platon","OWNER":8,
//              "SHIELD":100,"TORP.COUNT":0,"TORP.LCOUNT":0,"ISPLANET":1,"CONFIG.BEAMKILLRATE":1,
//              "CONFIG.BEAMCHARGERATE":1,"CONFIG.TORPMISSRATE":35,"CONFIG.TORPCHARGERATE":1,"CONFIG.CREWDEFENSERATE":0}]
//         },
//         {"MAGIC":0,"SEED":30372,"CAPABILITIES":0,"ALGORITHM":"PHost 3","UNIT":[
//             {"BEAM":10,"BEAM.COUNT":5,"CREW":148,"DAMAGE":0,"FIGHTER.BAYS":0,"FIGHTER.COUNT":0,"HULL":46,"ID":301,"IMAGE":76,
//              "LEVEL":0,"MASS":171,"NAME":"Orlik-4","OWNER":5,"SHIELD":100,"TORP":1,"TORP.COUNT":0,
//              "TORP.LCOUNT":3,"ISPLANET":0,"CONFIG.BEAMKILLRATE":3,"CONFIG.BEAMCHARGERATE":1,"CONFIG.TORPMISSRATE":35,
//              "CONFIG.TORPCHARGERATE":1,"CONFIG.CREWDEFENSERATE":0},
//             {"BEAM":7,"BEAM.COUNT":6,"CREW":0,"DAMAGE":0,"FIGHTER.BAYS":10,
//              "FIGHTER.COUNT":10,"ID":67,"IMAGE":0,"LEVEL":0,"MASS":204,"NAME":"Bolzano","OWNER":9,
//              "SHIELD":100,"TORP.COUNT":0,"TORP.LCOUNT":0,"ISPLANET":1,"CONFIG.BEAMKILLRATE":1,
//              "CONFIG.BEAMCHARGERATE":1,"CONFIG.TORPMISSRATE":35,"CONFIG.TORPCHARGERATE":1,"CONFIG.CREWDEFENSERATE":0}]
//         },
//         {"MAGIC":0,"SEED":2493,"CAPABILITIES":0,"ALGORITHM":"PHost 3","UNIT":[
//             {"BEAM":1,"BEAM.COUNT":12,"CREW":178,"DAMAGE":0,"FIGHTER.BAYS":0,"FIGHTER.COUNT":0,"HULL":41,"ID":112,"IMAGE":64,
//              "LEVEL":0,"MASS":172,"NAME":"Wybuch-E","OWNER":4,"SHIELD":100,"TORP":1,"TORP.COUNT":0,
//              "TORP.LCOUNT":1,"ISPLANET":0,"CONFIG.BEAMKILLRATE":1,"CONFIG.BEAMCHARGERATE":1,"CONFIG.TORPMISSRATE":35,
//              "CONFIG.TORPCHARGERATE":1,"CONFIG.CREWDEFENSERATE":0},
//             {"BEAM":3,"BEAM.COUNT":2,"CREW":15,"DAMAGE":0,"FIGHTER.BAYS":0,
//              "FIGHTER.COUNT":0,"HULL":51,"ID":263,"IMAGE":86,"LEVEL":0,"MASS":162,"NAME":"HEART OF GOLD CLASS",
//              "OWNER":7,"SHIELD":100,"TORP.COUNT":0,"TORP.LCOUNT":0,"ISPLANET":0,"CONFIG.BEAMKILLRATE":1,
//              "CONFIG.BEAMCHARGERATE":1,"CONFIG.TORPMISSRATE":35,"CONFIG.TORPCHARGERATE":1,"CONFIG.CREWDEFENSERATE":0}]
//         },
//         {"MAGIC":0,"SEED":53958,"CAPABILITIES":0,"ALGORITHM":"PHost 3","UNIT":[                  // <-------------
//             {"BEAM":7,"BEAM.COUNT":12,"CREW":1249,"DAMAGE":0,"FIGHTER.BAYS":14,"FIGHTER.COUNT":129,"HULL":99,"ID":492,"IMAGE":144,
//              "LEVEL":0,"MASS":751,"NAME":"Alvilda the Goth","OWNER":11,"SHIELD":100,"TORP.COUNT":0,
//              "TORP.LCOUNT":0,"ISPLANET":0,"CONFIG.BEAMKILLRATE":1,"CONFIG.BEAMCHARGERATE":1,"CONFIG.TORPMISSRATE":35,
//              "CONFIG.TORPCHARGERATE":1,"CONFIG.CREWDEFENSERATE":0},
//             {"BEAM":2,"BEAM.COUNT":2,"CREW":0,"DAMAGE":0,"FIGHTER.BAYS":3,
//              "FIGHTER.COUNT":3,"ID":425,"IMAGE":0,"LEVEL":0,"MASS":110,"NAME":"Steenrod","OWNER":10,
//              "SHIELD":100,"TORP.COUNT":0,"TORP.LCOUNT":0,"ISPLANET":1,"CONFIG.BEAMKILLRATE":1,
//              "CONFIG.BEAMCHARGERATE":1,"CONFIG.TORPMISSRATE":35,"CONFIG.TORPCHARGERATE":1,"CONFIG.CREWDEFENSERATE":0}]
//         },
//         {"MAGIC":0,"SEED":3343,"CAPABILITIES":0,"ALGORITHM":"PHost 3","UNIT":[
//             {"BEAM":6,"BEAM.COUNT":2,"CREW":306,"DAMAGE":0,"FIGHTER.BAYS":6,"FIGHTER.COUNT":35,"HULL":72,"ID":69,"IMAGE":121,
//              "LEVEL":0,"MASS":237,"NAME":"REKA WINA","OWNER":5,"SHIELD":100,"TORP.COUNT":0,"TORP.LCOUNT":0,
//              "ISPLANET":0,"CONFIG.BEAMKILLRATE":3,"CONFIG.BEAMCHARGERATE":1,"CONFIG.TORPMISSRATE":35,
//              "CONFIG.TORPCHARGERATE":1,"CONFIG.CREWDEFENSERATE":0},
//             {"BEAM":1,"BEAM.COUNT":1,"CREW":0,"DAMAGE":0,"FIGHTER.BAYS":1,
//              "FIGHTER.COUNT":1,"ID":91,"IMAGE":0,"LEVEL":0,"MASS":101,"NAME":"Cavalieri","OWNER":9,
//              "SHIELD":100,"TORP.COUNT":0,"TORP.LCOUNT":0,"ISPLANET":1,"CONFIG.BEAMKILLRATE":1,
//              "CONFIG.BEAMCHARGERATE":1,"CONFIG.TORPMISSRATE":35,"CONFIG.TORPCHARGERATE":1,"CONFIG.CREWDEFENSERATE":0}]
//         },
//         {"MAGIC":0,"SEED":50854,"CAPABILITIES":0,"ALGORITHM":"PHost 3","UNIT":[
//             {"BEAM":10,"BEAM.COUNT":5,"CREW":148,"DAMAGE":0,"FIGHTER.BAYS":0,"FIGHTER.COUNT":0,"HULL":46,"ID":78,"IMAGE":76,
//              "LEVEL":0,"MASS":171,"NAME":"Grawitacja","OWNER":5,"SHIELD":100,"TORP":10,"TORP.COUNT":0,
//              "TORP.LCOUNT":3,"ISPLANET":0,"CONFIG.BEAMKILLRATE":3,"CONFIG.BEAMCHARGERATE":1,"CONFIG.TORPMISSRATE":35,
//              "CONFIG.TORPCHARGERATE":1,"CONFIG.CREWDEFENSERATE":0},
//             {"BEAM":5,"BEAM.COUNT":4,"CREW":0,"DAMAGE":0,"FIGHTER.BAYS":7,
//              "FIGHTER.COUNT":7,"ID":71,"IMAGE":0,"LEVEL":0,"MASS":144,"NAME":"Borda","OWNER":7,
//              "SHIELD":100,"TORP.COUNT":0,"TORP.LCOUNT":0,"ISPLANET":1,"CONFIG.BEAMKILLRATE":1,
//              "CONFIG.BEAMCHARGERATE":1,"CONFIG.TORPMISSRATE":35,"CONFIG.TORPCHARGERATE":1,"CONFIG.CREWDEFENSERATE":0}]
//         },
//         {"MAGIC":0,"SEED":58533,"CAPABILITIES":0,"ALGORITHM":"PHost 3","UNIT":[
//             {"BEAM":10,"BEAM.COUNT":5,"CREW":148,"DAMAGE":0,"FIGHTER.BAYS":0,"FIGHTER.COUNT":0,"HULL":46,"ID":370,"IMAGE":76,
//              "LEVEL":0,"MASS":171,"NAME":"Mussolini","OWNER":5,"SHIELD":100,"TORP":1,"TORP.COUNT":0,
//              "TORP.LCOUNT":3,"ISPLANET":0,"CONFIG.BEAMKILLRATE":3,"CONFIG.BEAMCHARGERATE":1,"CONFIG.TORPMISSRATE":35,
//              "CONFIG.TORPCHARGERATE":1,"CONFIG.CREWDEFENSERATE":0},
//             {"BEAM":3,"BEAM.COUNT":2,"CREW":111,"DAMAGE":16,"FIGHTER.BAYS":0,
//              "FIGHTER.COUNT":0,"HULL":3,"ID":479,"IMAGE":29,"LEVEL":0,"MASS":142,"NAME":"Bambo 479",
//              "OWNER":6,"SHIELD":84,"TORP.COUNT":0,"TORP.LCOUNT":0,"ISPLANET":0,"CONFIG.BEAMKILLRATE":1,
//              "CONFIG.BEAMCHARGERATE":1,"CONFIG.TORPMISSRATE":35,"CONFIG.TORPCHARGERATE":1,"CONFIG.CREWDEFENSERATE":0}]
//         },
//         {"MAGIC":0,"SEED":3722,"CAPABILITIES":0,"ALGORITHM":"PHost 3","UNIT":[
//             {"BEAM":10,"BEAM.COUNT":15,"CREW":787,"DAMAGE":0,"FIGHTER.BAYS":0,"FIGHTER.COUNT":0,"HULL":35,"ID":314,"IMAGE":68,
//              "LEVEL":0,"MASS":681,"NAME":"Chirurg","OWNER":4,"SHIELD":100,"TORP":10,"TORP.COUNT":53,
//              "TORP.LCOUNT":13,"ISPLANET":0,"CONFIG.BEAMKILLRATE":1,"CONFIG.BEAMCHARGERATE":1,"CONFIG.TORPMISSRATE":35,
//              "CONFIG.TORPCHARGERATE":1,"CONFIG.CREWDEFENSERATE":0},
//             {"BEAM":6,"BEAM.COUNT":5,"CREW":0,"DAMAGE":0,"FIGHTER.BAYS":8,
//              "FIGHTER.COUNT":8,"ID":294,"IMAGE":0,"LEVEL":0,"MASS":166,"NAME":"Mengoli","OWNER":7,
//              "SHIELD":100,"TORP.COUNT":0,"TORP.LCOUNT":0,"ISPLANET":1,"CONFIG.BEAMKILLRATE":1,
//              "CONFIG.BEAMCHARGERATE":1,"CONFIG.TORPMISSRATE":35,"CONFIG.TORPCHARGERATE":1,"CONFIG.CREWDEFENSERATE":0}]}];
    };

    game::vcr::Object convertObject(const Object& in)
    {
        game::vcr::Object result;
        result.setMass(in.mass);
        result.setIsPlanet(in.isPlanet);
        result.setName(in.name);
        result.setDamage(in.damage);
        result.setCrew(in.crew);
        result.setId(in.id);
        result.setOwner(in.owner);
        result.setPicture(in.image);
        result.setHull(in.hull);
        result.setBeamType(in.beamType);
        result.setNumBeams(in.numBeams);
        result.setExperienceLevel(in.experienceLevel);
        result.setNumBays(in.numBays);
        result.setTorpedoType(in.torpedoType);
        result.setNumTorpedoes(in.numTorpedoes);
        result.setNumFighters(in.numFighters);
        result.setNumLaunchers(in.numLaunchers);
        result.setShield(in.shield);
        result.setBeamKillRate(in.beamKillRate);
        result.setBeamChargeRate(in.beamChargeRate);
        result.setTorpMissRate(in.torpMissRate);
        result.setTorpChargeRate(in.torpChargeRate);
        result.setCrewDefenseRate(in.crewDefenseRate);
        return result;
    }

    void initConfig(game::config::HostConfiguration& config)
    {
        config[config.AllowAlternativeCombat].set(1);
        config[config.BayLaunchInterval].set(2);
        config[config.BayRechargeBonus].set(1);
        config[config.BayRechargeRate].set(40);
        config[config.BeamFiringRange].set(25000);
        config[config.BeamHitBonus].set(12);
        config[config.BeamHitFighterCharge].set(500);
        config[config.BeamHitFighterRange].set(100000);
        config[config.BeamHitOdds].set(70);
        config[config.BeamHitShipCharge].set(600);
        config[config.BeamRechargeBonus].set(4);
        config[config.BeamRechargeRate].set(4);
        config[config.CrewKillScaling].set(30);
        config[config.EModBayRechargeBonus].set(0);
        config[config.EModBayRechargeRate].set("1,2,3,4,4,4,4,4,4,4");
        config[config.EModBeamHitBonus].set(0);
        config[config.EModBeamHitFighterCharge].set(0);
        config[config.EModBeamHitOdds].set(0);
        config[config.EModBeamRechargeBonus].set(0);
        config[config.EModBeamRechargeRate].set(0);
        config[config.EModCrewKillScaling].set("-5,-10,-15,-20,-20,-20,-20,-20,-20,-20");
        config[config.EModFighterBeamExplosive].set(0);
        config[config.EModFighterBeamKill].set(0);
        config[config.EModFighterMovementSpeed].set(0);
        config[config.EModHullDamageScaling].set(0);
        config[config.EModMaxFightersLaunched].set(0);
        config[config.EModShieldDamageScaling].set(0);
        config[config.EModShieldKillScaling].set(0);
        config[config.EModStrikesPerFighter].set("1,2,3,4,4,4,4,4,4,4");
        config[config.EModTorpHitBonus].set(0);
        config[config.EModTorpHitOdds].set("9,18,27,35,35,35,35,35,35,35");
        config[config.EModTubeRechargeBonus].set(0);
        config[config.EModTubeRechargeRate].set("1,2,3,8,8,8,8,8,8,8");
        config[config.FighterBeamExplosive].set(8);
        config[config.FighterBeamKill].set(8);
        config[config.FighterFiringRange].set(3000);
        config[config.FighterKillOdds].set(10);
        config[config.FighterMovementSpeed].set(300);
        config[config.FireOnAttackFighters].set(1);
        config[config.HullDamageScaling].set(20);
        config[config.MaxFightersLaunched].set(30);
        config[config.PlayerRace].set("1,2,3,4,5,6,7,8,9,10,11");
        config[config.ShieldDamageScaling].set(40);
        config[config.ShieldKillScaling].set(0);
        config[config.ShipMovementSpeed].set(100);
        config[config.StandoffDistance].set(10000);
        config[config.StrikesPerFighter].set(5);
        config[config.TorpFiringRange].set(30000);
        config[config.TorpHitBonus].set(13);
        config[config.TorpHitOdds].set(55);
        config[config.TubeRechargeBonus].set(7);
        config[config.TubeRechargeRate].set(30);
    }
}

void
TestGameVcrClassicPVCRAlgorithm::testTF()
{
    // Surroundings
    game::vcr::classic::NullVisualizer vis;
    game::config::HostConfiguration config;
    game::spec::ShipList list;
    initShipList(list);
    initConfig(config);

    // First fight
    game::vcr::classic::PVCRAlgorithm testee(false, vis, config, list.beams(), list.launchers());
    game::vcr::Object left(convertObject(battles[0].object[0]));
    game::vcr::Object right(convertObject(battles[0].object[1]));
    uint16_t seed = battles[0].seed;
    bool result = testee.checkBattle(left, right, seed);
    TS_ASSERT(!result);
    
    testee.initBattle(left, right, seed);
    while (testee.playCycle()) {
        // nix
    }
    testee.doneBattle(left, right);

    // Record #1:
    //         Ending time 410 (6:50)
    //         left-destroyed
    //   S:  0  D:100  C:1241  A: 92   |     S:  0  D: 61  C:698  A: 14
    TS_ASSERT_EQUALS(testee.getTime(), 410);
    TS_ASSERT(testee.getResult().contains(game::vcr::classic::LeftDestroyed));
    TS_ASSERT(!testee.getResult().contains(game::vcr::classic::RightDestroyed));
    TS_ASSERT(!testee.getResult().contains(game::vcr::classic::LeftCaptured));
    TS_ASSERT(!testee.getResult().contains(game::vcr::classic::RightCaptured));
    TS_ASSERT_EQUALS(left.getDamage(), 100);
    TS_ASSERT_EQUALS(right.getDamage(), 61);
    TS_ASSERT_EQUALS(left.getShield(), 0);
    TS_ASSERT_EQUALS(right.getShield(), 0);
    TS_ASSERT_EQUALS(left.getCrew(), 1241);
    TS_ASSERT_EQUALS(right.getCrew(), 698);
    TS_ASSERT_EQUALS(left.getNumTorpedoes(), 0);
    TS_ASSERT_EQUALS(right.getNumTorpedoes(), 14);
    TS_ASSERT_EQUALS(left.getNumFighters(), 92);
    TS_ASSERT_EQUALS(right.getNumFighters(), 0);
}

void
TestGameVcrClassicPVCRAlgorithm::testCarriers()
{
    // Surroundings
    game::vcr::classic::NullVisualizer vis;
    game::config::HostConfiguration config;
    game::spec::ShipList list;
    initShipList(list);
    initConfig(config);

    // First fight
    game::vcr::classic::PVCRAlgorithm testee(false, vis, config, list.beams(), list.launchers());
    game::vcr::Object left(convertObject(battles[1].object[0]));
    game::vcr::Object right(convertObject(battles[1].object[1]));
    uint16_t seed = battles[1].seed;
    bool result = testee.checkBattle(left, right, seed);
    TS_ASSERT(!result);
    
    testee.initBattle(left, right, seed);
    while (testee.playCycle()) {
        // nix
    }
    testee.doneBattle(left, right);
    
    // Record #12: (two fighter units)
    //         Ending time 245 (4:05)
    //         right-destroyed
    //   S:100  D:  0  C:1249  A:127   |     S:  0  D:100  C:  0  A:  0
    TS_ASSERT_EQUALS(testee.getTime(), 245);
    TS_ASSERT(!testee.getResult().contains(game::vcr::classic::LeftDestroyed));
    TS_ASSERT(testee.getResult().contains(game::vcr::classic::RightDestroyed));
    TS_ASSERT(!testee.getResult().contains(game::vcr::classic::LeftCaptured));
    TS_ASSERT(!testee.getResult().contains(game::vcr::classic::RightCaptured));
    TS_ASSERT_EQUALS(left.getDamage(), 0);
    TS_ASSERT_EQUALS(right.getDamage(), 100);
    TS_ASSERT_EQUALS(left.getShield(), 100);
    TS_ASSERT_EQUALS(right.getShield(), 0);
    TS_ASSERT_EQUALS(left.getCrew(), 1249);
    TS_ASSERT_EQUALS(right.getCrew(), 0);
    TS_ASSERT_EQUALS(left.getNumTorpedoes(), 0);
    TS_ASSERT_EQUALS(right.getNumTorpedoes(), 0);
    TS_ASSERT_EQUALS(left.getNumFighters(), 127);
    TS_ASSERT_EQUALS(right.getNumFighters(), 0);
}

/** Test behaviour if bonus computation overflows 100%.

    PCC2 bug #304: With the wrong implementation of randomRange100LT, beams fail to hit once the effective BeamHitOdds goes over 100.
    This means a ship with numerically better beams will perform much worse than expected,
    which gets especially visible against fighters where all beams do essentially the same damage.

    This test sets up two otherwise identical ships, one with type-1 beams, one with type-10.
    The expectation is that the type-10 ship wins. */
void
TestGameVcrClassicPVCRAlgorithm::testRandomBonus()
{
    // Surroundings
    game::vcr::classic::NullVisualizer vis;
    game::config::HostConfiguration config;
    game::spec::ShipList list;
    initShipList(list);
    initConfig(config);

    // BeamHitOdds = 88
    // BeamHitBonus = 12
    //  --> beam 1:   (1+2)*12/100   = 0   -> 88
    //  --> beam 10:  (40+80)*12/100 = 14  -> 102
    config[config.BeamHitOdds].set(88);

    //                                bt, bc  crew dmg bay ftr ht id im exp mass         ow  shld tt tc lc planet ------nu------
    const struct Object leftShip  = {  1, 10, 1000, 0, 8, 100, 1, 1, 1,  0, 400, "Left",  6, 100, 0, 0, 0, false, 1, 1, 35, 1, 0 };
    const struct Object rightShip = { 10, 10, 1000, 0, 8, 100, 1, 1, 1,  0, 400, "Right", 7, 100, 0, 0, 0, false, 1, 1, 35, 1, 0 };

    // First fight
    game::vcr::classic::PVCRAlgorithm testee(false, vis, config, list.beams(), list.launchers());
    game::vcr::Object left(convertObject(leftShip));
    game::vcr::Object right(convertObject(rightShip));
    uint16_t seed = 0;
    bool result = testee.checkBattle(left, right, seed);
    TS_ASSERT(!result);
    
    testee.initBattle(left, right, seed);
    while (testee.playCycle()) {
        // nix
    }
    testee.doneBattle(left, right);
    
    TS_ASSERT_EQUALS(testee.getTime(), 617);
    TS_ASSERT(testee.getResult().contains(game::vcr::classic::LeftDestroyed));
    TS_ASSERT(!testee.getResult().contains(game::vcr::classic::RightDestroyed));
    TS_ASSERT(!testee.getResult().contains(game::vcr::classic::LeftCaptured));
    TS_ASSERT(!testee.getResult().contains(game::vcr::classic::RightCaptured));
    TS_ASSERT_EQUALS(left.getDamage(), 100);
    TS_ASSERT_EQUALS(right.getDamage(), 53);
    TS_ASSERT_EQUALS(left.getShield(), 0);
    TS_ASSERT_EQUALS(right.getShield(), 0);
    TS_ASSERT_EQUALS(left.getCrew(), 851);
    TS_ASSERT_EQUALS(right.getCrew(), 921);
    TS_ASSERT_EQUALS(left.getNumTorpedoes(), 0);
    TS_ASSERT_EQUALS(right.getNumTorpedoes(), 0);
    TS_ASSERT_EQUALS(left.getNumFighters(), 20);
    TS_ASSERT_EQUALS(right.getNumFighters(), 61);
}
