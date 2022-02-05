/**
  *  \file game/browser/handlerlist.hpp
  *  \brief Class game::browser::HandlerList
  */
#ifndef C2NG_GAME_BROWSER_HANDLERLIST_HPP
#define C2NG_GAME_BROWSER_HANDLERLIST_HPP

#include "afl/container/ptrvector.hpp"
#include "game/browser/handler.hpp"
#include "game/task.hpp"

namespace game { namespace browser {

    /** List of handlers.
        Manages lifetime a list of Handler descendants, and implements the Handler interface on them.
        This produces a Handler that supports all storage types supported by the child Handler objects. */
    class HandlerList : public Handler {
     public:
        /** Constructor.
            Makes empty HandlerList. */
        HandlerList();

        /** Destructor. */
        ~HandlerList();

        /** Add new handler.
            @param h Newly-allocated Handler descendant; should not be 0. */
        void addNewHandler(Handler* h);

        // Handler methods:

        /** Handle folder name or URL.
            Invokes all child handlers' methods, until one returns true.

            @param name   [in]  Name or URL
            @param result [out] Result
            @retval true  Name was recognized by a Handler
            @retval false Name was not recognized by any Handler */
        virtual bool handleFolderName(String_t name, afl::container::PtrVector<Folder>& result);

        /** Create account folder.
            Invokes all child handlers' methods, until one returns non-null

            @param acc Account
            @return Newly-allocated folder; null if no handler can deal with this account */
        virtual Folder* createAccountFolder(Account& acc);

        /** Load game root for physical folder.
            Invokes all child handlers' methods, until one returns non-null

            @param dir    [in]     Directory
            @param config [in]     Configuration. Must live until the result callback completes.
            @param then   [in,out] Result callback (moved-away if responsible, otherwise unchanged)
            @return Non-null task if the directory was understood */
        virtual std::auto_ptr<Task_t> loadGameRootMaybe(afl::base::Ref<afl::io::Directory> dir, const game::config::UserConfiguration& config, std::auto_ptr<LoadGameRootTask_t>& then);

     private:
        afl::container::PtrVector<Handler> m_handlers;
    };

} }

#endif
