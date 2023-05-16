/**
  *  \file client/si/nullcontrol.cpp
  *  \brief Class client::si::NullControl
  */

#include "client/si/nullcontrol.hpp"
#include "client/si/userside.hpp"

void
client::si::NullControl::handleStateChange(RequestLink2 link, OutputState::Target /*target*/)
{
    fail(link);
}

void
client::si::NullControl::handleEndDialog(RequestLink2 link, int /*code*/)
{
    fail(link);
}

void
client::si::NullControl::handlePopupConsole(RequestLink2 link)
{
    fail(link);
}

void
client::si::NullControl::handleScanKeyboardMode(RequestLink2 link)
{
    fail(link);
}

void
client::si::NullControl::handleSetView(RequestLink2 link, String_t /*name*/, bool /*withKeymap*/)
{
    fail(link);
}

void
client::si::NullControl::handleUseKeymap(RequestLink2 link, String_t /*name*/, int /*prefix*/)
{
    fail(link);
}

void
client::si::NullControl::handleOverlayMessage(RequestLink2 link, String_t /*text*/)
{
    fail(link);
}

afl::base::Optional<game::Id_t>
client::si::NullControl::getFocusedObjectId(game::Reference::Type type) const
{
    return defaultGetFocusedObjectId(type);
}

game::interface::ContextProvider*
client::si::NullControl::createContextProvider()
{
    return 0;
}

void
client::si::NullControl::fail(RequestLink2 link)
{
    interface().continueProcessWithFailure(link, "Context error");
}
