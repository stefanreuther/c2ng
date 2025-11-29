/**
  *  \file ui/res/resid.hpp
  *  \brief Resource Identifiers in PCC2ng
  *
  *  In PCC2ng, a resource identifier is a string.
  *  The string consists of a sequence of components separated by ".",
  *  getting more specific toward the end.
  *  For example, the resource identifier for a Merlin (hull #105, picture #153) is
  *     ship.153.105
  *  If no resource provider provides this exact identifier, it is generalized by leaving out components,
  *  producing "ship.153" and "ship".
  *
  *  We allow any number of components.
  *
  *  Resource Ids are used only internally.
  *  A possible "use this URL for this unit" mapping happens in the resource providers.
  *  This allows creators of resource Ids to create related resource Ids by using different prefixes.
  *
  *  \change PCC2 makes a class for resource identifiers and hardcodes the structure (string + 2 ints),
  *  making dealing with them a little more cumbersome.
  *  PCC2's print representation looks a little different, too ("ship153_105").
  */
#ifndef C2NG_UI_RES_RESID_HPP
#define C2NG_UI_RES_RESID_HPP

#include "afl/string/string.hpp"

/** Macro to mark resource Ids for easy searching.
    Mark every string or format string that eventually ends up in a resource request with this macro. */
#define RESOURCE_ID(x) x

namespace ui { namespace res {

    /** Resource Id Prefix: Standard ship picture.
        \param first   Hull::getExternalPictureNumber()
        \param second  Hull::getId() */
    extern const char SHIP[];

    /** Resource Id Prefix: Standard planet picture.
        \param first   Planet::getTemperature()
        \param second  Planet::getId() */
    extern const char PLANET[];

    /** Resource Id Prefix: Standard starbase picture.
        \param first   max(Planet::getBaseTechLevel())
        \param second  Planet::getId() */
    extern const char BASE[];

    /** Resource Id Prefix: Ship moving to the right in VCR (fighting on the left).
        \param first   Hull::getExternalPictureNumber()
        \param second  Hull::getId() */
    extern const char VCR_RSHIP[];

    /** Resource Id Prefix: Ship moving to the left in VCR (fighting on the right).
        \param first   Hull::getExternalPictureNumber()
        \param second  Hull::getId() */
    extern const char VCR_LSHIP[];

    /** Resource Id Prefix: Fighter moving to the right in VCR.
        \param first   Race number. */
    extern const char VCR_FIGHTER[];


    /** Make resource Id from prefix and one integer.
        \param prefix Prefix
        \param a Integer
        \return resource Id */
    String_t makeResourceId(const char* prefix, int a);

    /** Make resource Id from prefix and two integers.
        \param prefix Prefix
        \param a,b Integers
        \return resource Id */
    String_t makeResourceId(const char* prefix, int a, int b);

    /** Generalize resource Id.
        If the resource Id contains a period-separated element, removes that.
        \param s [in/out] Resource Id
        \retval true identifier could be generalized
        \retval false identifier could not be generalized and has been left unchanged */
    bool generalizeResourceId(String_t& s);

    /** Match resource Id to prefix and one integer.
        If \c resId was constructed as <tt>makeResourceId(prefix, a)</tt> for some integer,
        produces that integer.
        \param resId [in] Resource Id to test
        \param prefix [in] Prefix to test against
        \param a [out] Numeric suffix
        \retval true Match; a has been set
        \retval false Mismatch; a has unspecified value */
    bool matchResourceId(const String_t& resId, const char* prefix, int& a);

    /** Match resource Id to prefix and two integers.
        If \c resId was constructed as <tt>makeResourceId(prefix, a, b)</tt> for some integers,
        produces these integers.
        \param resId [in] Resource Id to test
        \param prefix [in] Prefix to test against
        \param a,b [out] Numeric suffixes
        \retval true Match; a and b have been set
        \retval false Mismatch; a and b have unspecified value */
    bool matchResourceId(const String_t& resId, const char* prefix, int& a, int& b);

} }

#endif
