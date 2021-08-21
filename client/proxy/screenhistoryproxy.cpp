/**
  *  \file client/proxy/screenhistoryproxy.cpp
  */

#include "client/proxy/screenhistoryproxy.hpp"
#include "util/request.hpp"
#include "game/turn.hpp"
#include "game/game.hpp"
#include "afl/string/format.hpp"

using client::ScreenHistory;
using game::Reference;
using game::PlainName;
using game::LongName;
using afl::string::Format;

namespace {
    bool validate(game::Session& session, ScreenHistory::Reference ref)
    {
        // ex WScreenId::isValid, scrhist.pas:VerifyScreenHist (sort-of)
        game::Game* pGame = session.getGame().get();
        if (pGame == 0) {
            return false;
        }
        game::Turn* pTurn = pGame->getViewpointTurn().get();
        if (pTurn == 0) {
            return false;
        }
        switch (ref.getType()) {
         case ScreenHistory::Null:
            return false;

         case ScreenHistory::Ship:
         case ScreenHistory::ShipTask:
            return pTurn->universe().playedShips().getObjectByIndex(ref.getX()) != 0;

         case ScreenHistory::Planet:
         case ScreenHistory::PlanetTask:
            return pTurn->universe().playedPlanets().getObjectByIndex(ref.getX()) != 0;

         case ScreenHistory::Starbase:
         case ScreenHistory::StarbaseTask:
            return pTurn->universe().playedBases().getObjectByIndex(ref.getX()) != 0;

         case ScreenHistory::Starchart:
            return true;
            //  case RaceScreen:
            //     return true;
            //  case HistoryScreen:
            //     return current_histship_type.isValidIndex(x);

            //  case FleetScreen:
            //     return current_fleet_type.isValidIndex(x);
        }
        return false;
    }

    String_t getName(game::Session& session, ScreenHistory::Reference ref)
    {
        // ex WScreenId::toString
        String_t tmp;
        afl::string::Translator& tx = session.translator();
        switch (ref.getType()) {
         case ScreenHistory::Null:
            break;

         case ScreenHistory::Ship:
            if (session.getReferenceName(Reference(Reference::Ship, ref.getX()), LongName, tmp)) {
                return tmp;
            }
            break;

         case ScreenHistory::Planet:
            if (session.getReferenceName(Reference(Reference::Planet, ref.getX()), LongName, tmp)) {
                return tmp;
            }
            break;

         case ScreenHistory::Starbase:
            if (session.getReferenceName(Reference(Reference::Planet, ref.getX()), PlainName, tmp)) {
                return Format(tx("Starbase #%d: %s"), ref.getX(), tmp);
            }
            break;

         case ScreenHistory::ShipTask:
            if (session.getReferenceName(Reference(Reference::Ship, ref.getX()), PlainName, tmp)) {
                return Format(tx("Ship Task #%d: %s"), ref.getX(), tmp);
            }
            break;

         case ScreenHistory::PlanetTask:
            if (session.getReferenceName(Reference(Reference::Planet, ref.getX()), PlainName, tmp)) {
                return Format(tx("Planet Task #%d: %s"), ref.getX(), tmp);
            }
            break;

         case ScreenHistory::StarbaseTask:
            if (session.getReferenceName(Reference(Reference::Planet, ref.getX()), PlainName, tmp)) {
                return Format(tx("Starbase Task #%d: %s"), ref.getX(), tmp);
            }
            break;

         case ScreenHistory::Starchart:
            return Format(tx("Starchart (%d,%d)"), ref.getX(), ref.getY());
     // case RaceScreen:
     //    return tx("Race Screen");
     // case HistoryScreen:
     //    return Format(tx("Ship History %d: %s"), x, getDisplayedTurn().getCurrentUniverse().getShip(x).getName(GObject::PlainName));
     // case FleetScreen:
     //    return getFleetTitle(getDisplayedTurn().getCurrentUniverse(), x);
        }
        return String_t();
    }

    bool setCursor(game::map::ObjectCursor& cursor, game::Id_t id)
    {
        // ex client/screens.cc:trySetIndex (sort-of)
        // FIXME: make this a method of ObjectCursor?
        game::map::ObjectType* ty = cursor.getObjectType();
        if (ty != 0 && ty->getObjectByIndex(id) != 0) {
            cursor.setCurrentIndex(id);
            return true;
        } else {
            return false;
        }
    }

    bool activate(game::Session& session, ScreenHistory::Reference ref)
    {
        // ex scrhist.pas:GotoHistoryScreen
        game::Game* pGame = session.getGame().get();
        if (pGame == 0) {
            return false;
        }
        game::Turn* pTurn = pGame->getViewpointTurn().get();
        if (pTurn == 0) {
            return false;
        }
        switch (ref.getType()) {
         case ScreenHistory::Null:
            return false;

         case ScreenHistory::Ship:
         case ScreenHistory::ShipTask:
            return setCursor(pGame->cursors().currentShip(), ref.getX());

         case ScreenHistory::Planet:
         case ScreenHistory::PlanetTask:
            return setCursor(pGame->cursors().currentPlanet(), ref.getX());

         case ScreenHistory::Starbase:
         case ScreenHistory::StarbaseTask:
            return setCursor(pGame->cursors().currentBase(), ref.getX());

         case ScreenHistory::Starchart:
            pGame->cursors().location().set(game::map::Point(ref.getX(), ref.getY()));
            return true;
        }
        return false;
    }
}


client::proxy::ScreenHistoryProxy::ScreenHistoryProxy(util::RequestSender<game::Session> gameSender)
    : m_gameSender(gameSender)
{ }

bool
client::proxy::ScreenHistoryProxy::validateReference(Downlink& link, ScreenHistory::Reference ref)
{
    const ScreenHistory::Reference refs[] = {ref};
    afl::base::GrowableMemory<bool> results;
    validateReferences(link, refs, results);

    const bool* p = results.at(0);
    return p && *p;
}

void
client::proxy::ScreenHistoryProxy::validateReferences(Downlink& link, afl::base::Memory<const ScreenHistory::Reference> refs, afl::base::GrowableMemory<bool>& result)
{
    // ex client/history.cc:discardBogusEntries, sort-of
    class Validator : public util::Request<game::Session> {
     public:
        Validator(afl::base::Memory<const ScreenHistory::Reference> refs, afl::base::GrowableMemory<bool>& result)
            : m_refs(refs), m_result(result)
            { }

        ~Validator()
            { }

        virtual void handle(game::Session& session)
            {
                while (const ScreenHistory::Reference* pRef = m_refs.eat()) {
                    m_result.append(validate(session, *pRef));
                }
            }

     private:
        afl::base::Memory<const ScreenHistory::Reference> m_refs;
        afl::base::GrowableMemory<bool>& m_result;
    };

    Validator v(refs, result);
    link.call(m_gameSender, v);
}

String_t
client::proxy::ScreenHistoryProxy::getReferenceName(Downlink& link, ScreenHistory::Reference ref)
{
    const ScreenHistory::Reference refs[] = {ref};
    afl::base::GrowableMemory<String_t> results;
    getReferenceNames(link, refs, results);

    const String_t* p = results.at(0);
    return p ? *p : String_t();
}

void
client::proxy::ScreenHistoryProxy::getReferenceNames(Downlink& link, afl::base::Memory<const ScreenHistory::Reference> refs, afl::base::GrowableMemory<String_t>& result)
{
    class Task : public util::Request<game::Session> {
     public:
        Task(afl::base::Memory<const ScreenHistory::Reference> refs, afl::base::GrowableMemory<String_t>& result)
            : m_refs(refs), m_result(result)
            { }

        ~Task()
            { }

        virtual void handle(game::Session& session)
            {
                while (const ScreenHistory::Reference* pRef = m_refs.eat()) {
                    m_result.append(getName(session, *pRef));
                }
            }

     private:
        afl::base::Memory<const ScreenHistory::Reference> m_refs;
        afl::base::GrowableMemory<String_t>& m_result;
    };

    Task t(refs, result);
    link.call(m_gameSender, t);
}

bool
client::proxy::ScreenHistoryProxy::activateReference(Downlink& link, ScreenHistory::Reference ref)
{
    class Task : public util::Request<game::Session> {
     public:
        Task(ScreenHistory::Reference ref)
            : m_ref(ref), m_result(false)
            { }
        virtual void handle(game::Session& session)
            { m_result = activate(session, m_ref); }

        bool getResult() const
            { return m_result; }
        
     private:
        ScreenHistory::Reference m_ref;
        bool m_result;
    };

    Task t(ref);
    link.call(m_gameSender, t);
    return t.getResult();
}
