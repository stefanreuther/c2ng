/**
  *  \file game/map/minefield.hpp
  */
#ifndef C2NG_GAME_MAP_MINEFIELD_HPP
#define C2NG_GAME_MAP_MINEFIELD_HPP

#include "game/map/circularobject.hpp"
#include "game/hostversion.hpp"
#include "game/config/hostconfiguration.hpp"

namespace game { namespace map {

    class Minefield : public CircularObject {
     public:
        enum TypeReport {
            UnknownType,
            IsMine,
            IsWeb
        };
        enum SizeReport {
            RadiusKnown,
            UnitsKnown
        };
        /** Actions that lead to Minefield Update. These must keep their
            relative and absolute values, because these equal the `ScanType'
            of a UTILx.DAT entry, plus one. */
        enum ReasonReport {
            NoReason,           // ex mfa_None
            MinefieldLaid,      // ex mfa_Laid
            MinefieldSwept,     // ex mfa_Swept
            MinefieldScanned    // ex mfa_Scanned
        };

        // Constructor and Destructor:
        Minefield(Id_t id);
        virtual ~Minefield();

        // Object:
        virtual String_t getName(Name which, afl::string::Translator& tx, InterpreterInterface& iface) const;
        virtual Id_t getId() const;
        virtual bool getOwner(int& result) const;

        // MapObject:
        virtual bool getPosition(Point& result) const;

        // CircularObject:
        virtual bool getRadius(int& result) const;
        virtual bool getRadiusSquared(int32_t& result) const;

        // Minefield modification methods:
        void addReport(Point pos, int owner, TypeReport type, SizeReport size, int32_t sizeValue, int turn, ReasonReport reason);
        void internalCheck(int currentTurn, const game::HostVersion& host, const game::config::HostConfiguration& config);
        void erase();

        // Minefield inquiry methods:
        bool isValid() const;
        bool isWeb() const;
        ReasonReport getReason() const;
        int32_t getUnits() const;
        int32_t getUnitsAfterDecay(int32_t origUnits, const game::HostVersion& host, const game::config::HostConfiguration& config) const;
        int32_t getUnitsForLaying(const game::HostVersion& host, const game::config::HostConfiguration& config) const;
        int getTurnLastSeen() const;
        int32_t getUnitsLastSeen() const;
        double getPassRate(double distance, bool cloaked, int player, const game::config::HostConfiguration& config) const;

        static int32_t getRadiusFromUnits(int32_t units);

     private:
        const Id_t m_id;        // id

        // Main information
        Point m_position;       // x,y
        int m_owner;            // owner
        bool m_isWeb;           // web
        int32_t m_units;        // units
        int m_turn;             // turn (last seen)
        ReasonReport m_reason;  // last_action

        // previous data - FIXME: do we need this?
        int m_previousTurn;
        int32_t m_previousUnits;

        // cached current (current turn)
        int m_currentRadius;
        int32_t m_currentUnits;
    };

} }

#endif
