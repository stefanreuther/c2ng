/**
  *  \file game/interface/minefieldmethod.cpp
  *  \brief Enum game::interface::MinefieldMethod
  */

#include "game/interface/minefieldmethod.hpp"
#include "game/interface/objectcommand.hpp"

void
game::interface::callMinefieldMethod(game::map::Minefield& mf, MinefieldMethod imm, interpreter::Arguments& args, game::map::Universe& univ)
{
    switch (imm) {
     case immMark:
        IFObjMark(mf, args);
        break;

     case immUnmark:
        IFObjUnmark(mf, args);
        break;

     case immDelete:
        /* @q Delete (Minefield Command)
           Deletes the current minefield.
           Like <kbd>Del</kbd> in the minefield window,
           this can be used to delete minefields which are known to be out-of-date:
           | ForEach Minefield Do
           |   If Owner$=My.Race$ And Scanned<>3 Then Delete
           | Next
           deletes all your minefields which were not scanned this turn.
           If you're getting Winplan RSTs, you scan all your minefields each turn,
           so those you do not scan do no longer exist and will be deleted by the above command.

           After this command, all properties of the current minefield will yield EMPTY.
           @since PCC 1.0.12, PCC2 1.99.17, PCC2 2.40.1 */
        // ex values.pas:Minefield_Delete
        args.checkArgumentCount(0);
        univ.minefields().erase(mf.getId());
        break;
    }
}
