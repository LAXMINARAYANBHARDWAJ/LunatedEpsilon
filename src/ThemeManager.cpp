#include "ThemeManager.h"
#include "Logger.h"

#include <QApplication>
#include <QStyleFactory>
#include <QStyle>

Q_LOGGING_CATEGORY(lcTheme, "le.theme")

namespace LE {

ThemeManager::ThemeManager(QObject* parent)
    : QObject(parent)
{}

void ThemeManager::applyTheme(Theme theme)
{
    qCInfo(lcTheme) << "Applying theme:" << static_cast<int>(theme);

    m_current = theme;

    QApplication::setStyle(QStyleFactory::create("Fusion"));

    if (theme == Theme::System) {
        // Restore default palette and clear stylesheet override
        QApplication::setPalette(QApplication::style()->standardPalette());
        QApplication::instance()->setStyleSheet(QString{});
    } else {
        QPalette palette;

        switch (theme) {
            case Theme::Light:  palette = buildLightPalette();  break;
            case Theme::Dark:   palette = buildDarkPalette();   break;
            case Theme::AMOLED: palette = buildAmoledPalette(); break;
            default: break;
        }

        QApplication::setPalette(palette);
        QApplication::instance()->setStyleSheet(buildStyleSheet(theme));
    }

    emit themeChanged(theme);
    qCDebug(lcTheme) << "Theme applied successfully";
}

// ─── Palette Builders ────────────────────────────────────────────────────────

QPalette ThemeManager::buildLightPalette() const
{
    QPalette p;
    p.setColor(QPalette::Window,          QColor(0xF5F5F5));
    p.setColor(QPalette::WindowText,      QColor(0x1A1A1A));
    p.setColor(QPalette::Base,            QColor(0xFFFFFF));
    p.setColor(QPalette::AlternateBase,   QColor(0xEAEAEA));
    p.setColor(QPalette::Text,            QColor(0x1A1A1A));
    p.setColor(QPalette::Button,          QColor(0xE0E0E0));
    p.setColor(QPalette::ButtonText,      QColor(0x1A1A1A));
    p.setColor(QPalette::Highlight,       QColor(0x0078D4));
    p.setColor(QPalette::HighlightedText, QColor(0xFFFFFF));
    p.setColor(QPalette::PlaceholderText, QColor(0x888888));
    p.setColor(QPalette::ToolTipBase,     QColor(0xFFFFE1));
    p.setColor(QPalette::ToolTipText,     QColor(0x000000));
    return p;
}

QPalette ThemeManager::buildDarkPalette() const
{
    QPalette p;
    p.setColor(QPalette::Window,          QColor(0x1E1E1E));
    p.setColor(QPalette::WindowText,      QColor(0xE0E0E0));
    p.setColor(QPalette::Base,            QColor(0x252526));
    p.setColor(QPalette::AlternateBase,   QColor(0x2D2D30));
    p.setColor(QPalette::Text,            QColor(0xE0E0E0));
    p.setColor(QPalette::Button,          QColor(0x3C3C3C));
    p.setColor(QPalette::ButtonText,      QColor(0xE0E0E0));
    p.setColor(QPalette::Highlight,       QColor(0x0078D4));
    p.setColor(QPalette::HighlightedText, QColor(0xFFFFFF));
    p.setColor(QPalette::PlaceholderText, QColor(0x666666));
    p.setColor(QPalette::ToolTipBase,     QColor(0x2D2D30));
    p.setColor(QPalette::ToolTipText,     QColor(0xE0E0E0));
    return p;
}

QPalette ThemeManager::buildAmoledPalette() const
{
    QPalette p;
    p.setColor(QPalette::Window,          QColor(0x000000));
    p.setColor(QPalette::WindowText,      QColor(0xE0E0E0));
    p.setColor(QPalette::Base,            QColor(0x0A0A0A));
    p.setColor(QPalette::AlternateBase,   QColor(0x111111));
    p.setColor(QPalette::Text,            QColor(0xE0E0E0));
    p.setColor(QPalette::Button,          QColor(0x1A1A1A));
    p.setColor(QPalette::ButtonText,      QColor(0xE0E0E0));
    p.setColor(QPalette::Highlight,       QColor(0x0078D4));
    p.setColor(QPalette::HighlightedText, QColor(0xFFFFFF));
    p.setColor(QPalette::PlaceholderText, QColor(0x444444));
    p.setColor(QPalette::ToolTipBase,     QColor(0x0A0A0A));
    p.setColor(QPalette::ToolTipText,     QColor(0xE0E0E0));
    return p;
}

// ─── Stylesheet ──────────────────────────────────────────────────────────────

QString ThemeManager::buildStyleSheet(Theme theme) const
{
    // Fine-grained overrides that QPalette cannot express cleanly.
    // Intentionally minimal — palette carries the bulk of styling.
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