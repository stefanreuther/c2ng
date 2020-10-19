/**
  *  \file game/test/shiplist.cpp
  *  \brief Ship List Initialisation Functions for Testing
  */

#include "game/test/shiplist.hpp"

using game::spec::Cost;

namespace {
    /*
     *  Tables
     */

    struct CostStruct {
        uint8_t mc, t, d, m;
    };

    struct Beam {
        char name[20];
        CostStruct cost;
        uint8_t mass;
        uint8_t techLevel;
        uint8_t killPower;
        uint8_t damagePower;
    };

    const Beam PLIST_BEAMS[] = {
        // Name                 $$ Tr Du Mo  m TL Ki Da
        {"Laser Cannon",      {  1, 1, 0, 1},0, 1, 1, 2},
        {"Kill-O-Zap",        {  5, 1, 2, 0},0, 2,10, 1},
        {"Desintegrator",     { 10, 3, 1, 2},1, 4, 7,10},
        {"Phaser",            { 20, 5, 0, 2},1, 6,15,25},
        {"Disruptor",         { 45,10, 5, 5},1, 7,40,10},
        {"Electron Ram",      { 50,15, 5,10},2, 7,20,40},
        {"Ion Cannon",        { 60, 5,20, 5},1, 8,10,45},
        {"Turbolaser Battery",{ 90,20, 5,10},2, 9,30,60},
        {"Inpotron Cannon",   {110,10,10,10},3,10,70,35},
        {"Multitraf Spiral",  {130,25,15,10},3,10,40,80}
    };

    const Beam STANDARD_BEAMS[] = {
        // Name              $$ Tr Du Mo  m TL Ki Da
        {"Laser",           { 1, 1, 0, 0},1, 1,10, 3},
        {"X-Ray Laser",     { 2, 1, 0, 0},1, 1,15, 1},
        {"Plasma Bolt",     { 5, 1, 2, 0},2, 2, 3,10},
        {"Blaster",         {10, 1,12, 1},4, 3,10,25},
        {"Positron Beam",   {12, 1,12, 5},3, 4, 9,29},
        {"Disruptor",       {13, 1,12, 1},4, 5,30,20},
        {"Heavy Blaster",   {31, 1,12,14},7, 6,20,40},
        {"Phaser",          {35, 1,12,30},5, 7,30,35},
        {"Heavy Disruptor", {36, 1,17,37},7, 8,50,35},
        {"Heavy Phaser",    {54, 1,12,55},6,10,35,45}
    };

    struct Torpedo {
        char name[20];
        CostStruct torpedoCost;
        CostStruct launcherCost;
        uint8_t mass;
        uint8_t techLevel;
        uint8_t killPower;
        uint8_t damagePower;
    };

    const Torpedo PLIST_TORPEDOES[] = {
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

    const Torpedo STANDARD_TORPEDOES[] = {
        {"Mark 1 Photon", {1,1,1,1},  {1,1,1,0},  2,1,4,5},
        {"Proton torp",   {2,1,1,1},  {4,1,0,0},  2,2,6,8},
        {"Mark 2 Photon", {5,1,1,1},  {4,1,4,0},  2,3,3,10},
        {"Gamma Bomb",    {10,1,1,1}, {6,1, 3,1}, 4,3,15,2},
        {"Mark 3 Photon", {12,1,1,1}, {5,1,1,5},  2,4,9,15},
        {"Mark 4 Photon", {13,1,1,1}, {20,1,4,1}, 2,5,13,30},
        {"Mark 5 Photon", {31,1,1,1}, {57,1,7,14},3,6,17,35},
        {"Mark 6 Photon", {35,1,1,1}, {100,1,2,7},2,7,23,40},
        {"Mark 7 Photon", {36,1,1,1}, {120,1,3,8},3,8,25,48},
        {"Mark 8 Photon", {54,1,1,1}, {190,1,1,9},3,10,35,55}
    };


    /*
     *  Functions
     */

    Cost convertCost(const CostStruct& c)
    {
        Cost result;
        result.set(result.Duranium, c.d);
        result.set(result.Tritanium, c.t);
        result.set(result.Molybdenum, c.m);
        result.set(result.Money, c.mc);
        return result;
    }

    void initBeams(game::spec::ShipList& shipList, afl::base::Memory<const Beam> beams)
    {
        game::Id_t i = 0;
        while (const Beam* in = beams.eat()) {
            ++i;
            if (game::spec::Beam* out = shipList.beams().create(i)) {
                out->setKillPower(in->killPower);
                out->setDamagePower(in->damagePower);
                out->setMass(in->mass);
                out->setTechLevel(in->techLevel);
                out->setName(in->name);
                out->cost() = convertCost(in->cost);
            }
        }
    }

    void initTorpedoes(game::spec::ShipList& shipList, afl::base::Memory<const Torpedo> torps)
    {
        game::Id_t i = 0;
        while (const Torpedo* in = torps.eat()) {
            ++i;
            if (game::spec::TorpedoLauncher* out = shipList.launchers().create(i)) {
                out->setKillPower(in->killPower);
                out->setDamagePower(in->damagePower);
                out->setMass(in->mass);
                out->setTechLevel(in->techLevel);
                out->setName(in->name);
                out->cost() = convertCost(in->launcherCost);
                out->torpedoCost() = convertCost(in->torpedoCost);
            }
        }
    }
}

void
game::test::initStandardBeams(game::spec::ShipList& shipList)
{
    initBeams(shipList, STANDARD_BEAMS);
}

void
game::test::initStandardTorpedoes(game::spec::ShipList& shipList)
{
    initTorpedoes(shipList, STANDARD_TORPEDOES);
}

void
game::test::initPListBeams(game::spec::ShipList& shipList)
{
    initBeams(shipList, PLIST_BEAMS);
}

void
game::test::initPListTorpedoes(game::spec::ShipList& shipList)
{
    initTorpedoes(shipList, PLIST_TORPEDOES);
}

void
game::test::addOutrider(game::spec::ShipList& list)
{
    game::spec::Hull* p = list.hulls().create(OUTRIDER_HULL_ID);
    p->setName("OUTRIDER CLASS SCOUT");
    p->setInternalPictureNumber(9);
    p->setExternalPictureNumber(9);
    p->cost().set(Cost::Tritanium, 40);
    p->cost().set(Cost::Duranium, 20);
    p->cost().set(Cost::Molybdenum, 5);
    p->cost().set(Cost::Money, 50);
    p->setMaxFuel(260);
    p->setMaxCrew(180);
    p->setNumEngines(1);
    p->setMass(75);
    p->setTechLevel(1);
    p->setMaxCargo(40);
    p->setNumBays(0);
    p->setMaxLaunchers(0);
    p->setMaxBeams(1);
}

void
game::test::addGorbie(game::spec::ShipList& list)
{
    game::spec::Hull* p = list.hulls().create(GORBIE_HULL_ID);
    p->setName("GORBIE CLASS BATTLECARRIER");
    p->setInternalPictureNumber(107);
    p->setExternalPictureNumber(107);
    p->cost().set(Cost::Tritanium, 471);
    p->cost().set(Cost::Duranium, 142);
    p->cost().set(Cost::Molybdenum, 442);
    p->cost().set(Cost::Money, 790);
    p->setMaxFuel(1760);
    p->setMaxCrew(2287);
    p->setNumEngines(6);
    p->setMass(980);
    p->setTechLevel(10);
    p->setMaxCargo(250);
    p->setNumBays(10);
    p->setMaxLaunchers(0);
    p->setMaxBeams(10);
}

void
game::test::addAnnihilation(game::spec::ShipList& list)
{
    game::spec::Hull* p = list.hulls().create(ANNIHILATION_HULL_ID);
    p->setName("ANNIHILATION CLASS BATTLESHIP");
    p->setInternalPictureNumber(84);
    p->setExternalPictureNumber(84);
    p->cost().set(Cost::Tritanium, 343);
    p->cost().set(Cost::Duranium, 340);
    p->cost().set(Cost::Molybdenum, 550);
    p->cost().set(Cost::Money, 910);
    p->setMaxFuel(1260);
    p->setMaxCrew(2910);
    p->setNumEngines(6);
    p->setMass(960);
    p->setTechLevel(10);
    p->setMaxCargo(320);
    p->setNumBays(0);
    p->setMaxLaunchers(10);
    p->setMaxBeams(10);
}

void
game::test::addNovaDrive(game::spec::ShipList& list)
{
    game::spec::Engine* p = list.engines().create(NOVA_ENGINE_ID);
    p->setName("Nova Drive 5");
    p->cost().set(Cost::Tritanium, 3);
    p->cost().set(Cost::Duranium, 3);
    p->cost().set(Cost::Molybdenum, 7);
    p->cost().set(Cost::Money, 25);
    p->setTechLevel(5);
    p->setFuelFactor(1, 100);
    p->setFuelFactor(2, 415);
    p->setFuelFactor(3, 940);
    p->setFuelFactor(4, 1700);
    p->setFuelFactor(5, 2600);
    p->setFuelFactor(6, 10500);
    p->setFuelFactor(7, 14300);
    p->setFuelFactor(8, 23450);
    p->setFuelFactor(9, 72900);
}

void
game::test::addTranswarp(game::spec::ShipList& list)
{
    game::spec::Engine* p = list.engines().create(TRANSWARP_ENGINE_ID);
    p->setName("Transwarp Drive");
    p->cost().set(Cost::Tritanium, 3);
    p->cost().set(Cost::Duranium, 16);
    p->cost().set(Cost::Molybdenum, 35);
    p->cost().set(Cost::Money, 300);
    p->setTechLevel(10);
    p->setFuelFactor(1, 100);
    p->setFuelFactor(2, 400);
    p->setFuelFactor(3, 900);
    p->setFuelFactor(4, 1600);
    p->setFuelFactor(5, 2500);
    p->setFuelFactor(6, 3600);
    p->setFuelFactor(7, 4900);
    p->setFuelFactor(8, 6400);
    p->setFuelFactor(9, 8100);
}
