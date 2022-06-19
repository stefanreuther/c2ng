/**
  *  \file interpreter/mutexfunctions.hpp
  *  \brief Mutex Functions
  */
#ifndef C2NG_INTERPRETER_MUTEXFUNCTIONS_HPP
#define C2NG_INTERPRETER_MUTEXFUNCTIONS_HPP

namespace interpreter {

    class World;
    class BytecodeObject;

    /** Register Mutex functions on the given World.
        This function is called by World and therefore needn't be called by a user.
        \param world World */
    void registerMutexFunctions(World& world);

    /** Create mutex dummy functions on the given BytecodeObject.
        Defines the functions as local variables in the given object.
        These functions fulfill the same interface as the real functions,
        but do not actually check or take locks.

        This is used for implementing the "override locks" functionality for global actions.
        \param bco BytecodeObject */
    void registerDummyMutexFunctions(BytecodeObject& bco);

}

#endif
