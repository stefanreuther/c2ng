/**
  *  \file client/tiles/planettasktile.cpp
  *  \brief Class client::tiles::PlanetTaskTile
  */

#include "client/tiles/planettasktile.hpp"
#include "ui/spacer.hpp"

client::tiles::PlanetTaskTile::PlanetTaskTile(ui::Root& root, gfx::KeyEventConsumer& keyHandler, afl::string::Translator& tx)
    : TaskMessageTile(root, keyHandler, tx)
{
    // ex WPlanetAutoTaskCommandTile::init
    // statusPart() remains empty, but add a spacer so it consumes all remaining space
    statusPart().add(deleter().addNew(new ui::Spacer()));

    // Command buttons
    addCommandButton('1', tx("1 - Orders"));
    addCommandButton('2', tx("2 - Cargo"));
    addCommandButton('3', tx("3 - Misc."));
    commandPart().add(deleter().addNew(new ui::Spacer()));
}

client::tiles::PlanetTaskTile::~PlanetTaskTile()
{ }
