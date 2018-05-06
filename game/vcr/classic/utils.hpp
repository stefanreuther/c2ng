/**
  *  \file game/vcr/classic/utils.hpp
  */
#ifndef C2NG_GAME_VCR_CLASSIC_UTILS_HPP
#define C2NG_GAME_VCR_CLASSIC_UTILS_HPP

#include "game/session.hpp"
#include "game/vcr/classic/types.hpp"
#include "game/spec/shiplist.hpp"
#include "game/vcr/object.hpp"
#include "game/teamsettings.hpp"
#include "afl/string/translator.hpp"

namespace game { namespace vcr { namespace classic {

    class Database;

    Database* getDatabase(Session& s);

    // String_t getImageResourceId(const Object& object, Side side, const game::spec::ShipList& shipList);

    String_t formatBattleResult(BattleResult_t result,
                                const String_t& leftName, TeamSettings::Relation leftRelation,
                                const String_t& rightName, TeamSettings::Relation rightRelation,
                                const String_t& annotation,
                                afl::string::Translator& tx);

} } }

#endif
