#pragma once

#include <QObject>
#include <QPalette>
#include <QString>

namespace LE {

enum class Theme {
    System,
    Light,
    Dark,
    AMOLED
};

class ThemeManager : public QObject {
    Q_OBJECT

public:
    explicit ThemeManager(QObject* parent = nullptr);
    ~ThemeManager() override = default;

    void applyTheme(Theme theme);
    [[nodiscard]] Theme currentTheme() const noexcept { return m_current; }

signals:
    void themeChanged(Theme theme);

private:
    QPalette buildLightPalette()  const;
    QPalette buildDarkPalette()   const;
    QPalette buildAmoledPalette() const;

    QString buildStyleSheet(Theme theme) const;

    Theme m_current = Theme::System;
};

} // namespace LE
