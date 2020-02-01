/**
  *  \file client/si/compoundwidget.hpp
  *  \brief Template Class client::si::CompoundWidget
  */
#ifndef C2NG_CLIENT_SI_COMPOUNDWIDGET_HPP
#define C2NG_CLIENT_SI_COMPOUNDWIDGET_HPP

#include "ui/group.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/widget.hpp"

namespace client { namespace si {

    /** Compound widget.
        Sometimes we have a function widget (i.e. DecimalSelector, Listbox)
        that is used together with a few auxiliary widgets (buttons, scrollbars, frames).
        Whereas function calls ("SetValue") access the function widget,
        we need the entire assembly for purposes of UI layout etc.
        We only have a single widget reference available for use as a WidgetValue.

        In this case, create a CompoundWidget, passing it the inner and outer references.
        The CompoundWidget will behave as the outer widget,
        but will let you access the inner one.

        This wraps an additional ui::Group around the outer widget which is
        superfluous but harmless and simplifies things.

        \tparam InnerWidget Inner widget type */
    template<typename InnerWidget>
    class CompoundWidget : public ui::Group {
     public:
        /** Constructor.
            \param innerWidget Inner (functional) widget
            \param outerWidget Outer widget

            Widgets need to be allocated in the same WidgetHolder that will also hold the CompoundWidget. */
        CompoundWidget(InnerWidget& innerWidget, ui::Widget& outerWidget)
            : Group(ui::layout::HBox::instance0),
              m_innerWidget(innerWidget)
            { add(outerWidget); }

        /** Access inner widget. */
        InnerWidget& widget()
            { return m_innerWidget; }

     private:
        InnerWidget& m_innerWidget;
    };

} }

#endif
