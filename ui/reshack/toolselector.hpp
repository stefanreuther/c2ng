/**
  *  \file ui/reshack/toolselector.hpp
  *  \brief Class ui::reshack::ToolSelector
  */
#ifndef C2NG_UI_RESHACK_TOOLSELECTOR_HPP
#define C2NG_UI_RESHACK_TOOLSELECTOR_HPP

#include <vector>
#include "ui/reshack/modeselector.hpp"
#include "afl/container/ptrvector.hpp"

namespace ui { namespace reshack {

    class Painter;
    class Tool;

    /** Tool selector widget.
        @see Painter::setTool */
    class ToolSelector : public ModeSelector {
     public:
        /** Constructor.
            Call addNewTool() to add tools.
            @param root UI root
            @param p    Painter whose tool to control */
        ToolSelector(Root& root, Painter& p);
        ~ToolSelector();

        /** Add a new tool.
            @param key Key
            @param t   Newly-allocated tool. ToolSelector will assume ownership. */
        void addNewTool(util::Key_t key, Tool* t);

     protected:
        virtual bool isActive(size_t slot) const;
        virtual bool isUsable(size_t slot) const;
        virtual void activate(size_t slot);
        virtual String_t getName(size_t slot) const;
        virtual size_t getNumSlots() const;
        virtual size_t getSlotFromKey(util::Key_t k) const;

     private:
        afl::container::PtrVector<Tool> m_tools;
        std::vector<util::Key_t> m_keys;
        Painter& m_painter;
    };

} }

#endif
