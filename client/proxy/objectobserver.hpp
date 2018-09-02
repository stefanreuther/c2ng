/**
  *  \file client/proxy/objectobserver.hpp
  */
#ifndef C2NG_CLIENT_PROXY_OBJECTOBSERVER_HPP
#define C2NG_CLIENT_PROXY_OBJECTOBSERVER_HPP

namespace client { namespace proxy {

    class ObjectListener;

    class ObjectObserver {
     public:
        virtual ~ObjectObserver()
            { }

        virtual void addNewListener(ObjectListener* pListener) = 0;
    };

} }

#endif
