/**
  *  \file game/proxy/drawingproxy.hpp
  *  \brief Class game::proxy::DrawingProxy
  */
#ifndef C2NG_GAME_PROXY_DRAWINGPROXY_HPP
#define C2NG_GAME_PROXY_DRAWINGPROXY_HPP

#include "afl/base/optional.hpp"
#include "afl/base/signal.hpp"
#include "game/map/drawing.hpp"
#include "game/map/point.hpp"
#include "game/proxy/waitindicator.hpp"
#include "game/session.hpp"
#include "util/requestdispatcher.hpp"
#include "util/requestsender.hpp"
#include "util/stringlist.hpp"

namespace game { namespace proxy {

    /** Bidirectional, asynchronous proxy for modifying drawings.

        This proxy provides the notion of a <em>current drawing</em>.
        You can create (create())/select (selectNearestVisibleDrawing()/deselect (finish()) a current drawing using asynchronous operations.
        You can modify it using asynchronous operations.
        Changes through this proxy or another component are reported using the sig_update signal;
        deletion of the current drawing causes it to automatically become unset.

        In addition to receiving asynchronous updates, you can synchronously query the current status (getStatus()).

        All changes are reflected to the universe immediately.

        If a change causes a LineDrawing or RectangleDrawing to become zero-size when you create/select a new drawing or finish this one, it is deleted.
        This cleanup does not happen when the DrawingProxy is just destroyed.
        You should therefore call finish() if possible. */
    class DrawingProxy {
     public:
        /** Current drawing status.
            Unset when there is no current drawing.
            Otherwise, contains a copy of the current drawing. */
        typedef afl::base::Optional<game::map::Drawing> Status_t;

        /** Constructor.
            \param gameSender Game sender
            \param reply      RequestDispatcher to receive replies on */
        DrawingProxy(util::RequestSender<Session> gameSender, util::RequestDispatcher& reply);

        /** Destructor. */
        ~DrawingProxy();

        /** Get status, synchronously.
            \param [in]  ind     WaitIndicator for UI synchronisation
            \param [out] status  Result */
        void getStatus(WaitIndicator& ind, Status_t& status);

        /** Get list of used tags with names.
            \param [in]  ind  WaitIndicator for UI synchronisation
            \param [out] list Result list, containing tag/value pairs */
        void getTagList(WaitIndicator& ind, util::StringList& list);

        /** Create a new drawing.
            The new drawing is selected as new current drawing.

            Note that color, radius, kind are set to defaults; use other functions to set those.

            \param pos  Initial position
            \param type Type
            \see game::map::Drawing::Drawing */
        void create(game::map::Point pos, game::map::Drawing::Type type);

        void createCannedMarker(game::map::Point pos, int slot);

        /** Select nearest drawing.
            If a matching drawing is found, it is selected as new current drawing.
            If no matching drawing is found, no change to the current drawing happens.
            \param pos         Position
            \param maxDistance Maximum distance
            \see game::map::DrawingContainer::findNearestVisibleDrawing */
        void selectNearestVisibleDrawing(game::map::Point pos, double maxDistance);

        /** Select marker at a given position.
            If a matching marker is found, it is selected as new current drawing.
            If no matching marker is found, no change to the current drawing happens.
            \param pos Position
            \see game::map::DrawingContainer::findMarkerAt */
        void selectMarkerAt(game::map::Point pos);

        /** Finish working with the current drawing.
            Deselects the current drawing. */
        void finish();

        /** Set position.
            \param pos Position (starting point for line, rectangle; center for circle or marker)
            \see game::map::Drawing::setPos */
        void setPos(game::map::Point pos);

        /** Set other position.
            Valid for LineDrawing, RectangleDrawing, ignored for others.
            \param pos Other position (ending point)
            \see game::map::Drawing::setPos2 */
        void setPos2(game::map::Point pos);

        /** Change circle radius.
            Valid for CircleDrawing, ignored for others.
            \param delta Value to add to circle's radius */
        void changeCircleRadius(int delta);

        /** Set circle radius.
            Valid for CircleDrawing, ignored for others.
            \param r New radius
            \see game::map::Drawing::setCircleRadius */
        void setCircleRadius(int r);

        /** Continue a line.
            Finishes the current line and starts a new one with the same attributes where this one left off;
            selects the new drawing.
            Valid for LineDrawing, ignored for others. */
        void continueLine();

        /** Set marker kind (shape).
            Valid for MarkerDrawing, ignored for others.
            \param k Kind [0,NUM_USER_MARKERS)
            \see game::map::Drawing::setMarkerKind */
        void setMarkerKind(int k);

        /** Set drawing color.
            \param c Color
            \param adjacent true to set color for adjacent lines as well
            \see game::map::Drawing::setColor, game::map::DrawingContainer::setAdjacentLinesColor*/
        void setColor(uint8_t c, bool adjacent);

        /** Set drawing tag.
            \param tag Tag
            \param adjacent true to set tag for adjacent lines as well
            \see game::map::Drawing::setTag, game::map::DrawingContainer::setAdjacentLinesTag*/
        void setTag(util::Atom_t tag, bool adjacent);

        /** Set drawing tag, by name.
            \param tag Tag name. Will be parsed as number or converted to atom.
            \param adjacent true to set tag for adjacent lines as well
            \see game::map::Drawing::setTag, game::map::DrawingContainer::setAdjacentLinesTag*/
        void setTagName(String_t tag, bool adjacent);

        /** Erase current drawing.
            The proxy has no selected drawing afterwards.
            \param adjacent true to erase adjacent drawings as well
            \see game::map::DrawingContainer::erase, game::map::DrawingContainer::eraseAdjacentLines */
        void erase(bool adjacent);

        /** Set comment.
            Valid for MarkerDrawing, ignored for others.
            \param comment New comment
            \see game::map::Drawing::setComment */
        void setComment(String_t comment);

        /** Signal: current drawing changes.
            Emitted whenever the current drawing changes, either from a function call by this proxy,
            or by an external modification.
            \param status Copy of current drawing, if any. */
        afl::base::Signal<void(const Status_t&)> sig_update;

     private:
        class Trampoline;
        class TrampolineFromSession;

        util::RequestReceiver<DrawingProxy> m_reply;
        util::RequestSender<Trampoline> m_request;

        /* To reduce the amount of lag that can build up, we collect a sequence of operations of the same type,
           and give it to the game side only when that reports completion of the previous operation.
           Currently, this is implemented for setCircleRadius() which users can generate using Alt+Mouse Move;
           the scheme is easily extensible for others. */
        enum Request { None, CircleRadius };

        /** Active request.
            That request's game side will post a flushRequests() call to this side. */
        Request m_activeRequest;

        /** New setCircleRadius() request, if set. */
        afl::base::Optional<int> m_circleRadius;

        /** Prepare for request of a given type.
            \param newRequest Request
            \retval true Post request to game normally
            \retval false Queue this request */
        bool checkRequest(Request newRequest);

        /** Send pending requests to game side.
            All methods must call this or checkRequest(). */
        void flushRequests();
    };

} }

#endif
