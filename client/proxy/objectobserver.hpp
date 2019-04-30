/**
  *  \file client/proxy/objectobserver.hpp
  *  \brief Interface client::proxy::ObjectObserver
  */
#ifndef C2NG_CLIENT_PROXY_OBJECTOBSERVER_HPP
#define C2NG_CLIENT_PROXY_OBJECTOBSERVER_HPP

namespace client { namespace proxy {

    class ObjectListener;

    /** Interface for a map object observer.
        This interface is used by widgets that display map object content (e,g. stuff in client::tiles).
        A user of these widgets needs to supply a ObjectObserver descendant that provides the actual observation.
        The ObjectObserver lives in the UI thread, but will have a game-thread counterpart. */
    class ObjectObserver {
     public:
        /** Virtual destructor. */
        virtual ~ObjectObserver()
            { }

        /** Add new ObjectListener instance.
            This function is called from the UI thread.
            The ObjectListener is however passed into the game thread and must therefore not reference UI data.
            It will typically contain a util::RequestSender to reply.

            \param pListener newly-allocated ObjectListener; becomes owned by ObjectObserver */
        virtual void addNewListener(ObjectListener* pListener) = 0;
    };

} }

#endif
