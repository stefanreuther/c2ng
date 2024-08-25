/**
  *  \file game/interface/torpedofunction.hpp
  *  \brief Class game::interface::TorpedoFunction
  */
#ifndef C2NG_GAME_INTERFACE_TORPEDOFUNCTION_HPP
#define C2NG_GAME_INTERFACE_TORPEDOFUNCTION_HPP

#include "game/session.hpp"
#include "interpreter/indexablevalue.hpp"

namespace game { namespace interface {

    /** Implementation of the "Launcher" and "Torpedo" functions. */
    class TorpedoFunction : public interpreter::IndexableValue {
     public:
        /** Constructor.
            @param useLauncher true to publish launcher properties, false for torpedo properties
            @param session     Session */
        TorpedoFunction(bool useLauncher, Session& session);

        // IndexableValue:
        virtual interpreter::Context* get(interpreter::Arguments& args);
        virtual void set(interpreter::Arguments& args, const afl::data::Value* value);

        // CallableValue:
        virtual size_t getDimension(size_t which) const;
        virtual interpreter::Context* makeFirstContext();
        virtual TorpedoFunction* clone() const;

        // BaseValue:
        virtual String_t toString(bool readable) const;
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const;

     private:
        const bool m_useLauncher;
        Session& m_session;
    };

} }

#endif
