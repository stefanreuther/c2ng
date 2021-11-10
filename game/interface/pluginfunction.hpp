/**
  *  \file game/interface/pluginfunction.hpp
  */
#ifndef C2NG_GAME_INTERFACE_PLUGINFUNCTION_HPP
#define C2NG_GAME_INTERFACE_PLUGINFUNCTION_HPP

#include "interpreter/indexablevalue.hpp"
#include "game/session.hpp"
#include "game/interface/plugincontext.hpp"

namespace game { namespace interface {

    class PluginFunction : public interpreter::IndexableValue {
     public:
        PluginFunction(Session& session);

        // IndexableValue:
        virtual PluginContext* get(interpreter::Arguments& args);
        virtual void set(interpreter::Arguments& args, afl::data::Value* value);

        // CallableValue:
        virtual int32_t getDimension(int32_t which) const;
        virtual interpreter::Context* makeFirstContext();
        virtual PluginFunction* clone() const;

        // BaseValue:
        virtual String_t toString(bool readable) const;
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const;

     private:
        Session& m_session;
    };

} }

#endif
