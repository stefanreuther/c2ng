/**
  *  \file game/browser/synchronousfolder.hpp
  *  \brief Class game::browser::SynchronousFolder
  */
#ifndef C2NG_GAME_BROWSER_SYNCHRONOUSFOLDER_HPP
#define C2NG_GAME_BROWSER_SYNCHRONOUSFOLDER_HPP

#include "game/browser/folder.hpp"

namespace game { namespace browser {

    /** Folder with synchronous `loadContent` method.
        This overrides the asynchronous `loadContent` method with a version that implements a simpler synchronous interface.
        Use whenever your Folder implementation will never call out to the user.
        (This applies to most implementations that do not initiate a network connection.) */
    class SynchronousFolder : public Folder {
     public:
        /** Load content of this folder, synchronous version.
            Produces a list of new folders.
            If canEnter() returns true, this function can still be called but should return an empty (unmodified) list.

            This function can not defer a task.

            This function shall not throw exceptions.

            \param result [out] New folders appended here */
        virtual void loadContent(afl::container::PtrVector<Folder>& result) = 0;

        // Implementation of interface method:
        virtual std::auto_ptr<Task_t> loadContent(std::auto_ptr<LoadContentTask_t> then);
    };

} }

#endif
