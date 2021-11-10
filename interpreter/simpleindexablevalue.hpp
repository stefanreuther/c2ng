/**
  *  \file interpreter/simpleindexablevalue.hpp
  */
#ifndef C2NG_INTERPRETER_SIMPLEINDEXABLEVALUE_HPP
#define C2NG_INTERPRETER_SIMPLEINDEXABLEVALUE_HPP

#include "interpreter/indexablevalue.hpp"

namespace interpreter {

    class World;

    class SimpleIndexableValue : public IndexableValue {
     public:
        typedef afl::data::Value* Get_t(World& world, Arguments& args);
        typedef int32_t Dim_t(World& world, int32_t which);
        typedef Context* Make_t(World& world);

        SimpleIndexableValue(World& world, Get_t* get, Dim_t* dim, Make_t* make);
        ~SimpleIndexableValue();

        // IndexableValue:
        virtual afl::data::Value* get(Arguments& args);
        virtual void set(Arguments& args, afl::data::Value* value);
        virtual int32_t getDimension(int32_t which) const;

        // CallableValue:
        virtual Context* makeFirstContext();
        virtual SimpleIndexableValue* clone() const;

        // BaseValue:
        virtual String_t toString(bool readable) const;
        virtual void store(TagNode& out, afl::io::DataSink& aux, SaveContext& ctx) const;

     private:
        World& m_world;
        Get_t* m_get;
        Dim_t* m_dim;
        Make_t* m_make;
    };

}

#endif
