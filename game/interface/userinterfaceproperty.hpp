/**
  *  \file game/interface/userinterfaceproperty.hpp
  */
#ifndef C2NG_GAME_INTERFACE_USERINTERFACEPROPERTY_HPP
#define C2NG_GAME_INTERFACE_USERINTERFACEPROPERTY_HPP

namespace game { namespace interface {

    /** User interface property identifier. */
    enum UserInterfaceProperty {
        iuiScreenNumber,               ///< Current screen number (UI.Screen).
        iuiIterator,                   ///< Current iterator (UI.Iterator).
        // iuiScreenRegistered,           ///< true iff current screen is on screen history.
        iuiSimFlag,                    ///< true iff we are in the simulator (System.Sim).
        iuiScanX,                      ///< Scanner coordinate, X (UI.X). Can be null.
        iuiScanY,                      ///< Scanner coordinate, Y (UI.Y). Can be null.
        iuiChartX,                     ///< Starchart coordinate, X (Chart.X). Can be null.
        iuiChartY                      ///< Starchart coordinate, Y (Chart.Y). Can be null.
        // iuiKeymap                      ///< Keymap. A keymap object, do not expose to users! Can be null.
    };

} }

#endif
