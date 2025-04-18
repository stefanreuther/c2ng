/**
  *  \file client/widgets/expressionlist.hpp
  *  \brief Expression List Popup
  */
#ifndef C2NG_CLIENT_WIDGETS_EXPRESSIONLIST_HPP
#define C2NG_CLIENT_WIDGETS_EXPRESSIONLIST_HPP

#include "afl/string/string.hpp"
#include "game/proxy/expressionlistproxy.hpp"
#include "ui/root.hpp"

namespace client { namespace widgets {

    /** Display expression list popup.

        \param [in]     root    UI root
        \param [in]     ind     WaitIndicator
        \param [in]     proxy   ExpressionListProxy
        \param [in]     anchor  Anchor point for popup
        \param [in,out] value   Current value
        \param [out]    flags   New flags

        \retval true User chose an item; value/flags have been set
        \retval false User canceled */
    bool doExpressionListPopup(ui::Root& root,
                               game::proxy::WaitIndicator& ind,
                               game::proxy::ExpressionListProxy& proxy,
                               gfx::Point anchor,
                               String_t& value,
                               String_t& flags);

} }

#endif
