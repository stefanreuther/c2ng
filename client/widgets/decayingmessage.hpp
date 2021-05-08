/**
  *  \file client/widgets/decayingmessage.hpp
  *  \brief Decaying Message widget
  */
#ifndef C2NG_CLIENT_WIDGETS_DECAYINGMESSAGE_HPP
#define C2NG_CLIENT_WIDGETS_DECAYINGMESSAGE_HPP

#include "ui/root.hpp"
#include "afl/string/string.hpp"

namespace client { namespace widgets {

    void showDecayingMessage(ui::Root& root, String_t text);

} }

#endif
