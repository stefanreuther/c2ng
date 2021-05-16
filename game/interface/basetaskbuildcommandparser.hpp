/**
  *  \file game/interface/basetaskbuildcommandparser.hpp
  *  \brief Class game::interface::BaseTaskBuildCommandParser
  */
#ifndef C2NG_GAME_INTERFACE_BASETASKBUILDCOMMANDPARSER_HPP
#define C2NG_GAME_INTERFACE_BASETASKBUILDCOMMANDPARSER_HPP

#include "game/shipbuildorder.hpp"
#include "game/spec/shiplist.hpp"
#include "interpreter/taskpredictor.hpp"

namespace game { namespace interface {

    /** Starbase Auto Task Build Command Parser.
        This parses a single independant BuildShip/EnqueueShip command. */
    class BaseTaskBuildCommandParser : public interpreter::TaskPredictor {
     public:
        /** Constructor.
            \param shipList  Ship list (for resolving hull references) */
        BaseTaskBuildCommandParser(const game::spec::ShipList& shipList);

        // TaskPredictor:
        virtual bool predictInstruction(const String_t& name, interpreter::Arguments& args);

        const String_t& getVerb() const;
        const ShipBuildOrder& getOrder() const;

     private:
        const game::spec::ShipList& m_shipList;
        String_t m_verb;
        ShipBuildOrder m_order;

        void postBuildOrder(ShipBuildOrder order);
    };

} }

#endif
