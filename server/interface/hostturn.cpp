/**
  *  \file server/interface/hostturn.cpp
  *  \brief Interface server::interface::HostTurn
  */

#include "server/interface/hostturn.hpp"

const int32_t server::interface::HostTurn::MissingTurn;
const int32_t server::interface::HostTurn::GreenTurn;
const int32_t server::interface::HostTurn::YellowTurn;
const int32_t server::interface::HostTurn::RedTurn;
const int32_t server::interface::HostTurn::BadTurn;
const int32_t server::interface::HostTurn::StaleTurn;
const int32_t server::interface::HostTurn::NeedlessTurn;

const int32_t server::interface::HostTurn::TemporaryTurnFlag;


server::interface::HostTurn::Result::Result()
    : state(0),
      output(),
      gameId(0),
      slot(0),
      previousState(0),
      turnNumber(0),
      userId(),
      gameName(),
      allowTemp(false)
{ }

server::interface::HostTurn::Result::~Result()
{ }
