/**
  *  \file game/interface/globalproperty.hpp
  */
#ifndef C2NG_GAME_INTERFACE_GLOBALPROPERTY_HPP
#define C2NG_GAME_INTERFACE_GLOBALPROPERTY_HPP

#include "afl/data/value.hpp"
#include "game/session.hpp"

namespace game { namespace interface {

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

    afl::data::Value* getGlobalProperty(GlobalProperty igp, Session& session);
    void setGlobalProperty(GlobalProperty igp, Session& session, afl::data::Value* value);

} }

#endif
