/**
  *  \file game/browser/types.hpp
  *  \brief Browser types
  */
#ifndef C2NG_GAME_BROWSER_TYPES_HPP
#define C2NG_GAME_BROWSER_TYPES_HPP

#include "afl/container/ptrvector.hpp"
#include "afl/base/ptr.hpp"
#include "game/root.hpp"
#include "game/task.hpp"

namespace game { namespace browser {

    class Folder;

    /** Task: result of loadContent().
        Caller must provide the PtrVector with results.
        Callee can consume it (e.g. move it to a durable place).
        Must not throw. */
    typedef afl::base::Closure<void(afl::container::PtrVector<Folder>&)> LoadContentTask_t;

    /** Task: result of loadGameRoot().
        Must not throw. */
    typedef afl::base::Closure<void(afl::base::Ptr<Root>)> LoadGameRootTask_t;

} }

#endif
