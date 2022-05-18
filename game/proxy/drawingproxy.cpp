/**
  *  \file game/proxy/drawingproxy.cpp
  *  \brief Class game::proxy::DrawingProxy
  */

#include <set>
#include "game/proxy/drawingproxy.hpp"
#include "afl/string/format.hpp"
#include "afl/string/parse.hpp"
#include "game/config/markeroption.hpp"
#include "game/game.hpp"
#include "game/map/drawing.hpp"
#include "game/map/drawingcontainer.hpp"
#include "game/root.hpp"
#include "game/turn.hpp"

using game::map::Drawing;
using game::map::DrawingContainer;

namespace {
    int limitCircleRadius(int r)
    {
        // ex WDrawCircleChartMode::setRadius (sort-of)
        return std::max(1, std::min(Drawing::MAX_CIRCLE_RADIUS, r));
    }
}


/*
 *  FIXME: consider: automatically lose focus when viewpoint turn changes
 *  FIXME: consider: refuse modifying non-current turn
 */
class game::proxy::DrawingProxy::Trampoline {
 public:
    Trampoline(Session& session, const util::RequestSender<DrawingProxy>& reply);

    void packStatus(Status_t& out) const;
    void packTagList(util::StringList& list) const;

    void create(game::map::Point pos, Drawing::Type type);
    void createCannedMarker(game::map::Point pos, int slot);
    void selectNearestVisibleDrawing(game::map::Point pos, double maxDistance);
    void selectMarkerAt(game::map::Point pos);
    void finish();
    void setPos(game::map::Point pos);
    void setPos2(game::map::Point pos);
    void changeCircleRadius(int delta);
    void setCircleRadius(int r);
    void continueLine();
    void setMarkerKind(int k);
    void setColor(uint8_t color, bool adjacent);
    void setTag(util::Atom_t tag, bool adjacent);
    void setTagName(String_t tagName, bool adjacent);
    void erase(bool adjacent);
    void setComment(String_t comment);

 private:
    Session& m_session;
    util::RequestSender<DrawingProxy> m_reply;

    DrawingContainer::Iterator_t m_current;
    Turn* m_currentTurn;

    afl::base::SignalConnection conn_drawingChange;

    void sendDrawingUpdate();
    void sendStatus();
    void flushRequests();
    void onDrawingChange();
    Turn* getTurn() const;
    Game* getGame() const;
    void setCurrentDrawing(DrawingContainer::Iterator_t it, Turn* t);
};

game::proxy::DrawingProxy::Trampoline::Trampoline(Session& session, const util::RequestSender<DrawingProxy>& reply)
    : m_session(session),
      m_reply(reply),
      m_current(),
      m_currentTurn(0),
      conn_drawingChange()
{ }

void
game::proxy::DrawingProxy::Trampoline::packStatus(Status_t& out) const
{
    if (const Drawing* p = *m_current) {
        out = *p;
    } else {
        out = afl::base::Nothing;
    }
}

void
game::proxy::DrawingProxy::Trampoline::packTagList(util::StringList& list) const
{
    list.clear();
    if (Turn* t = getTurn()) {
        // Get list of atoms
        std::set<util::Atom_t> usedAtoms;
        DrawingContainer& cont = t->universe().drawings();
        for (DrawingContainer::Iterator_t it = cont.begin(); it != cont.end(); ++it) {
            usedAtoms.insert((**it).getTag());
        }

        // Store atoms into list
        const util::AtomTable& tab = m_session.world().atomTable();
        for (std::set<util::Atom_t>::const_iterator i = usedAtoms.begin(); i != usedAtoms.end(); ++i) {
            String_t name = tab.getStringFromAtom(*i);
            if (name.empty()) {
                name = afl::string::Format("%d", *i);
            }
            list.add(static_cast<int32_t>(*i), name);
        }
    }
}

void
game::proxy::DrawingProxy::Trampoline::create(game::map::Point pos, Drawing::Type type)
{
    if (Turn* t = getTurn()) {
        setCurrentDrawing(t->universe().drawings().addNew(new Drawing(pos, type)), t);
        m_session.notifyListeners();
    }
}

void
game::proxy::DrawingProxy::Trampoline::createCannedMarker(game::map::Point pos, int slot)
{
    if (Turn* t = getTurn()) {
        if (Root* r = m_session.getRoot().get()) {
            // Obtain configuration
            const game::config::MarkerOptionDescriptor* opt = game::config::UserConfiguration::getCannedMarker(slot);
            if (opt != 0) {
                // Draw it
                std::auto_ptr<Drawing> drawing(new Drawing(pos, r->userConfiguration()[*opt]()));

                // Add it
                setCurrentDrawing(t->universe().drawings().addNew(drawing.release()), t);
                m_session.notifyListeners();
            }
        }
    }
}

void
game::proxy::DrawingProxy::Trampoline::selectNearestVisibleDrawing(game::map::Point pos, double maxDistance)
{
    if (Game* g = getGame()) {
        if (Turn* t = getTurn()) {
            DrawingContainer& cont = t->universe().drawings();
            DrawingContainer::Iterator_t it = cont.findNearestVisibleDrawing(pos, g->mapConfiguration(), maxDistance);
            if (it != cont.end()) {
                setCurrentDrawing(it, t);
            }
        }
    }
}

void
game::proxy::DrawingProxy::Trampoline::selectMarkerAt(game::map::Point pos)
{
    if (Game* g = getGame()) {
        if (Turn* t = getTurn()) {
            DrawingContainer& cont = t->universe().drawings();
            DrawingContainer::Iterator_t it = cont.findMarkerAt(pos, g->mapConfiguration());
            if (it != cont.end()) {
                setCurrentDrawing(it, t);
            }
        }
    }
}

void
game::proxy::DrawingProxy::Trampoline::finish()
{
    setCurrentDrawing(DrawingContainer::Iterator_t(), 0);
    m_session.notifyListeners();
}

void
game::proxy::DrawingProxy::Trampoline::setPos(game::map::Point pos)
{
    if (Drawing* d = *m_current) {
        d->setPos(pos);
        sendDrawingUpdate();
    }
}

void
game::proxy::DrawingProxy::Trampoline::setPos2(game::map::Point pos)
{
    if (Drawing* d = *m_current) {
        if (d->getType() == Drawing::LineDrawing || d->getType() == Drawing::RectangleDrawing) {
            d->setPos2(pos);
            sendDrawingUpdate();
        }
    }
}

void
game::proxy::DrawingProxy::Trampoline::changeCircleRadius(int delta)
{
    if (Drawing* d = *m_current) {
        if (d->getType() == Drawing::CircleDrawing) {
            d->setCircleRadius(limitCircleRadius(delta + d->getCircleRadius()));
            sendDrawingUpdate();
        }
    }
}

void
game::proxy::DrawingProxy::Trampoline::setCircleRadius(int r)
{
    if (Drawing* d = *m_current) {
        if (d->getType() == Drawing::CircleDrawing) {
            d->setCircleRadius(limitCircleRadius(r));
            sendDrawingUpdate();
            flushRequests();
        }
    }
}

void
game::proxy::DrawingProxy::Trampoline::continueLine()
{
    Drawing* d = *m_current;
    Turn* t = getTurn();
    if (d != 0 && t != 0 && d->getType() == Drawing::LineDrawing) {
        Drawing* nd = new Drawing(*d);
        nd->setPos(d->getPos2());
        nd->setPos2(d->getPos2());
        setCurrentDrawing(t->universe().drawings().addNew(nd), t);
        m_session.notifyListeners();
    }
}

void
game::proxy::DrawingProxy::Trampoline::setMarkerKind(int k)
{
    if (Drawing* d = *m_current) {
        if (d->getType() == Drawing::MarkerDrawing) {
            d->setMarkerKind(k);
            sendDrawingUpdate();
        }
    }
}

void
game::proxy::DrawingProxy::Trampoline::setColor(uint8_t color, bool adjacent)
{
    if (Drawing* d = *m_current) {
        d->setColor(color);
        if (adjacent && d->getType() == Drawing::LineDrawing) {
            if (m_currentTurn != 0) {
                if (Game* g = getGame()) {
                    m_currentTurn->universe().drawings().setAdjacentLinesColor(d->getPos(),  color, g->mapConfiguration());
                    m_currentTurn->universe().drawings().setAdjacentLinesColor(d->getPos2(), color, g->mapConfiguration());
                }
            }
        }
        sendDrawingUpdate();
    }
}

void
game::proxy::DrawingProxy::Trampoline::setTag(util::Atom_t tag, bool adjacent)
{
    if (Drawing* d = *m_current) {
        d->setTag(tag);
        if (adjacent && d->getType() == Drawing::LineDrawing) {
            if (m_currentTurn != 0) {
                if (Game* g = getGame()) {
                    m_currentTurn->universe().drawings().setAdjacentLinesTag(d->getPos(),  tag, g->mapConfiguration());
                    m_currentTurn->universe().drawings().setAdjacentLinesTag(d->getPos2(), tag, g->mapConfiguration());
                }
            }
        }
        sendDrawingUpdate();
    }
}

void
game::proxy::DrawingProxy::Trampoline::setTagName(String_t tagName, bool adjacent)
{
    // ex WCreateMarkerDialog::getResult (sort-of)
    util::Atom_t atom;
    if (!afl::string::strToInteger(tagName, atom)) {
        atom = m_session.world().atomTable().getAtomFromString(tagName);
    }
    setTag(atom, adjacent);
}

void
game::proxy::DrawingProxy::Trampoline::erase(bool adjacent)
{
    Drawing* d = *m_current;
    Game* g = getGame();
    if (d != 0 && m_currentTurn != 0 && g != 0) {
        game::map::DrawingContainer& drawings = m_currentTurn->universe().drawings();
        if (adjacent && d->getType() == Drawing::LineDrawing) {
            game::map::Point a = d->getPos(), b = d->getPos2();
            drawings.erase(m_current);
            drawings.eraseAdjacentLines(a, g->mapConfiguration());
            drawings.eraseAdjacentLines(b, g->mapConfiguration());
        } else {
            drawings.erase(m_current);
        }
        m_session.notifyListeners();
    }
}

void
game::proxy::DrawingProxy::Trampoline::setComment(String_t comment)
{
    if (Drawing* d = *m_current) {
        if (d->getType() == Drawing::MarkerDrawing) {
            d->setComment(comment);
            sendDrawingUpdate();
        }
    }
}

void
game::proxy::DrawingProxy::Trampoline::sendDrawingUpdate()
{
    // Update the drawing container.
    // This will indirectly call sendStatus() which will report back to the user.
    if (m_currentTurn != 0) {
        m_currentTurn->universe().drawings().sig_change.raise();
    }

    // Notify session to propagate further, e.g. to the map renderer.
    m_session.notifyListeners();
}

void
game::proxy::DrawingProxy::Trampoline::sendStatus()
{
    class Task : public util::Request<DrawingProxy> {
     public:
        Task(const Trampoline& tpl)
            { tpl.packStatus(m_status); }
        virtual void handle(DrawingProxy& proxy)
            { proxy.sig_update.raise(m_status); }
     private:
        Status_t m_status;
    };
    m_reply.postNewRequest(new Task(*this));
}

void
game::proxy::DrawingProxy::Trampoline::flushRequests()
{
    m_reply.postRequest(&DrawingProxy::flushRequests);
}

void
game::proxy::DrawingProxy::Trampoline::onDrawingChange()
{
    if (*m_current == 0) {
        // Our drawing got deleted. Go back to idle state.
        setCurrentDrawing(DrawingContainer::Iterator_t(), 0);
    } else {
        // Our drawing might have changed. Update user.
        sendStatus();
    }
}

game::Turn*
game::proxy::DrawingProxy::Trampoline::getTurn() const
{
    if (Game* g = m_session.getGame().get()) {
        return g->getViewpointTurn().get();
    } else {
        return 0;
    }
}

game::Game*
game::proxy::DrawingProxy::Trampoline::getGame() const
{
    return m_session.getGame().get();
}

void
game::proxy::DrawingProxy::Trampoline::setCurrentDrawing(DrawingContainer::Iterator_t it, Turn* t)
{
    // Disconnect old
    conn_drawingChange.disconnect();

    // If previous drawing is invisible, remove it
    // ex WDrawMarkerChartMode::finishDrawing
    if (Drawing* prev = *m_current) {
        if (m_currentTurn != 0
            && (prev->getType() == Drawing::LineDrawing || prev->getType() == Drawing::RectangleDrawing)
            && prev->getPos() == prev->getPos2())
        {
            m_currentTurn->universe().drawings().erase(m_current);
        }
    }

    // Reconnect
    if (t != 0 && *it != 0) {
        m_current = it;
        m_currentTurn = t;
        conn_drawingChange = t->universe().drawings().sig_change.add(this, &Trampoline::onDrawingChange);
    } else {
        m_current = DrawingContainer::Iterator_t();
        m_currentTurn = 0;
    }

    // Update user
    sendStatus();
}



/*
 *  TrampolineFromSession
 */

class game::proxy::DrawingProxy::TrampolineFromSession : public afl::base::Closure<Trampoline*(game::Session&)> {
 public:
    TrampolineFromSession(const util::RequestSender<DrawingProxy>& reply)
        : m_reply(reply)
        { }
    virtual Trampoline* call(game::Session& session)
        { return new Trampoline(session, m_reply); }
 private:
    util::RequestSender<DrawingProxy> m_reply;
};



game::proxy::DrawingProxy::DrawingProxy(util::RequestSender<Session> gameSender, util::RequestDispatcher& reply)
    : m_reply(reply, *this),
      m_request(gameSender.makeTemporary(new TrampolineFromSession(m_reply.getSender()))),
      m_activeRequest(None),
      m_circleRadius()
{ }

game::proxy::DrawingProxy::~DrawingProxy()
{ }

void
game::proxy::DrawingProxy::getStatus(WaitIndicator& ind, Status_t& status)
{
    class Task : public util::Request<Trampoline> {
     public:
        Task(Status_t& status)
            : m_status(status)
            { }
        virtual void handle(Trampoline& tpl)
            { tpl.packStatus(m_status); }
     private:
        Status_t& m_status;
    };
    flushRequests();
    Task t(status);
    ind.call(m_request, t);
}

void
game::proxy::DrawingProxy::getTagList(WaitIndicator& ind, util::StringList& list)
{
    class Task : public util::Request<Trampoline> {
     public:
        Task(util::StringList& list)
            : m_list(list)
            { }
        virtual void handle(Trampoline& tpl)
            { tpl.packTagList(m_list); }
     private:
        util::StringList& m_list;
    };
    flushRequests();
    Task t(list);
    ind.call(m_request, t);
}

void
game::proxy::DrawingProxy::create(game::map::Point pos, game::map::Drawing::Type type)
{
    flushRequests();
    m_request.postRequest(&Trampoline::create, pos, type);
}

void
game::proxy::DrawingProxy::createCannedMarker(game::map::Point pos, int slot)
{
    flushRequests();
    m_request.postRequest(&Trampoline::createCannedMarker, pos, slot);
}

void
game::proxy::DrawingProxy::selectNearestVisibleDrawing(game::map::Point pos, double maxDistance)
{
    flushRequests();
    m_request.postRequest(&Trampoline::selectNearestVisibleDrawing, pos, maxDistance);
}

void
game::proxy::DrawingProxy::selectMarkerAt(game::map::Point pos)
{
    flushRequests();
    m_request.postRequest(&Trampoline::selectMarkerAt, pos);
}

void
game::proxy::DrawingProxy::finish()
{
    flushRequests();
    m_request.postRequest(&Trampoline::finish);
}

void
game::proxy::DrawingProxy::setPos(game::map::Point pos)
{
    flushRequests();
    m_request.postRequest(&Trampoline::setPos, pos);
}

void
game::proxy::DrawingProxy::setPos2(game::map::Point pos)
{
    flushRequests();
    m_request.postRequest(&Trampoline::setPos2, pos);
}

void
game::proxy::DrawingProxy::changeCircleRadius(int delta)
{
    flushRequests();
    m_request.postRequest(&Trampoline::changeCircleRadius, delta);
}

void
game::proxy::DrawingProxy::setCircleRadius(int r)
{
    if (checkRequest(CircleRadius)) {
        m_request.postRequest(&Trampoline::setCircleRadius, r);
    } else {
        m_circleRadius = r;
    }
}

void
game::proxy::DrawingProxy::continueLine()
{
    flushRequests();
    m_request.postRequest(&Trampoline::continueLine);
}

void
game::proxy::DrawingProxy::setMarkerKind(int k)
{
    flushRequests();
    m_request.postRequest(&Trampoline::setMarkerKind, k);
}

void
game::proxy::DrawingProxy::setColor(uint8_t c, bool adjacent)
{
    flushRequests();
    m_request.postRequest(&Trampoline::setColor, c, adjacent);
}

void
game::proxy::DrawingProxy::setTag(util::Atom_t tag, bool adjacent)
{
    flushRequests();
    m_request.postRequest(&Trampoline::setTag, tag, adjacent);
}

void
game::proxy::DrawingProxy::setTagName(String_t tag, bool adjacent)
{
    flushRequests();
    m_request.postRequest(&Trampoline::setTagName, tag, adjacent);
}

void
game::proxy::DrawingProxy::erase(bool adjacent)
{
    flushRequests();
    m_request.postRequest(&Trampoline::erase, adjacent);
}

void
game::proxy::DrawingProxy::setComment(String_t comment)
{
    flushRequests();
    m_request.postRequest(&Trampoline::setComment, comment);
}

bool
game::proxy::DrawingProxy::checkRequest(Request newRequest)
{
    if (m_activeRequest == newRequest) {
        // Same request already active; queue it.
        return false;
    } else {
        // Different or no request. Flush and process it.
        flushRequests();
        m_activeRequest = newRequest;
        return true;
    }
}

void
game::proxy::DrawingProxy::flushRequests()
{
    switch (m_activeRequest) {
     case None:
        break;
     case CircleRadius:
        if (const int* p = m_circleRadius.get()) {
            m_request.postRequest(&Trampoline::setCircleRadius, *p);
        }
        m_circleRadius.clear();
        break;
    }
    m_activeRequest = None;
}
