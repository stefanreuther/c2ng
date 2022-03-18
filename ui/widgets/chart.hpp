/**
  *  \file ui/widgets/chart.hpp
  *  \brief Class ui::widgets::Chart
  */
#ifndef C2NG_UI_WIDGETS_CHART_HPP
#define C2NG_UI_WIDGETS_CHART_HPP

#include <vector>
#include "gfx/basecontext.hpp"
#include "afl/container/ptrvector.hpp"
#include "ui/icons/icon.hpp"
#include "ui/root.hpp"
#include "ui/simplewidget.hpp"
#include "ui/tooltip.hpp"
#include "util/datatable.hpp"
#include "util/numberformatter.hpp"

namespace ui { namespace widgets {

    /** Chart (diagram) widget.
        This widget displays a util::DataTable as a chart with Id-based configuration of the layout.
        Styles can be defined for each row Id.
        Ids appearing in the data without configured style will be applied a default style. */
    class Chart : public SimpleWidget {
     public:
        /** Line Mode: extend this line to the left.
            Draws a horizontal line from the Y axis to the first data point. */
        static const int Line_ExtendLeft = 1;

        /** Line Mode: extend this line to the right.
            If the line (=row) has fewer data points (=columns) than the entire chart,
            draws a horizontal line to the end. */
        static const int Line_ExtendRight = 2;

        /** Line Mode: stop drawing on unknown points.
            By default, a line is drawn between know points, even if unknown points are inbetween. */
        static const int Line_SkipGaps = 4;

        /** Line Mode: label this line on the left. */
        static const int Line_LabelLeft = 8;


        /** Point icons. */
        enum PointIcon {
            NoIcon,
            DotIcon
        };

        /** Reference to a style for modification.
            (This interface reserves the right to support bulk modification.) */
        class StyleRef {
         public:
            /** Set line thickness.
                \param n Thickness in pixels. Default is 1. Set to 0 to hide this line.
                \return *this
                \see gfx::BaseContext::setLineThickness */
            StyleRef& setLineThickness(int n);

            /** Set line pattern.
                \param pattern Pattern. Default is SOLID_LINE.
                \return *this
                \see gfx::BaseContext::setLinePattern */
            StyleRef& setLinePattern(gfx::LinePattern_t pattern);

            /** Set color.
                \param color Color, from ui::ColorScheme. Default is Color_Black.
                \return *this */
            StyleRef& setColor(uint8_t color);

            /** Set alpha (transparency).
                \param alpha Alpha. Default is OPAQUE_ALPHA.
                \return *this
                \see gfx::BaseContext::setAlpha */
            StyleRef& setAlpha(gfx::Alpha_t alpha);

            /** Set line mode.
                \param mode New mode. Combination of Line_XXX constants.
                            Default is 0, with meanings of the bits chosen to produce the traditional behaviour.
                \return *this */
            StyleRef& setLineMode(int mode);

            /** Set point icon.
                \param icon New icon. Default is PointIcon.
                \return *this */
            StyleRef& setPointIcon(PointIcon icon);

            /** Set Z-order.
                Lines are drawn in ascending Z order.
                \param z Z order. Default is 0.
                \return *this */
            StyleRef& setZOrder(int z);

         private:
            friend class Chart;
            StyleRef(Chart& parent, size_t index);
            Chart& m_parent;
            size_t m_index;
        };

        /** Constructor.
            \param root Root
            \param size Default size
            \param fmt  Number formatter, for labels */
        Chart(Root& root, gfx::Point size, util::NumberFormatter fmt);
        ~Chart();

        /** Set content.
            \param data Data. Object will become owned by Chart. */
        void setContent(std::auto_ptr<util::DataTable> data);

        /** Set auxiliary content.
            This can contain additional rows that will be displayed like the regular content.
            For convenience, it can be exchanged separately.
            This is intended for auxiliary lines, for example.
            \param data Data. Object will become owned by Chart. */
        void setAuxContent(std::auto_ptr<util::DataTable> data);

        /** Get content.
            \return data. */
        const util::DataTable* getContent() const;

        /** Get default style.
            \return handle to modify this style. */
        StyleRef defaultStyle();

        /** Get style for a given Id.
            Creates a separate style slot by copying the default style, unless a style for this Id already exists.
            \param id Row Id
            \return handle to modify this style. */
        StyleRef style(int id);

        /** Add overlay icon.
            The icon is displayed on top of the chart.
            \param id    Id. If another icon with this Id already exists, it is replaced.
            \param pos   Position relative to top/left of widget
            \param pIcon Icon; will become owned by Chart. Passing null here is equivalent to removeIcon(). */
        void addNewIcon(int id, gfx::Point pos, ui::icons::Icon* pIcon);

        /** Remove overlay icon.
            \param id Id of icon */
        void removeIcon(int id);

        // SimpleWidget:
        virtual void draw(gfx::Canvas& can);
        virtual void handleStateChange(State st, bool enable);
        virtual void handlePositionChange(gfx::Rectangle& oldPosition);
        virtual ui::layout::Info getLayoutInfo() const;
        virtual bool handleKey(util::Key_t key, int prefix);
        virtual bool handleMouse(gfx::Point pt, MouseButtons_t pressedButtons);

     private:
        struct CompareZOrder;
        struct Layout;

        struct Style {
            int id;
            gfx::LinePattern_t linePattern;
            gfx::Alpha_t alpha;
            uint8_t lineThickness;
            uint8_t color;
            uint8_t lineMode;
            PointIcon pointIcon;
            int z;
        };

        struct Icon {
            int id;
            gfx::Point pos;
            std::auto_ptr<ui::icons::Icon> icon;
        };

        Root& m_root;
        gfx::Point m_size;
        std::vector<Style> m_style;
        std::auto_ptr<util::DataTable> m_data;
        std::auto_ptr<util::DataTable> m_auxData;
        util::NumberFormatter m_formatter;
        std::auto_ptr<Layout> m_layout;
        ui::Tooltip m_tooltip;
        afl::container::PtrVector<Icon> m_icons;

        const Layout& getLayout();

        void onTooltipHover(gfx::Point pos);

        static const Style DEFAULT_STYLE;
        const Style& getStyleForId(int id) const;
    };

} }

#endif
