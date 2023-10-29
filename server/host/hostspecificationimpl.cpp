/**
  *  \file server/host/hostspecificationimpl.cpp
  *  \brief Class server::host::HostSpecificationImpl
  */

#include "server/host/hostspecificationimpl.hpp"
#include "afl/data/hashvalue.hpp"
#include "afl/data/stringvalue.hpp"
#include "afl/io/internalstream.hpp"
#include "afl/io/json/writer.hpp"
#include "server/errors.hpp"
#include "server/host/game.hpp"
#include "server/host/gamearbiter.hpp"
#include "server/host/root.hpp"
#include "server/host/session.hpp"
#include "util/string.hpp"

namespace {
    String_t getToolPath(server::host::Root& root, const String_t& toolName)
    {
        return root.toolRoot().byName(toolName).stringField("path").get();
    }
}

server::host::HostSpecificationImpl::HostSpecificationImpl(const Session& session, Root& root, server::host::spec::Publisher& pub)
    : m_session(session),
      m_root(root),
      m_publisher(pub)
{ }

server::Value_t*
server::host::HostSpecificationImpl::getShiplistData(String_t shiplistId, Format format, const afl::data::StringList_t& keys)
{
    return getShiplistDataWithFlak(shiplistId, format, keys, "flak");
}

server::Value_t*
server::host::HostSpecificationImpl::getGameData(int32_t gameId, Format format, const afl::data::StringList_t& keys)
{
    // Check existence and permission
    GameArbiter::Guard guard(m_root.arbiter(), gameId, GameArbiter::Simple);
    Game game(m_root, gameId);
    m_session.checkPermission(game, Game::ReadPermission);

    // Does game use FLAK? If so, use that.
    String_t flakTool = "flak";
    afl::data::StringList_t tools;
    game.tools().getAll(tools);
    for (size_t i = 0; i < tools.size(); ++i) {
        const String_t toolId = tools[i];
        if (util::strStartsWith(toolId, "flak") && m_root.toolRoot().byName(toolId).stringField("kind").get() == "combat") {
            flakTool = toolId;
            break;
        }
    }

    // If master has not run, return shiplist instead.
    if (!game.getConfigInt("masterHasRun") != 0) {
        return getShiplistDataWithFlak(game.getConfig("shiplist"), format, keys, flakTool);
    } else {
        // Check game directory
        String_t path = game.getDirectory();
        if (path.empty()) {
            throw std::runtime_error(ITEM_NOT_FOUND);
        }
        path += "/data";

        // Retrieve data
        // xref Root::invalidateGameData
        return formatResult(m_publisher.getSpecificationData(path, getToolPath(m_root, flakTool), keys), format);
    }
}

server::Value_t*
server::host::HostSpecificationImpl::formatResult(afl::data::Hash::Ref_t result, Format fmt)
{
    switch (fmt) {
     case Direct:
        return new afl::data::HashValue(result);

     case JsonString: {
        afl::io::InternalStream sink;
        afl::io::json::Writer(sink).visitHash(*result);
        return new afl::data::StringValue(afl::string::fromBytes(sink.getContent()));
     }
    }

    return 0;
}

server::Value_t*
server::host::HostSpecificationImpl::getShiplistDataWithFlak(String_t shiplistId, Format format, const afl::data::StringList_t& keys, String_t flakTool)
{
    // Retrieve path; also checks whether ship list exists
    String_t path = m_root.shipListRoot().byName(shiplistId).stringField("path").get();
    if (path.empty()) {
        throw std::runtime_error(ITEM_NOT_FOUND);
    }

    // Retrieve data
    // xref Root::invalidateShipListData
    return formatResult(m_publisher.getSpecificationData(path, getToolPath(m_root, flakTool), keys), format);
}
