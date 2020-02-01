/**
  *  \file game/interface/drawingmethod.cpp
  */

#include "game/interface/drawingmethod.hpp"
#include "game/interface/drawingproperty.hpp"

namespace {
    /* @q Delete (Drawing Command)
       Delete this drawing.
       @since PCC 1.0.14, PCC2 1.99.20, PCC2 2.40.1 */
    void IFDrawingDelete(game::map::DrawingContainer& container,
                         game::map::DrawingContainer::Iterator_t it,
                         interpreter::Arguments& args)
    {
        // ex values.pas:Marker_Delete
        args.checkArgumentCount(0);
        if (*it != 0) {
            container.erase(it);
        }
    }

    /* @q SetComment s:Str (Drawing Command)
       Set drawing comment.
       @see Comment (Drawing Property)
       @since PCC 1.0.14, PCC2 1.99.20, PCC2 2.40.1 */
    void IFDrawingSetComment(game::map::DrawingContainer& container,
                             game::map::DrawingContainer::Iterator_t it,
                             interpreter::Arguments& args)
    {
        // ex values.pas:Marker_SetComment
        args.checkArgumentCount(1);
        if (*it != 0) {
            // FIXME: PCC1 limits to markers:
            // IF (context^.pp^.Typ<>3) THEN BEGIN
            //   FPError_Str := 'Not a marker';
            //   Exit;
            // END;
            setDrawingProperty(**it, game::interface::idpComment, args.getNext());
            container.sig_change.raise();
        }
    }

    /* @q SetColor c:Int (Drawing Command)
       Set drawing color.
       @see Color (Drawing Property)
       @since PCC 1.0.14, PCC2 1.99.20 */
    void IFDrawingSetColor(game::map::DrawingContainer& container,
                           game::map::DrawingContainer::Iterator_t it,
                           interpreter::Arguments& args)
    {
        // ex values.pas:Marker_SetColor
        args.checkArgumentCount(1);
        if (*it != 0) {
            setDrawingProperty(**it, game::interface::idpColor, args.getNext());
            container.sig_change.raise();
        }
    }
}

void
game::interface::callDrawingMethod(game::map::DrawingContainer& container,
                                   game::map::DrawingContainer::Iterator_t it,
                                   DrawingMethod method,
                                   interpreter::Arguments& args)
{
    switch (method) {
     case idmDelete:
        IFDrawingDelete(container, it, args);
        break;
     case idmSetComment:
        IFDrawingSetComment(container, it, args);
        break;
     case idmSetColor:
        IFDrawingSetColor(container, it, args);
        break;
    }
}
