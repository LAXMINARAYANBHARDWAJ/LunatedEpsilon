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
        QApplication::setPalette(QApplication::style()->standardPalette());
        QApplication::instance()->setStyleSheet(buildStyleSheet(Theme::System));
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
    p.setColor(QPalette::Window,          QColor(0xF3F3F3));
    p.setColor(QPalette::WindowText,      QColor(0x1A1A1A));
    p.setColor(QPalette::Base,            QColor(0xFFFFFF));
    p.setColor(QPalette::AlternateBase,   QColor(0xEAEAEA));
    p.setColor(QPalette::Text,            QColor(0x1A1A1A));
    p.setColor(QPalette::Button,          QColor(0xE0E0E0));
    p.setColor(QPalette::ButtonText,      QColor(0x1A1A1A));
    p.setColor(QPalette::Highlight,       QColor(0x0067C0));
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
    p.setColor(QPalette::WindowText,      QColor(0xDCDCDC));
    p.setColor(QPalette::Base,            QColor(0x252526));
    p.setColor(QPalette::AlternateBase,   QColor(0x2D2D30));
    p.setColor(QPalette::Text,            QColor(0xDCDCDC));
    p.setColor(QPalette::Button,          QColor(0x3C3C3C));
    p.setColor(QPalette::ButtonText,      QColor(0xDCDCDC));
    p.setColor(QPalette::Highlight,       QColor(0x0078D4));
    p.setColor(QPalette::HighlightedText, QColor(0xFFFFFF));
    p.setColor(QPalette::PlaceholderText, QColor(0x555555));
    p.setColor(QPalette::ToolTipBase,     QColor(0x2D2D30));
    p.setColor(QPalette::ToolTipText,     QColor(0xDCDCDC));
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
    p.setColor(QPalette::PlaceholderText, QColor(0x3A3A3A));
    p.setColor(QPalette::ToolTipBase,     QColor(0x0A0A0A));
    p.setColor(QPalette::ToolTipText,     QColor(0xE0E0E0));
    return p;
}

// ─── Stylesheet ──────────────────────────────────────────────────────────────
//
// Structure: each theme block defines rules for the following components in order:
//   1. Title bar (#titleBar) and its label
//   2. Title bar control buttons — normal, hover, pressed, close variants
//   3. Theme selector combobox (#themeBox) in the title bar
//   4. Central widget background (#centralWidget)
//   5. Select and Convert buttons (#selectBtn, #convertBtn)
//   6. Browse buttons (generic QPushButton inside path widgets)
//   7. QLineEdit (path input fields)
//   8. QComboBox (location mode selector)
//   9. QProgressBar
//  10. Status / file labels (#statusLabel, #fileLabel)
//  11. QScrollBar
//  12. QToolTip
//  13. QMenu (context menu)
//
// QPalette covers base colours; this stylesheet handles geometry, radius,
// border, hover/pressed transitions, and elements QPalette cannot express.

QString ThemeManager::buildStyleSheet(Theme theme) const
{
    switch (theme) {

    // ── System ───────────────────────────────────────────────────────────────
    // Minimal overrides — let the platform style lead. Only fix geometry and
    // the title bar, which has no native equivalent regardless of theme.
    case Theme::System:
        return QStringLiteral(
            /* Title bar */
            "QWidget#titleBar {"
            "  background-color: palette(window);"
            "  border-bottom: 1px solid palette(mid);"
            "}"
            "QLabel#titleLabel {"
            "  color: palette(window-text);"
            "  font-size: 13px;"
            "  font-weight: 600;"
            "  padding-left: 2px;"
            "}"

            /* Title bar buttons */
            "QPushButton#titleBarBtn {"
            "  background: transparent;"
            "  border: none;"
            "  color: palette(window-text);"
            "  font-size: 13px;"
            "}"
            "QPushButton#titleBarBtn:hover {"
            "  background-color: palette(mid);"
            "}"
            "QPushButton#titleBarBtn:pressed {"
            "  background-color: palette(dark);"
            "}"
            "QPushButton#titleBarCloseBtn {"
            "  background: transparent;"
            "  border: none;"
            "  color: palette(window-text);"
            "  font-size: 13px;"
            "}"
            "QPushButton#titleBarCloseBtn:hover {"
            "  background-color: #C42B1C;"
            "  color: #FFFFFF;"
            "}"
            "QPushButton#titleBarCloseBtn:pressed {"
            "  background-color: #A12315;"
            "  color: #FFFFFF;"
            "}"

            /* Theme selector */
            "QComboBox#themeBox {"
            "  border: 1px solid palette(mid);"
            "  border-radius: 4px;"
            "  padding: 0 8px;"
            "  font-size: 12px;"
            "  min-width: 80px;"
            "}"

            /* Main buttons */
            "QPushButton#selectBtn, QPushButton#convertBtn {"
            "  border-radius: 4px;"
            "  padding: 0 20px;"
            "  font-size: 13px;"
            "}"

            /* QProgressBar */
            "QProgressBar {"
            "  border: none;"
            "  border-radius: 3px;"
            "  background-color: palette(mid);"
            "  height: 6px;"
            "}"
            "QProgressBar::chunk {"
            "  border-radius: 3px;"
            "  background-color: palette(highlight);"
            "}"

            /* Context menu */
            "QMenu {"
            "  border: 1px solid palette(mid);"
            "  border-radius: 4px;"
            "  padding: 4px 0;"
            "}"
            "QMenu::item {"
            "  padding: 5px 28px 5px 16px;"
            "  font-size: 12px;"
            "}"
            "QMenu::item:selected {"
            "  background-color: palette(highlight);"
            "  color: palette(highlighted-text);"
            "}"
            "QMenu::separator {"
            "  height: 1px;"
            "  background: palette(mid);"
            "  margin: 3px 8px;"
            "}"
        );

    // ── Light ─────────────────────────────────────────────────────────────────
    case Theme::Light:
        return QStringLiteral(
            /* Title bar */
            "QWidget#titleBar {"
            "  background-color: #F0F0F0;"
            "  border-bottom: 1px solid #D0D0D0;"
            "}"
            "QLabel#titleLabel {"
            "  color: #1A1A1A;"
            "  font-size: 13px;"
            "  font-weight: 600;"
            "  padding-left: 2px;"
            "}"

            /* Title bar buttons */
            "QPushButton#titleBarBtn {"
            "  background: transparent;"
            "  border: none;"
            "  color: #1A1A1A;"
            "  font-size: 13px;"
            "}"
            "QPushButton#titleBarBtn:hover {"
            "  background-color: #DADADA;"
            "}"
            "QPushButton#titleBarBtn:pressed {"
            "  background-color: #C8C8C8;"
            "}"
            "QPushButton#titleBarCloseBtn {"
            "  background: transparent;"
            "  border: none;"
            "  color: #1A1A1A;"
            "  font-size: 13px;"
            "}"
            "QPushButton#titleBarCloseBtn:hover {"
            "  background-color: #C42B1C;"
            "  color: #FFFFFF;"
            "}"
            "QPushButton#titleBarCloseBtn:pressed {"
            "  background-color: #A12315;"
            "  color: #FFFFFF;"
            "}"

            /* Theme selector */
            "QComboBox#themeBox {"
            "  background-color: #FFFFFF;"
            "  border: 1px solid #C0C0C0;"
            "  border-radius: 4px;"
            "  padding: 0 8px;"
            "  font-size: 12px;"
            "  color: #1A1A1A;"
            "  min-width: 80px;"
            "}"
            "QComboBox#themeBox::drop-down { border: none; width: 20px; }"
            "QComboBox#themeBox QAbstractItemView {"
            "  background-color: #FFFFFF;"
            "  border: 1px solid #C0C0C0;"
            "  selection-background-color: #0067C0;"
            "  selection-color: #FFFFFF;"
            "}"

            /* Central widget */
            "QWidget#centralWidget { background-color: #F3F3F3; }"

            /* Select / Convert buttons */
            "QPushButton#selectBtn {"
            "  background-color: #E8E8E8;"
            "  border: 1px solid #C0C0C0;"
            "  border-radius: 4px;"
            "  color: #1A1A1A;"
            "  padding: 0 20px;"
            "  font-size: 13px;"
            "  min-width: 140px;"
            "}"
            "QPushButton#selectBtn:hover  { background-color: #D8D8D8; border-color: #A0A0A0; }"
            "QPushButton#selectBtn:pressed { background-color: #C8C8C8; }"

            "QPushButton#convertBtn {"
            "  background-color: #0067C0;"
            "  border: none;"
            "  border-radius: 4px;"
            "  color: #FFFFFF;"
            "  padding: 0 20px;"
            "  font-size: 13px;"
            "  font-weight: 600;"
            "  min-width: 140px;"
            "}"
            "QPushButton#convertBtn:hover   { background-color: #005BA5; }"
            "QPushButton#convertBtn:pressed  { background-color: #004E8C; }"
            "QPushButton#convertBtn:disabled {"
            "  background-color: #C8C8C8;"
            "  color: #909090;"
            "}"

            /* Browse buttons */
            "QPushButton {"
            "  border-radius: 4px;"
            "  font-size: 12px;"
            "}"

            /* Line edits */
            "QLineEdit {"
            "  background-color: #FFFFFF;"
            "  border: 1px solid #C0C0C0;"
            "  border-radius: 4px;"
            "  padding: 4px 8px;"
            "  font-size: 12px;"
            "  color: #1A1A1A;"
            "  selection-background-color: #0067C0;"
            "  selection-color: #FFFFFF;"
            "}"
            "QLineEdit:focus { border-color: #0067C0; }"

            /* Location mode combobox */
            "QComboBox {"
            "  background-color: #FFFFFF;"
            "  border: 1px solid #C0C0C0;"
            "  border-radius: 4px;"
            "  padding: 4px 8px;"
            "  font-size: 12px;"
            "  color: #1A1A1A;"
            "  min-width: 200px;"
            "}"
            "QComboBox::drop-down { border: none; width: 20px; }"
            "QComboBox QAbstractItemView {"
            "  background-color: #FFFFFF;"
            "  border: 1px solid #C0C0C0;"
            "  selection-background-color: #0067C0;"
            "  selection-color: #FFFFFF;"
            "}"

            /* Progress bar */
            "QProgressBar {"
            "  border: none;"
            "  border-radius: 3px;"
            "  background-color: #DCDCDC;"
            "  max-height: 6px;"
            "}"
            "QProgressBar::chunk {"
            "  border-radius: 3px;"
            "  background-color: #0067C0;"
            "}"

            /* Labels */
            "QLabel#fileLabel   { color: #444444; font-size: 12px; }"
            "QLabel#statusLabel { color: #444444; font-size: 12px; }"

            /* Scrollbar */
            "QScrollBar:vertical {"
            "  background: #EBEBEB; width: 8px; border-radius: 4px;"
            "}"
            "QScrollBar::handle:vertical {"
            "  background: #BBBBBB; border-radius: 4px; min-height: 20px;"
            "}"
            "QScrollBar::handle:vertical:hover { background: #999999; }"
            "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }"

            /* Tooltip */
            "QToolTip {"
            "  background-color: #FFFBE6;"
            "  color: #1A1A1A;"
            "  border: 1px solid #C8C8C8;"
            "  border-radius: 3px;"
            "  padding: 3px 6px;"
            "  font-size: 12px;"
            "}"

            /* Context menu */
            "QMenu {"
            "  background-color: #FFFFFF;"
            "  border: 1px solid #C0C0C0;"
            "  border-radius: 4px;"
            "  padding: 4px 0;"
            "}"
            "QMenu::item {"
            "  padding: 5px 28px 5px 16px;"
            "  font-size: 12px;"
            "  color: #1A1A1A;"
            "}"
            "QMenu::item:selected {"
            "  background-color: #0067C0;"
            "  color: #FFFFFF;"
            "  border-radius: 3px;"
            "  margin: 0 4px;"
            "}"
            "QMenu::item:disabled { color: #AAAAAA; }"
            "QMenu::separator {"
            "  height: 1px;"
            "  background: #D8D8D8;"
            "  margin: 3px 8px;"
            "}"
            "QMenu::indicator:checked {"
            "  width: 10px; height: 10px;"
            "  image: none;"
            "  background-color: #0067C0;"
            "  border-radius: 2px;"
            "  margin-left: 4px;"
            "}"
        );

    // ── Dark ──────────────────────────────────────────────────────────────────
    case Theme::Dark:
        return QStringLiteral(
            /* Title bar */
            "QWidget#titleBar {"
            "  background-color: #2D2D2D;"
            "  border-bottom: 1px solid #3A3A3A;"
            "}"
            "QLabel#titleLabel {"
            "  color: #DCDCDC;"
            "  font-size: 13px;"
            "  font-weight: 600;"
            "  padding-left: 2px;"
            "}"

            /* Title bar buttons */
            "QPushButton#titleBarBtn {"
            "  background: transparent;"
            "  border: none;"
            "  color: #DCDCDC;"
            "  font-size: 13px;"
            "}"
            "QPushButton#titleBarBtn:hover {"
            "  background-color: #404040;"
            "}"
            "QPushButton#titleBarBtn:pressed {"
            "  background-color: #505050;"
            "}"
            "QPushButton#titleBarCloseBtn {"
            "  background: transparent;"
            "  border: none;"
            "  color: #DCDCDC;"
            "  font-size: 13px;"
            "}"
            "QPushButton#titleBarCloseBtn:hover {"
            "  background-color: #C42B1C;"
            "  color: #FFFFFF;"
            "}"
            "QPushButton#titleBarCloseBtn:pressed {"
            "  background-color: #A12315;"
            "  color: #FFFFFF;"
            "}"

            /* Theme selector */
            "QComboBox#themeBox {"
            "  background-color: #3C3C3C;"
            "  border: 1px solid #555555;"
            "  border-radius: 4px;"
            "  padding: 0 8px;"
            "  font-size: 12px;"
            "  color: #DCDCDC;"
            "  min-width: 80px;"
            "}"
            "QComboBox#themeBox::drop-down { border: none; width: 20px; }"
            "QComboBox#themeBox QAbstractItemView {"
            "  background-color: #3C3C3C;"
            "  border: 1px solid #555555;"
            "  selection-background-color: #0078D4;"
            "  selection-color: #FFFFFF;"
            "}"

            /* Central widget */
            "QWidget#centralWidget { background-color: #1E1E1E; }"

            /* Select / Convert buttons */
            "QPushButton#selectBtn {"
            "  background-color: #3C3C3C;"
            "  border: 1px solid #555555;"
            "  border-radius: 4px;"
            "  color: #DCDCDC;"
            "  padding: 0 20px;"
            "  font-size: 13px;"
            "  min-width: 140px;"
            "}"
            "QPushButton#selectBtn:hover  { background-color: #484848; border-color: #666666; }"
            "QPushButton#selectBtn:pressed { background-color: #525252; }"

            "QPushButton#convertBtn {"
            "  background-color: #0078D4;"
            "  border: none;"
            "  border-radius: 4px;"
            "  color: #FFFFFF;"
            "  padding: 0 20px;"
            "  font-size: 13px;"
            "  font-weight: 600;"
            "  min-width: 140px;"
            "}"
            "QPushButton#convertBtn:hover   { background-color: #006BBD; }"
            "QPushButton#convertBtn:pressed  { background-color: #005FA6; }"
            "QPushButton#convertBtn:disabled {"
            "  background-color: #3A3A3A;"
            "  color: #606060;"
            "}"

            /* Browse buttons */
            "QPushButton {"
            "  border-radius: 4px;"
            "  font-size: 12px;"
            "}"

            /* Line edits */
            "QLineEdit {"
            "  background-color: #252526;"
            "  border: 1px solid #3F3F3F;"
            "  border-radius: 4px;"
            "  padding: 4px 8px;"
            "  font-size: 12px;"
            "  color: #DCDCDC;"
            "  selection-background-color: #0078D4;"
            "  selection-color: #FFFFFF;"
            "}"
            "QLineEdit:focus { border-color: #0078D4; }"

            /* Location mode combobox */
            "QComboBox {"
            "  background-color: #252526;"
            "  border: 1px solid #3F3F3F;"
            "  border-radius: 4px;"
            "  padding: 4px 8px;"
            "  font-size: 12px;"
            "  color: #DCDCDC;"
            "  min-width: 200px;"
            "}"
            "QComboBox::drop-down { border: none; width: 20px; }"
            "QComboBox QAbstractItemView {"
            "  background-color: #252526;"
            "  border: 1px solid #3F3F3F;"
            "  selection-background-color: #0078D4;"
            "  selection-color: #FFFFFF;"
            "}"

            /* Progress bar */
            "QProgressBar {"
            "  border: none;"
            "  border-radius: 3px;"
            "  background-color: #3A3A3A;"
            "  max-height: 6px;"
            "}"
            "QProgressBar::chunk {"
            "  border-radius: 3px;"
            "  background-color: #0078D4;"
            "}"

            /* Labels */
            "QLabel#fileLabel   { color: #909090; font-size: 12px; }"
            "QLabel#statusLabel { color: #909090; font-size: 12px; }"

            /* Scrollbar */
            "QScrollBar:vertical {"
            "  background: #2D2D30; width: 8px; border-radius: 4px;"
            "}"
            "QScrollBar::handle:vertical {"
            "  background: #555555; border-radius: 4px; min-height: 20px;"
            "}"
            "QScrollBar::handle:vertical:hover { background: #707070; }"
            "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }"

            /* Tooltip */
            "QToolTip {"
            "  background-color: #2D2D30;"
            "  color: #DCDCDC;"
            "  border: 1px solid #444444;"
            "  border-radius: 3px;"
            "  padding: 3px 6px;"
            "  font-size: 12px;"
            "}"

            /* Context menu */
            "QMenu {"
            "  background-color: #2D2D2D;"
            "  border: 1px solid #444444;"
            "  border-radius: 4px;"
            "  padding: 4px 0;"
            "}"
            "QMenu::item {"
            "  padding: 5px 28px 5px 16px;"
            "  font-size: 12px;"
            "  color: #DCDCDC;"
            "}"
            "QMenu::item:selected {"
            "  background-color: #0078D4;"
            "  color: #FFFFFF;"
            "  border-radius: 3px;"
            "  margin: 0 4px;"
            "}"
            "QMenu::item:disabled { color: #555555; }"
            "QMenu::separator {"
            "  height: 1px;"
            "  background: #444444;"
            "  margin: 3px 8px;"
            "}"
            "QMenu::indicator:checked {"
            "  width: 10px; height: 10px;"
            "  image: none;"
            "  background-color: #0078D4;"
            "  border-radius: 2px;"
            "  margin-left: 4px;"
            "}"
        );

    // ── AMOLED ────────────────────────────────────────────────────────────────
    case Theme::AMOLED:
        return QStringLiteral(
            /* Title bar */
            "QWidget#titleBar {"
            "  background-color: #000000;"
            "  border-bottom: 1px solid #1C1C1C;"
            "}"
            "QLabel#titleLabel {"
            "  color: #E0E0E0;"
            "  font-size: 13px;"
            "  font-weight: 600;"
            "  padding-left: 2px;"
            "}"

            /* Title bar buttons */
            "QPushButton#titleBarBtn {"
            "  background: transparent;"
            "  border: none;"
            "  color: #E0E0E0;"
            "  font-size: 13px;"
            "}"
            "QPushButton#titleBarBtn:hover {"
            "  background-color: #1A1A1A;"
            "}"
            "QPushButton#titleBarBtn:pressed {"
            "  background-color: #262626;"
            "}"
            "QPushButton#titleBarCloseBtn {"
            "  background: transparent;"
            "  border: none;"
            "  color: #E0E0E0;"
            "  font-size: 13px;"
            "}"
            "QPushButton#titleBarCloseBtn:hover {"
            "  background-color: #C42B1C;"
            "  color: #FFFFFF;"
            "}"
            "QPushButton#titleBarCloseBtn:pressed {"
            "  background-color: #A12315;"
            "  color: #FFFFFF;"
            "}"

            /* Theme selector */
            "QComboBox#themeBox {"
            "  background-color: #0A0A0A;"
            "  border: 1px solid #2A2A2A;"
            "  border-radius: 4px;"
            "  padding: 0 8px;"
            "  font-size: 12px;"
            "  color: #E0E0E0;"
            "  min-width: 80px;"
            "}"
            "QComboBox#themeBox::drop-down { border: none; width: 20px; }"
            "QComboBox#themeBox QAbstractItemView {"
            "  background-color: #0A0A0A;"
            "  border: 1px solid #2A2A2A;"
            "  selection-background-color: #0078D4;"
            "  selection-color: #FFFFFF;"
            "}"

            /* Central widget */
            "QWidget#centralWidget { background-color: #000000; }"

            /* Select / Convert buttons */
            "QPushButton#selectBtn {"
            "  background-color: #1A1A1A;"
            "  border: 1px solid #2A2A2A;"
            "  border-radius: 4px;"
            "  color: #E0E0E0;"
            "  padding: 0 20px;"
            "  font-size: 13px;"
            "  min-width: 140px;"
            "}"
            "QPushButton#selectBtn:hover  { background-color: #242424; border-color: #3A3A3A; }"
            "QPushButton#selectBtn:pressed { background-color: #2E2E2E; }"

            "QPushButton#convertBtn {"
            "  background-color: #0078D4;"
            "  border: none;"
            "  border-radius: 4px;"
            "  color: #FFFFFF;"
            "  padding: 0 20px;"
            "  font-size: 13px;"
            "  font-weight: 600;"
            "  min-width: 140px;"
            "}"
            "QPushButton#convertBtn:hover   { background-color: #006BBD; }"
            "QPushButton#convertBtn:pressed  { background-color: #005FA6; }"
            "QPushButton#convertBtn:disabled {"
            "  background-color: #181818;"
            "  color: #3A3A3A;"
            "}"

            /* Browse buttons */
            "QPushButton {"
            "  border-radius: 4px;"
            "  font-size: 12px;"
            "}"

            /* Line edits */
            "QLineEdit {"
            "  background-color: #0A0A0A;"
            "  border: 1px solid #222222;"
            "  border-radius: 4px;"
            "  padding: 4px 8px;"
            "  font-size: 12px;"
            "  color: #E0E0E0;"
            "  selection-background-color: #0078D4;"
            "  selection-color: #FFFFFF;"
            "}"
            "QLineEdit:focus { border-color: #0078D4; }"

            /* Location mode combobox */
            "QComboBox {"
            "  background-color: #0A0A0A;"
            "  border: 1px solid #222222;"
            "  border-radius: 4px;"
            "  padding: 4px 8px;"
            "  font-size: 12px;"
            "  color: #E0E0E0;"
            "  min-width: 200px;"
            "}"
            "QComboBox::drop-down { border: none; width: 20px; }"
            "QComboBox QAbstractItemView {"
            "  background-color: #0A0A0A;"
            "  border: 1px solid #222222;"
            "  selection-background-color: #0078D4;"
            "  selection-color: #FFFFFF;"
            "}"

            /* Progress bar */
            "QProgressBar {"
            "  border: none;"
            "  border-radius: 3px;"
            "  background-color: #1A1A1A;"
            "  max-height: 6px;"
            "}"
            "QProgressBar::chunk {"
            "  border-radius: 3px;"
            "  background-color: #0078D4;"
            "}"

            /* Labels */
            "QLabel#fileLabel   { color: #606060; font-size: 12px; }"
            "QLabel#statusLabel { color: #606060; font-size: 12px; }"

            /* Scrollbar */
            "QScrollBar:vertical {"
            "  background: #000000; width: 8px; border-radius: 4px;"
            "}"
            "QScrollBar::handle:vertical {"
            "  background: #333333; border-radius: 4px; min-height: 20px;"
            "}"
            "QScrollBar::handle:vertical:hover { background: #484848; }"
            "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }"

            /* Tooltip */
            "QToolTip {"
            "  background-color: #0A0A0A;"
            "  color: #E0E0E0;"
            "  border: 1px solid #222222;"
            "  border-radius: 3px;"
            "  padding: 3px 6px;"
            "  font-size: 12px;"
            "}"

            /* Context menu */
            "QMenu {"
            "  background-color: #0A0A0A;"
            "  border: 1px solid #222222;"
            "  border-radius: 4px;"
            "  padding: 4px 0;"
            "}"
            "QMenu::item {"
            "  padding: 5px 28px 5px 16px;"
            "  font-size: 12px;"
            "  color: #E0E0E0;"
            "}"
            "QMenu::item:selected {"
            "  background-color: #0078D4;"
            "  color: #FFFFFF;"
            "  border-radius: 3px;"
            "  margin: 0 4px;"
            "}"
            "QMenu::item:disabled { color: #333333; }"
            "QMenu::separator {"
            "  height: 1px;"
            "  background: #1C1C1C;"
            "  margin: 3px 8px;"
            "}"
            "QMenu::indicator:checked {"
            "  width: 10px; height: 10px;"
            "  image: none;"
            "  background-color: #0078D4;"
            "  border-radius: 2px;"
            "  margin-left: 4px;"
            "}"
        );

    default:
        return {};
    }
}

} // namespace LE
