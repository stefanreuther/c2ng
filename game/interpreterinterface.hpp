/**
  *  \file game/interpreterinterface.hpp
  *  \brief Interface game::InterpreterInterface
  */
#ifndef C2NG_GAME_INTERPRETERINTERFACE_HPP
#define C2NG_GAME_INTERPRETERINTERFACE_HPP

#include "afl/base/optional.hpp"
#include "afl/data/value.hpp"
#include "afl/string/string.hpp"

namespace game {

    /** Interface for game components to interpreter properties (and then some).
        This implements a few cross-links required by game components.
        - access to interpreter properties
        - access to specification elements */
    class InterpreterInterface {
     public:
        /** Scope of a query. */
        enum Scope {
            Ship,
            Planet,
            Base
        };

        virtual ~InterpreterInterface()
            { }

        /** Get comment for an object.
            \param scope Object type
            \param id Object Id
            \return comment, "" if none */
        virtual String_t getComment(Scope scope, int id) const = 0;

        /** Check whether object has an auto-task.
            \param scope Object type
            \param id Object Id
            \return result */
        virtual bool hasTask(Scope scope, int id) const = 0;

        /** Get short name of a hull.
            \param nr Hull Id
            \return Name, if available */
        virtual afl::base::Optional<String_t> getHullShortName(int nr) const = 0;

        /** Get adjective name of a player.
            \param nr Player number
            \return Name, if available */
        virtual afl::base::Optional<String_t> getPlayerAdjective(int nr) const = 0;
    };

}

#endif
