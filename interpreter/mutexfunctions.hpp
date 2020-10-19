/**
  *  \file interpreter/mutexfunctions.hpp
  *  \brief Mutex Functions
  */
#ifndef C2NG_INTERPRETER_MUTEXFUNCTIONS_HPP
#define C2NG_INTERPRETER_MUTEXFUNCTIONS_HPP

namespace interpreter {

    class World;

    /** Register Mutex functions on the given World.
        This function is called by World and therefore needn't be called by a user.
        \param world World */
    void registerMutexFunctions(World& world);

}

#endif
