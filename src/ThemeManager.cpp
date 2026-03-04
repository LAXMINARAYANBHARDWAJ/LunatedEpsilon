#include "ThemeManager.h"
#include "Logger.h"

#include <QApplication>
#include <QColor>
#include <QStyle>
#include <QStyleFactory>

Q_LOGGING_CATEGORY(lcTheme, "le.theme")

namespace LE {

ThemeManager::ThemeManager(QObject* parent)
    : QObject(parent)
{}

void ThemeManager::captureSystemDefaults() {
    if (m_capturedSystemDefaults) return;

    // Cast to QApplication to access stylesheet methods
    auto* app = qobject_cast<QApplication*>(QApplication::instance());
    if (!app) return;

    m_systemPalette = QApplication::palette();
    m_systemStyleSheet = app->styleSheet(); // Now visible

    if (auto* style = QApplication::style()) {
        m_systemStyleName = style->objectName();
    }

    m_capturedSystemDefaults = true;
}

void ThemeManager::applyTheme(Theme theme)
{
    qCInfo(lcTheme) << "Applying theme:" << static_cast<int>(theme);

    captureSystemDefaults();

    auto* app = qobject_cast<QApplication*>(QApplication::instance());
    if (!app) {
        m_current = theme;
        emit themeChanged(theme);
        return;
    }

    const auto restoreSystem = [&]() {
        if (!m_systemStyleName.isEmpty()) {
            if (auto* style = QStyleFactory::create(m_systemStyleName)) {
                QApplication::setStyle(style);
            }
        }

        QApplication::setPalette(m_systemPalette);
        app->setStyleSheet(buildStyleSheet(theme));
    };

    if (theme == Theme::System) {
        restoreSystem();
        m_current = theme;
        emit themeChanged(theme);
        qCDebug(lcTheme) << "Theme applied successfully";
        return;
    }

    if (auto* fusion = QStyleFactory::create("Fusion")) {
        QApplication::setStyle(fusion);
    } else {
        qCWarning(lcTheme) << "Fusion style unavailable; keeping current style";
    }

    QPalette palette;
    switch (theme) {
        case Theme::Light:  palette = buildLightPalette();  break;
        case Theme::Dark:   palette = buildDarkPalette();   break;
        case Theme::AMOLED: palette = buildAmoledPalette(); break;
        case Theme::System:
            break;
        default:
            qCWarning(lcTheme) << "Unknown theme value; restoring system defaults";
            restoreSystem();
            m_current = Theme::System;
            emit themeChanged(m_current);
            return;
    }

    QApplication::setPalette(palette);
    app->setStyleSheet(buildStyleSheet(theme));

    m_current = theme;
    emit themeChanged(theme);
    qCDebug(lcTheme) << "Theme applied successfully";
}

// ----------------
// Palette Builders
// ----------------

QPalette ThemeManager::buildLightPalette() const
{
    QPalette p;
    p.setColor(QPalette::Window,          QColor(QRgb(0xF5F5F5)));
    p.setColor(QPalette::WindowText,      QColor(QRgb(0x1A1A1A)));
    p.setColor(QPalette::Base,            QColor(QRgb(0xFFFFFF)));
    p.setColor(QPalette::AlternateBase,   QColor(QRgb(0xEAEAEA)));
    p.setColor(QPalette::Text,            QColor(QRgb(0x1A1A1A)));
    p.setColor(QPalette::Button,          QColor(QRgb(0xE0E0E0)));
    p.setColor(QPalette::ButtonText,      QColor(QRgb(0x1A1A1A)));
    p.setColor(QPalette::Highlight,       QColor(QRgb(0x0078D4)));
    p.setColor(QPalette::HighlightedText, QColor(QRgb(0xFFFFFF)));
    p.setColor(QPalette::PlaceholderText, QColor(QRgb(0x888888)));
    p.setColor(QPalette::ToolTipBase,     QColor(QRgb(0xFFFFE1)));
    p.setColor(QPalette::ToolTipText,     QColor(QRgb(0x000000)));
    return p;
}

QPalette ThemeManager::buildDarkPalette() const
{
    QPalette p;
    p.setColor(QPalette::Window,          QColor(QRgb(0x1E1E1E)));
    p.setColor(QPalette::WindowText,      QColor(QRgb(0xE0E0E0)));
    p.setColor(QPalette::Base,            QColor(QRgb(0x252526)));
    p.setColor(QPalette::AlternateBase,   QColor(QRgb(0x2D2D30)));
    p.setColor(QPalette::Text,            QColor(QRgb(0xE0E0E0)));
    p.setColor(QPalette::Button,          QColor(QRgb(0x3C3C3C)));
    p.setColor(QPalette::ButtonText,      QColor(QRgb(0xE0E0E0)));
    p.setColor(QPalette::Highlight,       QColor(QRgb(0x0078D4)));
    p.setColor(QPalette::HighlightedText, QColor(QRgb(0xFFFFFF)));
    p.setColor(QPalette::PlaceholderText, QColor(QRgb(0x666666)));
    p.setColor(QPalette::ToolTipBase,     QColor(QRgb(0x2D2D30)));
    p.setColor(QPalette::ToolTipText,     QColor(QRgb(0xE0E0E0)));
    return p;
}

QPalette ThemeManager::buildAmoledPalette() const
{
    QPalette p;
    p.setColor(QPalette::Window,          QColor(QRgb(0x000000)));
    p.setColor(QPalette::WindowText,      QColor(QRgb(0xE0E0E0)));
    p.setColor(QPalette::Base,            QColor(QRgb(0x0A0A0A)));
    p.setColor(QPalette::AlternateBase,   QColor(QRgb(0x111111)));
    p.setColor(QPalette::Text,            QColor(QRgb(0xE0E0E0)));
    p.setColor(QPalette::Button,          QColor(QRgb(0x1A1A1A)));
    p.setColor(QPalette::ButtonText,      QColor(QRgb(0xE0E0E0)));
    p.setColor(QPalette::Highlight,       QColor(QRgb(0x0078D4)));
    p.setColor(QPalette::HighlightedText, QColor(QRgb(0xFFFFFF)));
    p.setColor(QPalette::PlaceholderText, QColor(QRgb(0x444444)));
    p.setColor(QPalette::ToolTipBase,     QColor(QRgb(0x0A0A0A)));
    p.setColor(QPalette::ToolTipText,     QColor(QRgb(0xE0E0E0)));
    return p;
}

// ----------
// Stylesheet
// ----------

QString ThemeManager::buildStyleSheet(Theme theme) const
{
    // Fine-grained overrides that QPalette cannot express cleanly.
    // Intentionally minimal - palette carries the bulk of styling.
    switch (theme) {
        case Theme::Light:
            return QStringLiteral(
                "QScrollBar:vertical { background: #EBEBEB; width: 8px; }"
                "QScrollBar::handle:vertical { background: #BBBBBB; border-radius: 4px; }"
                "QToolTip { border: 1px solid #CCCCCC; }"
            );

        case Theme::Dark:
            return QStringLiteral(
                "QScrollBar:vertical { background: #2D2D30; width: 8px; }"
                "QScrollBar::handle:vertical { background: #555555; border-radius: 4px; }"
                "QToolTip { border: 1px solid #444444; }"
            );

        case Theme::AMOLED:
            return QStringLiteral(
                "QScrollBar:vertical { background: #000000; width: 8px; }"
                "QScrollBar::handle:vertical { background: #333333; border-radius: 4px; }"
                "QToolTip { border: 1px solid #222222; }"
            );

        default:
            return {};
    }
}

} // namespace LE
