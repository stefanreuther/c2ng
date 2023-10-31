/**
  *  \file server/play/friendlycodepacker.cpp
  *  \brief Class server::play::FriendlyCodePacker
  */

#include "server/play/friendlycodepacker.hpp"
#include "afl/data/hash.hpp"
#include "afl/data/hashvalue.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/vectorvalue.hpp"
#include "game/interface/friendlycodeproperty.hpp"
#include "game/root.hpp"
#include "game/spec/friendlycode.hpp"
#include "game/spec/friendlycodelist.hpp"
#include "game/spec/shiplist.hpp"

using afl::data::Hash;
using afl::data::HashValue;
using afl::data::Vector;
using afl::data::VectorValue;
using game::spec::FriendlyCode;
using game::spec::FriendlyCodeList;
using game::spec::ShipList;
using game::Root;
namespace gi = game::interface;

server::play::FriendlyCodePacker::FriendlyCodePacker(const game::spec::ShipList& shipList, const game::Root& root, afl::string::Translator& tx)
    : m_shipList(shipList), m_root(root), m_translator(tx)
{ }

server::Value_t*
server::play::FriendlyCodePacker::buildValue() const
{
    Vector::Ref_t result = Vector::create();
    const FriendlyCodeList& friendlyCodes = m_shipList.friendlyCodes();
    for (size_t i = 0, n = friendlyCodes.size(); i < n; ++i) {
        if (const FriendlyCode* fc = friendlyCodes.at(i)) {
            Hash::Ref_t data = Hash::create();
            data->setNew("NAME",        gi::getFriendlyCodeProperty(*fc, gi::ifpName,        m_root.playerList(), m_translator));
            data->setNew("DESCRIPTION", gi::getFriendlyCodeProperty(*fc, gi::ifpDescription, m_root.playerList(), m_translator));
            data->setNew("FLAGS",       gi::getFriendlyCodeProperty(*fc, gi::ifpFlags,       m_root.playerList(), m_translator));
            data->setNew("RACES",       gi::getFriendlyCodeProperty(*fc, gi::ifpRaces,       m_root.playerList(), m_translator));
            result->pushBackNew(new HashValue(data));
        }
    }
    return new VectorValue(result);
}

String_t
server::play::FriendlyCodePacker::getName() const
{
    return "fcode";
}
