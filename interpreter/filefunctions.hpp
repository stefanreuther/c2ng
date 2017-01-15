/**
  *  \file interpreter/filefunctions.hpp
  *  \brief Interpreter: File I/O Related Stuff
  */
#ifndef C2NG_INTERPRETER_FILEFUNCTIONS_HPP
#define C2NG_INTERPRETER_FILEFUNCTIONS_HPP

namespace interpreter {

    class World;

    /** Register file-related functions on a World instance.
        This includes
        - special commands (e.g. Open)
        - regular commands (e.g. Close, Seek)
        - regular functions (e.g. FPos())
        - regular functions to implement special commands (CC$Open) and builtins (CC$Print)
        \param world World instance */
    void registerFileFunctions(World& world);

}

#endif
