/**
  *  \file game/interface/globalproperty.hpp
  *  \brief Enum game::interface::GlobalProperty
  */
#ifndef C2NG_GAME_INTERFACE_GLOBALPROPERTY_HPP
#define C2NG_GAME_INTERFACE_GLOBALPROPERTY_HPP

#include "afl/data/value.hpp"
#include "game/session.hpp"

namespace game { namespace interface {

    /** Global property identifier.

        Global properties generally have some builtin "magic".
        Other global values, such as built-in functions like "Planet()", "Ufo()", are implemented
        as regular global values in interpreter::World::globalPropertyNames() / interpreter::World::globalValues().
        GlobalContext is responsible for publishing both. */
    enum GlobalProperty {
        igpFileFormatLocal,
        igpFileFormatRemote,
        igpGameDirectory,
        igpMyInMsgs,
        igpMyOutMsgs,
        igpMyVCRs,
        igpRootDirectory,
        igpSelectionLayer,
        igpSystemLanguage,
        igpSystemProgram,
        igpSystemVersion,
        igpSystemVersionCode,
        igpSystemHasPassword,
        igpSystemHost,
        igpSystemHostCode,
        igpSystemHostVersion,
        igpRandomSeed,
        igpRegSharewareFlag,
        igpRegSharewareText,
        igpRegStr1,
        igpRegStr2,
        igpTurnNumber,
        igpTurnDate,
        igpTurnIsNew,
        igpTurnTime
    };

    /** Get global property.
        @param igp      Property identifier
        @param session  Session
        @return newly-allocated value */
    afl::data::Value* getGlobalProperty(GlobalProperty igp, Session& session);

    /** Set global property.
        @param igp      Property identifier
        @param session  Session
        @param value    New value
        @throw interpreter::Error if property is not assignable or value is invalid */
    void setGlobalProperty(GlobalProperty igp, Session& session, const afl::data::Value* value);

} }

#endif
