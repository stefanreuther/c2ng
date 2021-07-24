/**
  *  \file gfx/keyeventconsumer.hpp
  *  \brief Interface gfx::KeyEventConsumer
  */
#ifndef C2NG_GFX_KEYEVENTCONSUMER_HPP
#define C2NG_GFX_KEYEVENTCONSUMER_HPP

#include "util/key.hpp"
#include "afl/base/deletable.hpp"

namespace gfx {

    class KeyEventConsumer : public afl::base::Deletable {
     public:
        /** Handle keypress.
            \param key Key that was pressed
            \param prefix Prefix (repeat) count */
        virtual bool handleKey(util::Key_t key, int prefix) = 0;
    };

}

#endif
