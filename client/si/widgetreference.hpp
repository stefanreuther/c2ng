/**
  *  \file client/si/widgetreference.hpp
  *  \brief Class client::si::WidgetReference
  */
#ifndef C2NG_CLIENT_SI_WIDGETREFERENCE_HPP
#define C2NG_CLIENT_SI_WIDGETREFERENCE_HPP

#include "afl/base/ref.hpp"
#include "afl/base/types.hpp"
#include "ui/widget.hpp"

namespace client { namespace si {

    class WidgetHolder;
    class Control;

    /** Reference to a widget created by/for scripts.
        Widgets accessible from scripts are represented as an index into a WidgetHolder.
        This is a convenience class to store a WidgetHolder/index pair. */
    class WidgetReference {
     public:
        /** Constructor.
            \param holder WidgetHolder
            \param slot   Slot into WidgetHolder (from WidgetHolder::addNewWidget). */
        WidgetReference(afl::base::Ref<WidgetHolder> holder, size_t slot);

        /** Copy constructor.
            \param other Reference to copy from */
        WidgetReference(const WidgetReference& other);

        /** Destructor. */
        ~WidgetReference();

        /** Get the contained widget.
            \param ctl Control to prove you're the correct thread.
            \return widget; null if the slot number is out-of-range or the widget was destroyed for some reason
            \see WidgetHolder::get() */
        ui::Widget* get(Control& ctl) const;

        /** Get contained WidgetHolder.
            \return WidgetHolder */
        WidgetHolder& getHolder() const;

        /** Get slot number.
            \return slot number */
        size_t getSlot() const;

        /** Make reference to related widget.
            After you have called getHolder().addNewWidget(),
            you can use this function to create a reference to that widget.
            \param peerSlot another index into this WidgetReference's WidgetHolder.
            \return reference to related widget */
        WidgetReference makePeer(size_t peerSlot) const;

     private:
        // Unassignable for now (although this is not a design limitation)
        const WidgetReference& operator=(const WidgetReference&);

        afl::base::Ref<WidgetHolder> m_holder;
        size_t m_slot;
    };

} }

#endif
