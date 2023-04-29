/**
  *  \file interpreter/directoryfunctions.hpp
  *  \brief Interpreter: File System Directory Access
  */
#ifndef C2NG_INTERPRETER_DIRECTORYFUNCTIONS_HPP
#define C2NG_INTERPRETER_DIRECTORYFUNCTIONS_HPP

namespace interpreter {

    class World;

    /** Register directory-related functions on a World instance.
        For now, this is the DirectoryEntry() function
        \param world World instance */
    void registerDirectoryFunctions(World& world);

}

#endif
