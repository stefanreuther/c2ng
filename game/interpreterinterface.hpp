/**
  *  \file game/interpreterinterface.hpp
  *  \brief Interface game::InterpreterInterface
  */
#ifndef C2NG_GAME_INTERPRETERINTERFACE_HPP
#define C2NG_GAME_INTERPRETERINTERFACE_HPP

#include "afl/string/string.hpp"
#include "afl/data/value.hpp"

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
        virtual String_t getComment(Scope scope, int id) = 0;

        /** Check whether object has an auto-task.
            \param scope Object type
            \param id Object Id
            \return result */
        virtual bool hasTask(Scope scope, int id) = 0;

        /** Get short name of a hull.
            \param nr [in] Hull Id
            \param out [out] Name
            \return true on success, false if name is not available */
        virtual bool getHullShortName(int nr, String_t& out) = 0;

        /** Get adjective name of a player.
            \param nr [in] Player number
            \param out [out] Name
            \return true on success, false if name is not available */
        virtual bool getPlayerAdjective(int nr, String_t& out) = 0;
    };

}

#endif
