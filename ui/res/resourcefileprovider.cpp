/**
  *  \file ui/res/resourcefileprovider.cpp
  */

#include "ui/res/resourcefileprovider.hpp"
#include "ui/res/resid.hpp"
#include "ui/res/manager.hpp"

namespace {
    /** PCC 1.x Resource Ids. For things that are pictures, add 20000 to get
        the 256-color version. Some are only available in 256 colors.

        This list corresponds to PCC 1.1.16. */
    enum {
        cc1id_NoPic        = 100,        /**< Placeholder for non-existant image. No longer exists in current cc.res. */
        cc1id_People       = 200,        /**< Race leader image. 200+race, actually. */

        /* Pictures from the Game */
        cc1id_PS_Mine      = 300,        /**< Planetary structure: mine */
        cc1id_PS_Factory   = 301,        /**< Planetary structure: factory */
        cc1id_PS_Defense   = 302,        /**< Planetary structure: defense */
        cc1id_Base_Hi      = 303,        /**< High-tech base */
        cc1id_Planet       = 304,        /**< Generic planet (VCR) */
        cc1id_PS_SBDefense = 305,        /**< Starbase defense */
        cc1id_Base_Lo      = 306,        /**< Low-tech base */
        cc1id_NVC          = 401,        /**< Non-visual contact */

        /* Pictures used in Help texts */
        cc1id_Help_Planet    = 350,
        cc1id_Help_IonMap    = 351,
        cc1id_Help_Minefield = 352,
        cc1id_Help_Ship      = 353,
        cc1id_Help_Transfer  = 354,
        cc1id_Help_Meteor    = 355,
        cc1id_Help_Title     = 356,
        cc1id_Help_Ion       = 357,
        cc1id_Help_Button    = 358,
        cc1id_Help_Legend    = 359,
        cc1id_Help_Compass   = 360,
        cc1id_Help_Fleets    = 361,
        cc1id_Help_Markers   = 362,
        cc1id_Help_Sectors   = 363,
        cc1id_Help_Sphere    = 364,      /**< Illustration of rectangular wrap. */
        cc1id_Help_Legend1   = 365,
        cc1id_Help_Legend2   = 366,
        cc1id_Help_Divider   = 367,      /**< Divider for script manual, only available as 20367 */
        cc1id_Help_RoundWrap = 368,      /**< Illustration of circular wrap. */

        /* Headings for multi-column checkbox arrays */
        cc1id_ChartOpt = 402,
        cc1id_AllyOpt  = 403,
        cc1id_XferOpt  = 404,

        /* Pictures from GUI */
        cc1id_CheckboxOff  = 410,
        cc1id_CheckboxOn   = 411,
        cc1id_CheckboxCond = 412,
        cc1id_CheckboxFill = 416,

        cc1id_Shade1      = 413,       /* "shade" on control screens */
        cc1id_Shade2      = 414,
        cc1id_Shade3      = 415,

        cc1id_PC_Active   = 417,       /* auto task program counter, subroutine is active (hollow triangle) */
        cc1id_PC_Inactive = 418,

        cc1id_Bulb        = 470,       /* Tip of the Day */

        cc1id_Texture1    = 4500,      /* backgrounds */
        cc1id_Texture2    = 4501,      /* window frames */
    
        /* Specification stuff */
        cc1id_Spec_PConfig  = 510,  /**< pconfig defaults. Highly version dependant binary format */
        cc1id_Spec_Disttabl = 511,  /**< Disttabl.dat */
    
        cc1id_Spec_Beamspec = 520,  /* compressed images of spec files */
        cc1id_Spec_Engspec  = 521,
        cc1id_Spec_Hullspec = 522,
        cc1id_Spec_Torpspec = 523,
        cc1id_Spec_Truehull = 524,
        cc1id_Spec_Xyplan   = 525,
        cc1id_Spec_PlanetNm = 526,
        cc1id_Spec_RaceNm   = 527,
        cc1id_Spec_StormNm  = 528,

        /* Tree Widget Help Texts */
        cc1id_Global_Help   = 550,
        cc1id_Setup_Help    = 551,
        cc1id_Hullfunc_Help = 552,

        /* Main Menu */
        cc1id_Menu_Ship   = 600,
        cc1id_Menu_Planet = 601,
        cc1id_Menu_Base   = 602,
        cc1id_Menu_Chart  = 603,
        cc1id_Menu_Help   = 604,

        /* Opener */
        cc1id_Opener_Bar1 = 700,
        cc1id_Opener_Bar2 = 701,
        cc1id_Opener_Load = 702,
        cc1id_Opener_Save = 703,

        /* Ships */
        cc1id_Ships = 1000,         /* 1000 + Hull().Image$, actually */
        /* 1500+ reserved for monochrome versions of ships */

        /* Climate. PCC generates the individual pictures at 3000+ from the
           ones at 2000+ */
        cc1id_Climate     = 2000,   /* 2000 + magic */
        cc1id_Climate_F   = 3000,   /* 3000 + fahrenheit */

        /* Fonts */
        cc1id_Font_Title  = 3900,
        cc1id_Font_Mono   = 3901,
        cc1id_Font_Small  = 3902,
        cc1id_Font_Normal = 3903,
        cc1id_Font_Tiny   = 3904,

        /* VCR */
        cc1id_VCR_Fighter = 4100,       /* 4100 + race */
        cc1id_VCR_Ship    = 4200,
        cc1id_VCR_Planet  = 4201,
        cc1id_VCR_Torp    = 4202,

        cc1id_VCR_RTL     = 4300,       /* these are the TV station logos shown in the upper-left :-) */
        cc1id_VCR_CNN     = 4301,
        cc1id_VCR_Cartoon = 4302,
        cc1id_VCR_NBC     = 4303,

        /* Weapons */
        cc1id_Fighter     = 4000,       /* 4000 + race */
        cc1id_Torps       = 4600,       /* + type */
        cc1id_Launchers   = 4700,       /* + type */
        cc1id_Beams       = 4800

        /* Unmapped here:
           4400 .. 4405     Bitmaps for Zap/Zot egg
           5000 .. 5006     Screen layouts
           10000 .. 19999   Help. PCC itself uses 10000..~10200 for regular help,
           12000..~12100 for glossary, 13000..13999 for rule help,
           14000..14999 for interpreter help */
    };
}

ui::res::ResourceFileProvider::ResourceFileProvider(afl::base::Ref<afl::io::Stream> file)
    : m_file(file)
{
    // ex ResProviderResFile::ResProviderResFile
}

ui::res::ResourceFileProvider::~ResourceFileProvider()
{ }

afl::base::Ptr<gfx::Canvas>
ui::res::ResourceFileProvider::loadImage(String_t name, Manager& mgr)
{
    // ex ResProviderResFile::loadPixmap
    int a;
    if (matchResourceId(name, SHIP, a)) {
        return loadImageById(cc1id_Ships + a, mgr);
    } else if (matchResourceId(name, BASE, a)) {
        return loadImageById(a > 6 ? cc1id_Base_Hi : cc1id_Base_Lo, mgr);
    } else if (matchResourceId(name, VCR_FIGHTER, a)) {
        return loadImageById(cc1id_VCR_Fighter + a, mgr);
    } else if (matchResourceId(name, PLANET, a)) {
        return loadImageById(cc1id_Climate_F + a, mgr);
    } else if (matchResourceId(name, "res", a)) {
        return loadImageById(a, mgr);
    } else {
        return 0;
    }
}

afl::base::Ptr<gfx::Canvas>
ui::res::ResourceFileProvider::loadImageById(uint16_t id, Manager& mgr)
{
    // ex ResProviderResFile::loadPixmap
    // Try 256-color version
    afl::base::Ptr<afl::io::Stream> in = m_file.openMember(id + 20000);
    if (in.get() != 0) {
        return mgr.loadImage(*in);
    }

    // Try 16-color version
    in = m_file.openMember(id);
    if (in.get() != 0) {
        return mgr.loadImage(*in);
    }

    // Not contained in resource file
    return 0;
}
