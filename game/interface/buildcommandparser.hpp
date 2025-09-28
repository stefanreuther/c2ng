/**
  *  \file game/interface/buildcommandparser.hpp
  *  \brief Class game::interface::BuildCommandParser
  */
#ifndef C2NG_GAME_INTERFACE_BUILDCOMMANDPARSER_HPP
#define C2NG_GAME_INTERFACE_BUILDCOMMANDPARSER_HPP

#include "afl/string/translator.hpp"
#include "game/map/universe.hpp"
#include "game/root.hpp"
#include "game/spec/shiplist.hpp"
#include "game/types.hpp"
#include "interpreter/taskeditor.hpp"
#include "interpreter/taskpredictor.hpp"

namespace game { namespace interface {

    /** Parser for "Build" commands.
        This parses a single independant Build command and generates information about it.
        Information includes a textual description of the order, costs, and missing amounts.
        Supported commands include BuildShip, BuildBase, ship component building (BuildHulls etc.),
        and structure building (BuildFactories etc.).

        To use,
        - create;
        - call setLimit or loadLimit to support partially executed commands;
        - call predictInstruction(), indirectly using TaskPredictor::predictStatement();
        - extract result using getResult(). */
    class BuildCommandParser : public interpreter::TaskPredictor {
     public:
        /** Type of order. */
        enum OrderType {
            OrderType_Other,
            OrderType_Ship
        };

        /** Result. */
        struct Result {
            /** Textual information about things to be built.
                In case of a ship, output of ShipBuildOrder::describe(). */
            afl::data::StringList_t info;

            /** Remaining cost. */
            game::spec::Cost cost;

            /** Missing resources required to complete command. */
            game::spec::Cost missingAmount;

            /** True if this is a ship build order. */
            OrderType type;

            Result()
                : info(), cost(), missingAmount(), type(OrderType_Other)
                { }
        };

        /** Constructor.
            @param pl        Planet
            @param shipList  Ship list (for validating parameters, costs)
            @param root      Root (for configuration)
            @param tx        Translator */
        BuildCommandParser(game::map::Planet& pl,
                           game::spec::ShipList& shipList,
                           Root& root,
                           afl::string::Translator& tx);

        /** Set limit for commands that build multiple items (e.g.\ "BuildFactoriesWait").
            If nonzero and in range, assumes that this command will build only so many structures
            instead of the amount requested.

            @param n Limit
            @see loadLimit() */
        void setLimit(int n);

        /** Check task for applicable limit.
            If the process is currently executing the given command,
            retrieve the "Build.Remainder" variable.
            This assumes that the command is implemented using a "Do .. While Build.Remainder" loop.

            @param editor TaskEditor
            @param pc     Program counter */
        void loadLimit(const interpreter::TaskEditor& editor, size_t pc);

        /** Get result.
            @return result, if any. */
        std::auto_ptr<Result> getResult();

        // TaskPredictor:
        virtual bool predictInstruction(const String_t& name, interpreter::Arguments& args);

     private:
        game::map::Planet& m_planet;
        game::spec::ShipList& m_shipList;
        Root& m_root;
        afl::string::Translator& m_translator;
        int m_limit;
        std::auto_ptr<Result> m_result;

        void handleBuildShip(interpreter::Arguments& args);
        void handleBuildBase(interpreter::Arguments& args);
        void handleBuildStructure(interpreter::Arguments& args, PlanetaryBuilding type);
        void handleBuildParts(interpreter::Arguments& args, TechLevel area);

        int getLimitedAmount(int requested) const;
    };

} }

#endif
