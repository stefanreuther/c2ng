/**
  *  \file game/interface/vcrsidefunction.hpp
  */
#ifndef C2NG_GAME_INTERFACE_VCRSIDEFUNCTION_HPP
#define C2NG_GAME_INTERFACE_VCRSIDEFUNCTION_HPP

#include "interpreter/indexablevalue.hpp"
#include "game/interface/vcrsidecontext.hpp"
#include "game/root.hpp"
#include "game/turn.hpp"
#include "game/spec/shiplist.hpp"

namespace game { namespace interface {

    class VcrSideFunction : public interpreter::IndexableValue {
     public:
        VcrSideFunction(size_t battleNumber,
                        Session& session,
                        afl::base::Ref<Root> root,     // for PlayerList
                        afl::base::Ref<Turn> turn,     // for Turn
                        afl::base::Ref<game::spec::ShipList> shipList);

        // IndexableValue:
        virtual VcrSideContext* get(interpreter::Arguments& args);
        virtual void set(interpreter::Arguments& args, afl::data::Value* value);

        // CallableValue:
        virtual int32_t getDimension(int32_t which) const;
        virtual VcrSideContext* makeFirstContext();
        virtual VcrSideFunction* clone() const;

        // BaseValue:
        virtual String_t toString(bool readable) const;
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, afl::charset::Charset& cs, interpreter::SaveContext& ctx) const;

        int32_t getNumObjects() const;

     private:
        const size_t m_battleNumber;
        Session& m_session;
        const afl::base::Ref<Root> m_root;
        const afl::base::Ref<Turn> m_turn;
        const afl::base::Ref<game::spec::ShipList> m_shipList;
    };

} }

#endif
