/**
  *  \file game/browser/types.hpp
  *  \brief Browser types
  */
#ifndef C2NG_GAME_BROWSER_TYPES_HPP
#define C2NG_GAME_BROWSER_TYPES_HPP

#include "afl/container/ptrvector.hpp"
#include "afl/base/ptr.hpp"
#include "game/root.hpp"

namespace game { namespace browser {

    class Folder;

    /** Browser task type.
        Generic type for a pending task.
        Must not throw. */
    typedef afl::base::Closure<void()> Task_t;

    /** Task: result of loadContent().
        Must not throw. */
    typedef afl::base::Closure<void(afl::container::PtrVector<Folder>&)> LoadContentTask_t;

    /** Task: result of loadGameRoot().
        Must not throw. */
    typedef afl::base::Closure<void(afl::base::Ptr<Root>)> LoadGameRootTask_t;

} }

#endif
