#pragma once

#include "ThemeManager.h"
#include "Converter.h"

#include <QMainWindow>
#include <QFutureWatcher>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QProgressBar>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <optional>
#include <string>

namespace LE {

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override = default;

protected:
    bool nativeEvent(const QByteArray& eventType, void* message, qintptr* result) override;
    void changeEvent(QEvent* event) override;

private slots:
    void onSelectFile();
    void onBrowseBasePath();
    void onBrowseCustomPath();
    void onConvert();
    void onConversionFinished();
    void onLocationModeChanged(int index);
    void onBasePathTextChanged();
    void onCustomPathTextChanged();
    void onThemeChanged(int index);
    void onTitleBarContextMenu(const QPoint& pos);

private:
    void buildUi();
    void buildTitleBar();
    void buildCentralContent();
    void connectSignals();

    void updateConvertButtonState();
    void setConversionInProgress(bool inProgress);
    void showError(const QString& message);
    void animateProgressTo(int targetPercent);
    void applyWindowIcon();
    void setAlwaysOnTop(bool enabled);

    // Title bar controls
    QWidget*     m_titleBar     = nullptr;
    QLabel*      m_titleLabel   = nullptr;
    QPushButton* m_minimizeBtn  = nullptr;
    QPushButton* m_maximizeBtn  = nullptr;
    QPushButton* m_closeBtn     = nullptr;

    // Main UI controls
    QPushButton*  m_selectBtn        = nullptr;
    QLabel*       m_fileLabel        = nullptr;
    QWidget*      m_basePathWidget   = nullptr;
    QLineEdit*    m_basePathEdit     = nullptr;
    QPushButton*  m_browseBaseBtn    = nullptr;
    QComboBox*    m_locationModeBox  = nullptr;
    QWidget*      m_customPathWidget = nullptr;
    QLineEdit*    m_customPathEdit   = nullptr;
    QPushButton*  m_browseCustomBtn  = nullptr;
    QPushButton*  m_convertBtn       = nullptr;
    QProgressBar* m_progressBar      = nullptr;
    QLabel*       m_statusLabel      = nullptr;
    QComboBox*    m_themeBox         = nullptr;

    // State
    QString      m_filePath;
    QString      m_inputExt;
    bool         m_alwaysOnTop = false;

    // Conversion error transported from worker thread to UI thread
    std::optional<std::string> m_conversionError;

    ThemeManager          m_themeManager;
    Converter             m_converter;
    QFutureWatcher<void>  m_futureWatcher;
};

} // namespace LE
