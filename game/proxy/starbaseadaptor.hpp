/**
  *  \file game/proxy/starbaseadaptor.hpp
  *  \brief Interface game::proxy::StarbaseAdaptor
  */
#ifndef C2NG_GAME_PROXY_STARBASEADAPTOR_HPP
#define C2NG_GAME_PROXY_STARBASEADAPTOR_HPP

#include "afl/base/deletable.hpp"
#include "game/cargocontainer.hpp"
#include "game/map/planet.hpp"
#include "game/session.hpp"

namespace game { namespace proxy {

    /** Adaptor to access a starbase, for related proxies.
        Allows the proxies to work on real or fake planets. */
    class StarbaseAdaptor : public afl::base::Deletable {
     public:
        /** Access the subject planet.
            @return planet */
        virtual game::map::Planet& planet() = 0;

        /** Access session.
            Caller will retrieve Root, ShipList, Game from it,
            but is not supposed to modify it.
            @return session */
        virtual Session& session() = 0;

        /** Find ship cloning at this planet.
            For a real planet, use game::map::Universe::findShipCloningAt(); for a fake planet, return false.

            @param [out]    id     Ship Id
            @param [out]    name   Ship name
            @retval true  Found a ship; id/name updated
            @retval false No cloning ship found */
        virtual bool findShipCloningHere(Id_t& id, String_t& name) = 0;

        /** Cancel all clone orders at this planet.
            For a real planet, use game::map::cancelAllCloneOrders(); for a fake planet, ignore. */
        virtual void cancelAllCloneOrders() = 0;

        /** Notify listeners.
            For a real planet, use Session::notifyListeners() to publish changes;
            for a fake planet, ignore. */
        virtual void notifyListeners() = 0;
    };

} }

#endif
