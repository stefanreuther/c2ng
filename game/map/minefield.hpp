/**
  *  \file game/map/minefield.hpp
  *  \brief Class game::map::Minefield
  */
#ifndef C2NG_GAME_MAP_MINEFIELD_HPP
#define C2NG_GAME_MAP_MINEFIELD_HPP

#include "game/config/hostconfiguration.hpp"
#include "game/hostversion.hpp"
#include "game/map/circularobject.hpp"

namespace game { namespace map {

    /** Minefield.
        Minefields can be scanned anew or known from history.
        For this, the Minefield class allows consumption of reports.

        In addition, we allow temporary freestanding Minefield objects for computations.
        Thos can be manipulated at will. */
    class Minefield : public CircularObject {
     public:
        /** Minefield type in a report. */
        enum TypeReport {
            UnknownType,        ///< Report does not say what type this minefield is.
            IsMine,             ///< Report says this is a regular minefield.
            IsWeb               ///< Report says this is a web minefield.
        };

        /** Size in a report. */
        enum SizeReport {
            RadiusKnown,        ///< Report includes a minefield radius.
            UnitsKnown          ///< Report includes a minefield unit count.
        };

        /** Actions that lead to Minefield Update.
            These must keep their relative and absolute values,
            because these equal the 'ScanType' of a UTILx.DAT entry, plus one.

            The order indicates a priority reason:
            a minefield can be laid, then swept, then scanned;
            thus, "scan" reports have most recent data. */
        enum ReasonReport {
            NoReason,           ///< Unknown reason/known from history. ex mfa_None
            MinefieldLaid,      ///< Minefield was laid. ex mfa_Laid
            MinefieldSwept,     ///< Minefield was swept. ex mfa_Swept
            MinefieldScanned    ///< Minefield was scanned. ex mfa_Scanned
        };

        /** Constructor.
            Makes an object representing a nonexistant minefield.
            \param id Minefield Id */
        explicit Minefield(Id_t id);

        /** Copy constructor.
            Makes a copy of another object.
            \param other Other object */
        Minefield(const Minefield& other);

        /** Parameterized constructor.
            Makes a minefield with the given parameters.
            Use for temporary objects.
            \param id     Minefield Id
            \param center Minefield center position
            \param owner  Minefield owner
            \param isWeb  true for web minefield
            \param units  Number of mine units */
        Minefield(Id_t id, Point center, int owner, bool isWeb, int32_t units);

        /** Destructor. */
        virtual ~Minefield();

        // Object:
        virtual String_t getName(ObjectName which, afl::string::Translator& tx, const InterpreterInterface& iface) const;
        virtual afl::base::Optional<int> getOwner() const;
        virtual afl::base::Optional<Point> getPosition() const;

        // CircularObject:
        virtual afl::base::Optional<int> getRadius() const;
        virtual afl::base::Optional<int32_t> getRadiusSquared() const;

        /** Add minefield report.
            New information is included in this object if it is newer or better than existing information.
            \param pos       Minefield center position
            \param owner     Minefield owner
            \param type      Minefield type
            \param size      Determines meaning of sizeValue
            \param sizeValue Minefield size (units or radius)
            \param turn      Turn number
            \param reason    Reason of this report */
        void addReport(Point pos, int owner, TypeReport type, SizeReport size, int32_t sizeValue, int turn, ReasonReport reason);

        /** Do internal checks for this minefield.
            Internal checks do not require a partner to interact with.
            If this is a history minefield, this will compute the current information (mine decay).
            \param currentTurn Turn number
            \param host        Host version (determines formulas)
            \param config      Host configuration (determines parameters to formulas) */
        void internalCheck(int currentTurn, const game::HostVersion& host, const game::config::HostConfiguration& config);

        /** Erase this minefield by making it not valid.
            As per the rule that objects that are part of a Universe never disappear, the object remains existant.
            \param sig sig_setChange signal to raise in the process */
        void erase(afl::base::Signal<void(Id_t)>* sig);

        /** Set number of mine units.
            This method is intended to be used on temporary Minefield objects only, not on those in the universe.
            \param units Minefield units */
        void setUnits(int32_t units);

        /** Check validity.
            \return true if minefield is valid. If false, other members shall not be used. */
        bool isValid() const;

        /** Check for web minefield.
            \return true if web minefield */
        bool isWeb() const;

        /** Get reason why this minefield is seen.
            \return best ReasonReport that was used for this minefield. */
        ReasonReport getReason() const;

        /** Get number of minefield units.
            \return Number of units */
        int32_t getUnits() const;

        /** Get number of units after one turn of decay.
            This applies the mine decay formula to the given number of units, using the other parameters from this minefield.
            \param origUnits Number of units
            \param host        Host version (determines formulas)
            \param config      Host configuration (determines parameters to formulas)
            \return number of units after decay */
        int32_t getUnitsAfterDecay(int32_t origUnits, const game::HostVersion& host, const game::config::HostConfiguration& config) const;

        /** Get number of minefield units to consider for mine laying prediction.
            \param host        Host version (determines formulas)
            \param config      Host configuration (determines parameters to formulas)
            \return number of units (=radius-squared)
            \see HostVersion::isMineLayingAfterMineDecay() */
        int32_t getUnitsForLaying(const game::HostVersion& host, const game::config::HostConfiguration& config) const;

        /** Get turn when minefield was last scanned.
            \return turn number */
        int getTurnLastSeen() const;

        /** Get number of units when minefield was last scanned.
            \return units */
        int32_t getUnitsLastSeen() const;

        /** Compute successful passage rate.
            This is the inverse of the "hit rate", for a given distance.
            Under THost, there is only "the" hit rate.
            Under PHost, various options are arrayized, and dynamic by experience and
            speed. This function returns the value for an inexperienced ship owned by the given player, at warp 9 (=the worst possible case).

            Note that actually under THost the problem is much more complicated due to the interesting implementation;
            see <http://phost.de/~stefan/minehits.html>.
            We do not attempt to emulate that here.

            \param distance Distance to cover, in ly
            \param cloaked  Whether ship is cloaked
            \param player   Player
            \param config   Host configuration
            \return Passage rate, [0.0, 1.0] */
        double getPassRate(double distance, bool cloaked, int player, const game::config::HostConfiguration& config) const;

        /** Compute minefield radius from unit number.
            \param units Number of units
            \return radius */
        static int32_t getRadiusFromUnits(int32_t units);

     private:
        // Main information
        Point m_position;       // x,y
        int m_owner;            // owner
        bool m_isWeb;           // web
        int32_t m_units;        // units
        int m_turn;             // turn (last seen)
        ReasonReport m_reason;  // last_action

        // previous data
        int m_previousTurn;
        int32_t m_previousUnits;

        // cached current (current turn)
        int m_currentTurn;
        int m_currentRadius;
        int32_t m_currentUnits;
    };

} }

#endif
