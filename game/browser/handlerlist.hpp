/**
  *  \file game/browser/handlerlist.hpp
  *  \brief Class game::browser::HandlerList
  */
#ifndef C2NG_GAME_BROWSER_HANDLERLIST_HPP
#define C2NG_GAME_BROWSER_HANDLERLIST_HPP

#include "afl/container/ptrvector.hpp"
#include "game/browser/handler.hpp"

namespace game { namespace browser {

    /** List of handlers.
        Manages lifetime a list of Handler descendants, and implements the Handler interface on them. */
    class HandlerList : public Handler {
     public:
        /** Constructor.
            Makes empty HandlerList. */
        HandlerList();

        /** Destructor. */
        ~HandlerList();

        /** Add new handler.
            \param h Newly-allocated Handler descendant; should not be 0. */
        void addNewHandler(Handler* h);

        // Handler methods:
        virtual bool handleFolderName(String_t name, afl::container::PtrVector<Folder>& result);
        virtual Folder* createAccountFolder(Account& acc);
        virtual afl::base::Ptr<Root> loadGameRoot(afl::base::Ref<afl::io::Directory> dir);

     private:
        afl::container::PtrVector<Handler> m_handlers;
    };

} }

#endif
