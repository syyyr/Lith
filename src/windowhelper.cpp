#include "windowhelper.h"
#include "lith.h"

#include <QApplication>
#include <QPalette>
#include <QtMath>

#include <QtGui/qpa/qplatformwindow.h>

#include <math.h>

WindowHelper::WindowHelper(QObject *parent) : QObject(parent) {
}

void WindowHelper::init() {
    if (Lith::instance()->settingsGet()->forceDarkThemeGet())
        m_darkTheme = true;
    else if (Lith::instance()->settingsGet()->forceLightThemeGet())
        m_darkTheme = false;
    else
        m_darkTheme = detectSystemDarkStyle();
    emit darkThemeChanged();

    resetColorScheme();
}

qreal WindowHelper::getBottomSafeAreaSize() {
    if (qApp->allWindows().count() > 0) {
        auto window = qApp->allWindows().first();
        QPlatformWindow *platformWindow = static_cast<QPlatformWindow *>(window->handle());
        QMargins margins = platformWindow->safeAreaMargins();
        return margins.bottom();
    }
    else {
        return 0.0;
    }
}

bool WindowHelper::detectSystemDarkStyle() {
#if WIN32
    QSettings registry("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", QSettings::NativeFormat);
    auto value = registry.value("AppsUseLightTheme");
    if (value.isValid() && value.canConvert<int>()) {
        return !value.toInt();
    }
    return false;
#else
    const QColor textColor = QGuiApplication::palette().color(QPalette::Text);
    if (qSqrt(((textColor.red() * textColor.red()) * 0.299) +
              ((textColor.green() * textColor.green()) * 0.587) +
              ((textColor.blue() * textColor.blue()) * 0.114)) > 128)
        return true;
    return false;
#endif
}

void WindowHelper::resetColorScheme() {
    QPalette palette;

    auto darken = [](const QColor &color, qreal amount = 0.01) {
        qreal h, s, l, a;
        color.getHslF(&h, &s, &l, &a);

        qreal lightness = l - amount;
        if (lightness < 0) {
            lightness = 0;
        }

        return QColor::fromHslF(h, s, lightness, a);
    };

    auto desaturate = [](const QColor &color, qreal amount = 0.01) {
        qreal h, s, l, a;
        color.getHslF(&h, &s, &l, &a);

        qreal saturation = s - amount;
        if (saturation < 0) {
            saturation = 0;
        }
        return QColor::fromHslF(h, saturation, l, a);
    };

    auto lighten = [](const QColor &color, qreal amount = 0.01)
    {
        qreal h, s, l, a;
        color.getHslF(&h, &s, &l, &a);

        qreal lightness = l + amount;
        if (lightness > 1) {
            lightness = 1;
        }
        return QColor::fromHslF(h, s, lightness, a);
    };

    auto mix = [](const QColor &c1, const QColor &c2, qreal bias = 0.5)
    {
        auto mixQreal = [](qreal a, qreal b, qreal bias) {
            return a + (b - a) * bias;
        };

        if (bias <= 0.0) {
            return c1;
        }

        if (bias >= 1.0) {
            return c2;
        }

        if (std::isnan(bias)) {
            return c1;

        }

        qreal r = mixQreal(c1.redF(), c2.redF(), bias);
        qreal g = mixQreal(c1.greenF(), c2.greenF(), bias);
        qreal b = mixQreal(c1.blueF(), c2.blueF(), bias);
        qreal a = mixQreal(c1.alphaF(), c2.alphaF(), bias);

        return QColor::fromRgbF(r, g, b, a);
    };

    if (m_darkTheme) {
        // Colors defined in GTK adwaita style in _colors.scss
        QColor base_color;
        QColor bg_color;
        if (Lith::instance()->settingsGet()->useTrueBlackWithDarkThemeGet()) {
            base_color = Qt::black;
            bg_color = Qt::black;
        }
        else {
            base_color = lighten(desaturate(QColor("#241f31"), 1.0), 0.02);
            bg_color = darken(desaturate(QColor("#3d3846"), 1.0), 0.04);
        }
        QColor text_color = QColor("white");
        QColor fg_color = QColor("#eeeeec");
        QColor selected_bg_color = darken(QColor("#3584e4"), 0.2);
        QColor selected_fg_color = QColor("white");
        QColor osd_text_color = QColor("white");
        QColor osd_bg_color = QColor("black");
        QColor shadow = ("#dd000000");

        QColor backdrop_fg_color = mix(fg_color, bg_color);
        QColor backdrop_base_color = lighten(base_color, 0.01);
        QColor backdrop_selected_fg_color = mix(text_color, backdrop_base_color, 0.2);

        // This is the color we use as initial color for the gradient in normal state
        // Defined in _drawing.scss button(normal)
        QColor button_base_color = darken(bg_color, 0.01);

        QColor link_color = lighten(selected_bg_color, 0.2);
        QColor link_visited_color = lighten(selected_bg_color, 0.1);

        palette.setColor(QPalette::All,      QPalette::Window,          bg_color);
        palette.setColor(QPalette::All,      QPalette::WindowText,      fg_color);
        palette.setColor(QPalette::All,      QPalette::Base,            base_color);
        palette.setColor(QPalette::All,      QPalette::AlternateBase,   base_color);
        palette.setColor(QPalette::All,      QPalette::ToolTipBase,     osd_bg_color);
        palette.setColor(QPalette::All,      QPalette::ToolTipText,     osd_text_color);
        palette.setColor(QPalette::All,      QPalette::Text,            fg_color);
        palette.setColor(QPalette::All,      QPalette::Button,          button_base_color);
        palette.setColor(QPalette::All,      QPalette::ButtonText,      fg_color);
        palette.setColor(QPalette::All,      QPalette::BrightText,      text_color);

        palette.setColor(QPalette::All,      QPalette::Light,           lighten(button_base_color));
        palette.setColor(QPalette::All,      QPalette::Midlight,        mix(lighten(button_base_color), button_base_color));
        palette.setColor(QPalette::All,      QPalette::Mid,             mix(darken(button_base_color), button_base_color));
        palette.setColor(QPalette::All,      QPalette::Dark,            darken(button_base_color));
        palette.setColor(QPalette::All,      QPalette::Shadow,          shadow);

        palette.setColor(QPalette::All,      QPalette::Highlight,       selected_bg_color);
        palette.setColor(QPalette::All,      QPalette::HighlightedText, selected_fg_color);

        palette.setColor(QPalette::All,      QPalette::Link,            link_color);
        palette.setColor(QPalette::All,      QPalette::LinkVisited,     link_visited_color);


        QColor insensitive_fg_color = mix(fg_color, bg_color);
        QColor insensitive_bg_color = mix(bg_color, base_color, 0.4);

        palette.setColor(QPalette::Disabled, QPalette::Window,          insensitive_bg_color);
        palette.setColor(QPalette::Disabled, QPalette::WindowText,      insensitive_fg_color);
        palette.setColor(QPalette::Disabled, QPalette::Base,            base_color);
        palette.setColor(QPalette::Disabled, QPalette::AlternateBase,   base_color);
        palette.setColor(QPalette::Disabled, QPalette::Text,            insensitive_fg_color);
        palette.setColor(QPalette::Disabled, QPalette::Button,          insensitive_bg_color);
        palette.setColor(QPalette::Disabled, QPalette::ButtonText,      insensitive_fg_color);
        palette.setColor(QPalette::Disabled, QPalette::BrightText,      text_color);

        palette.setColor(QPalette::Disabled, QPalette::Light,           lighten(insensitive_bg_color));
        palette.setColor(QPalette::Disabled, QPalette::Midlight,        mix(lighten(insensitive_bg_color), insensitive_bg_color));
        palette.setColor(QPalette::Disabled, QPalette::Mid,             mix(darken(insensitive_bg_color), insensitive_bg_color));
        palette.setColor(QPalette::Disabled, QPalette::Dark,            darken(insensitive_bg_color));
        palette.setColor(QPalette::Disabled, QPalette::Shadow,          shadow);

        palette.setColor(QPalette::Disabled, QPalette::Highlight,       selected_bg_color);
        palette.setColor(QPalette::Disabled, QPalette::HighlightedText, selected_fg_color);

        palette.setColor(QPalette::Disabled, QPalette::Link,            link_color);
        palette.setColor(QPalette::Disabled, QPalette::LinkVisited,     link_visited_color);


        palette.setColor(QPalette::Inactive, QPalette::Window,          bg_color);
        palette.setColor(QPalette::Inactive, QPalette::WindowText,      backdrop_fg_color);
        palette.setColor(QPalette::Inactive, QPalette::Base,            backdrop_base_color);
        palette.setColor(QPalette::Inactive, QPalette::AlternateBase,   backdrop_base_color);
        palette.setColor(QPalette::Inactive, QPalette::Text,            backdrop_fg_color);
        palette.setColor(QPalette::Inactive, QPalette::Button,          button_base_color);
        palette.setColor(QPalette::Inactive, QPalette::ButtonText,      backdrop_fg_color);
        palette.setColor(QPalette::Inactive, QPalette::BrightText,      text_color);

        palette.setColor(QPalette::Inactive, QPalette::Light,           lighten(insensitive_bg_color));
        palette.setColor(QPalette::Inactive, QPalette::Midlight,        mix(lighten(insensitive_bg_color), insensitive_bg_color));
        palette.setColor(QPalette::Inactive, QPalette::Mid,             mix(darken(insensitive_bg_color), insensitive_bg_color));
        palette.setColor(QPalette::Inactive, QPalette::Dark,            darken(insensitive_bg_color));
        palette.setColor(QPalette::Inactive, QPalette::Shadow,          shadow);

        palette.setColor(QPalette::Inactive, QPalette::Highlight,       selected_bg_color);
        palette.setColor(QPalette::Inactive, QPalette::HighlightedText, backdrop_selected_fg_color);

        palette.setColor(QPalette::Inactive, QPalette::Link,            link_color);
        palette.setColor(QPalette::Inactive, QPalette::LinkVisited,     link_visited_color);
    }
    else {
        // Colors defined in GTK adwaita style in _colors.scss
        QColor base_color = QColor("white");
        QColor text_color = QColor("black");
        QColor bg_color = QColor("#f6f5f4");
        QColor fg_color = QColor("#2e3436");
        QColor selected_bg_color = QColor("#3584e4");
        QColor selected_fg_color = QColor("white");
        QColor osd_text_color = QColor("white");
        QColor osd_bg_color = QColor("black");
        QColor shadow = ("#dd000000");

        QColor backdrop_fg_color = mix(fg_color, bg_color);
        QColor backdrop_base_color = darken(base_color, 0.01);
        QColor backdrop_selected_fg_color = backdrop_base_color;

        // This is the color we use as initial color for the gradient in normal state
        // Defined in _drawing.scss button(normal)
        QColor button_base_color = darken(bg_color, 0.04);

        QColor link_color = darken(selected_bg_color, 0.1);
        QColor link_visited_color = darken(selected_bg_color, 0.2);

        palette.setColor(QPalette::All,      QPalette::Window,          bg_color);
        palette.setColor(QPalette::All,      QPalette::WindowText,      fg_color);
        palette.setColor(QPalette::All,      QPalette::Base,            base_color);
        palette.setColor(QPalette::All,      QPalette::AlternateBase,   base_color);
        palette.setColor(QPalette::All,      QPalette::ToolTipBase,     osd_bg_color);
        palette.setColor(QPalette::All,      QPalette::ToolTipText,     osd_text_color);
        palette.setColor(QPalette::All,      QPalette::Text,            fg_color);
        palette.setColor(QPalette::All,      QPalette::Button,          button_base_color);
        palette.setColor(QPalette::All,      QPalette::ButtonText,      fg_color);
        palette.setColor(QPalette::All,      QPalette::BrightText,      text_color);

        palette.setColor(QPalette::All,      QPalette::Light,           lighten(button_base_color));
        palette.setColor(QPalette::All,      QPalette::Midlight,        mix(lighten(button_base_color), button_base_color));
        palette.setColor(QPalette::All,      QPalette::Mid,             mix(darken(button_base_color), button_base_color));
        palette.setColor(QPalette::All,      QPalette::Dark,            darken(button_base_color));
        palette.setColor(QPalette::All,      QPalette::Shadow,          shadow);

        palette.setColor(QPalette::All,      QPalette::Highlight,       selected_bg_color);
        palette.setColor(QPalette::All,      QPalette::HighlightedText, selected_fg_color);

        palette.setColor(QPalette::All,      QPalette::Link,            link_color);
        palette.setColor(QPalette::All,      QPalette::LinkVisited,     link_visited_color);

        QColor insensitive_fg_color = mix(fg_color, bg_color);
        QColor insensitive_bg_color = mix(bg_color, base_color, 0.4);

        palette.setColor(QPalette::Disabled, QPalette::Window,          insensitive_bg_color);
        palette.setColor(QPalette::Disabled, QPalette::WindowText,      insensitive_fg_color);
        palette.setColor(QPalette::Disabled, QPalette::Base,            base_color);
        palette.setColor(QPalette::Disabled, QPalette::AlternateBase,   base_color);
        palette.setColor(QPalette::Disabled, QPalette::Text,            insensitive_fg_color);
        palette.setColor(QPalette::Disabled, QPalette::Button,          insensitive_bg_color);
        palette.setColor(QPalette::Disabled, QPalette::ButtonText,      insensitive_fg_color);
        palette.setColor(QPalette::Disabled, QPalette::BrightText,      text_color);

        palette.setColor(QPalette::Disabled, QPalette::Light,           lighten(insensitive_bg_color));
        palette.setColor(QPalette::Disabled, QPalette::Midlight,        mix(lighten(insensitive_bg_color), insensitive_bg_color));
        palette.setColor(QPalette::Disabled, QPalette::Mid,             mix(darken(insensitive_bg_color), insensitive_bg_color));
        palette.setColor(QPalette::Disabled, QPalette::Dark,            darken(insensitive_bg_color));
        palette.setColor(QPalette::Disabled, QPalette::Shadow,          shadow);

        palette.setColor(QPalette::Disabled, QPalette::Highlight,       selected_bg_color);
        palette.setColor(QPalette::Disabled, QPalette::HighlightedText, selected_fg_color);

        palette.setColor(QPalette::Disabled, QPalette::Link,            link_color);
        palette.setColor(QPalette::Disabled, QPalette::LinkVisited,     link_visited_color);


        palette.setColor(QPalette::Inactive, QPalette::Window,          bg_color);
        palette.setColor(QPalette::Inactive, QPalette::WindowText,      backdrop_fg_color);
        palette.setColor(QPalette::Inactive, QPalette::Base,            backdrop_base_color);
        palette.setColor(QPalette::Inactive, QPalette::AlternateBase,   backdrop_base_color);
        palette.setColor(QPalette::Inactive, QPalette::Text,            backdrop_fg_color);
        palette.setColor(QPalette::Inactive, QPalette::Button,          button_base_color);
        palette.setColor(QPalette::Inactive, QPalette::ButtonText,      backdrop_fg_color);
        palette.setColor(QPalette::Inactive, QPalette::BrightText,      text_color);

        palette.setColor(QPalette::Inactive, QPalette::Light,           lighten(insensitive_bg_color));
        palette.setColor(QPalette::Inactive, QPalette::Midlight,        mix(lighten(insensitive_bg_color), insensitive_bg_color));
        palette.setColor(QPalette::Inactive, QPalette::Mid,             mix(darken(insensitive_bg_color), insensitive_bg_color));
        palette.setColor(QPalette::Inactive, QPalette::Dark,            darken(insensitive_bg_color));
        palette.setColor(QPalette::Inactive, QPalette::Shadow,          shadow);

        palette.setColor(QPalette::Inactive, QPalette::Highlight,       selected_bg_color);
        palette.setColor(QPalette::Inactive, QPalette::HighlightedText, backdrop_selected_fg_color);

        palette.setColor(QPalette::Inactive, QPalette::Link,            link_color);
        palette.setColor(QPalette::Inactive, QPalette::LinkVisited,     link_visited_color);
    }

    qApp->setPalette(palette);
}