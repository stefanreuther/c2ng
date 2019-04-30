/**
  *  \file game/data/friendlycode.hpp
  */
#ifndef C2NG_GAME_DATA_FRIENDLYCODE_HPP
#define C2NG_GAME_DATA_FRIENDLYCODE_HPP

#include <vector>
#include "game/spec/friendlycode.hpp"
#include "game/playerlist.hpp"
#include "game/spec/friendlycodelist.hpp"

namespace game { namespace data {

    class FriendlyCode {
     public:
        FriendlyCode(const String_t& code, const String_t& description);

        const String_t& getCode() const;

        const String_t& getDescription() const;

     private:
        String_t m_code;
        String_t m_description;
    };

    typedef std::vector<FriendlyCode> FriendlyCodeList_t;

    void packFriendlyCodeList(FriendlyCodeList_t& out, const game::spec::FriendlyCodeList& in, const PlayerList& players);

} }

#endif
