#include "MainWindow.h"
#include "Logger.h"

#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <QTimer>
#include <QFileInfo>
#include <QEvent>
#include <QtConcurrent/QtConcurrent>

#include <windows.h>
#include <dwmapi.h>
#include <windowsx.h>  // GET_X_LPARAM, GET_Y_LPARAM

Q_LOGGING_CATEGORY(lcWindow, "le.window")
Q_LOGGING_CATEGORY(lcThread, "le.thread")

namespace LE {

static constexpr int kResizeBorderWidth = 8;
static constexpr int kTitleBarHeight    = 36;

// ─── Constructor ─────────────────────────────────────────────────────────────

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowFlags(Qt::Window);
    setAttribute(Qt::WA_TranslucentBackground, false);
    resize(760, 560);
    setMinimumSize(600, 450);

    // Extend DWM frame into client area to restore shadow
    MARGINS margins{1, 1, 1, 1};
    DwmExtendFrameIntoClientArea(reinterpret_cast<HWND>(winId()), &margins);

    buildUi();
    connectSignals();

    // Apply system theme by default
    m_themeManager.applyTheme(Theme::System);

    qCInfo(lcWindow) << "MainWindow constructed";
}

// ─── UI Construction ─────────────────────────────────────────────────────────

void MainWindow::buildUi()
{
    buildTitleBar();
    buildCentralContent();
}

void MainWindow::buildTitleBar()
{
    m_titleBar = new QWidget(this);
    m_titleBar->setObjectName("titleBar");
    m_titleBar->setFixedHeight(kTitleBarHeight);

    m_titleLabel = new QLabel("LunatedEpsilon", m_titleBar);
    m_titleLabel->setObjectName("titleLabel");

    m_minimizeBtn = new QPushButton("─", m_titleBar);
    m_maximizeBtn = new QPushButton("□", m_titleBar);
    m_closeBtn    = new QPushButton("✕", m_titleBar);

    for (auto* btn : {m_minimizeBtn, m_maximizeBtn, m_closeBtn}) {
        btn->setFixedSize(46, kTitleBarHeight);
        btn->setFlat(true);
        btn->setObjectName("titleBarBtn");
    }
    m_closeBtn->setObjectName("titleBarCloseBtn");

    // Theme selector in title bar
    m_themeBox = new QComboBox(m_titleBar);
    m_themeBox->addItems({"System", "Light", "Dark", "AMOLED"});
    m_themeBox->setFixedHeight(24);
    m_themeBox->setObjectName("themeBox");

    auto* layout = new QHBoxLayout(m_titleBar);
    layout->setContentsMargins(12, 0, 0, 0);
    layout->setSpacing(0);

    layout->addWidget(m_titleLabel);
    layout->addStretch();
    layout->addWidget(m_themeBox);
    layout->addSpacing(12);
    layout->addWidget(m_minimizeBtn);
    layout->addWidget(m_maximizeBtn);
    layout->addWidget(m_closeBtn);
}

void MainWindow::buildCentralContent()
{
    auto* central = new QWidget(this);
    central->setObjectName("centralWidget");

    auto* rootLayout = new QVBoxLayout(central);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);
    rootLayout->addWidget(m_titleBar);

    auto* contentWidget = new QWidget(central);
    auto* contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setAlignment(Qt::AlignCenter);
    contentLayout->setSpacing(10);
    contentLayout->setContentsMargins(40, 20, 40, 30);

    // Select File
    m_selectBtn = new QPushButton("Select File", contentWidget);
    m_selectBtn->setObjectName("selectBtn");
    m_selectBtn->setFixedHeight(34);

    m_fileLabel = new QLabel("No file selected", contentWidget);
    m_fileLabel->setObjectName("fileLabel");
    m_fileLabel->setAlignment(Qt::AlignCenter);

    // Base path row (M3U → M3U8)
    m_basePathWidget = new QWidget(contentWidget);
    m_basePathEdit   = new QLineEdit(m_basePathWidget);
    m_basePathEdit->setPlaceholderText("Base folder path");
    m_browseBaseBtn  = new QPushButton("Browse", m_basePathWidget);
    m_browseBaseBtn->setFixedWidth(80);
    {
        auto* row = new QHBoxLayout(m_basePathWidget);
        row->setContentsMargins(0, 0, 0, 0);
        row->addWidget(m_basePathEdit);
        row->addWidget(m_browseBaseBtn);
    }
    m_basePathWidget->setVisible(false);

    // Location mode (M3U8 → M3U)
    m_locationModeBox = new QComboBox(contentWidget);
    m_locationModeBox->addItems({"Keep original path", "Use custom base path"});
    m_locationModeBox->setVisible(false);

    // Custom path row
    m_customPathWidget = new QWidget(contentWidget);
    m_customPathEdit   = new QLineEdit(m_customPathWidget);
    m_customPathEdit->setPlaceholderText("Custom base path");
    m_browseCustomBtn  = new QPushButton("Browse", m_customPathWidget);
    m_browseCustomBtn->setFixedWidth(80);
    {
        auto* row = new QHBoxLayout(m_customPathWidget);
        row->setContentsMargins(0, 0, 0, 0);
        row->addWidget(m_customPathEdit);
        row->addWidget(m_browseCustomBtn);
    }
    m_customPathWidget->setVisible(false);

    // Convert button
    m_convertBtn = new QPushButton("Convert", contentWidget);
    m_convertBtn->setObjectName("convertBtn");
    m_convertBtn->setFixedHeight(36);
    m_convertBtn->setEnabled(false);

    // Progress bar
    m_progressBar = new QProgressBar(contentWidget);
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);
    m_progressBar->setFixedWidth(320);
    m_progressBar->setVisible(false);
    m_progressBar->setTextVisible(false);

    // Status label
    m_statusLabel = new QLabel(contentWidget);
    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->setObjectName("statusLabel");
    m_statusLabel->setVisible(false);

    contentLayout->addWidget(m_selectBtn,       0, Qt::AlignCenter);
    contentLayout->addWidget(m_fileLabel,        0, Qt::AlignCenter);
    contentLayout->addWidget(m_basePathWidget,   0, Qt::AlignCenter);
    contentLayout->addWidget(m_locationModeBox,  0, Qt::AlignCenter);
    contentLayout->addWidget(m_customPathWidget, 0, Qt::AlignCenter);
    contentLayout->addWidget(m_convertBtn,       0, Qt::AlignCenter);
    contentLayout->addWidget(m_progressBar,      0, Qt::AlignCenter);
    contentLayout->addWidget(m_statusLabel,      0, Qt::AlignCenter);

    rootLayout->addWidget(contentWidget, 1);

    setCentralWidget(central);
}

// ─── Signals ─────────────────────────────────────────────────────────────────

void MainWindow::connectSignals()
{
    connect(m_closeBtn,    &QPushButton::clicked, this, &QMainWindow::close);
    connect(m_minimizeBtn, &QPushButton::clicked, this, &QMainWindow::showMinimized);
    connect(m_maximizeBtn, &QPushButton::clicked, this, [this]() {
        isMaximized() ? showNormal() : showMaximized();
    });

    connect(m_selectBtn,     &QPushButton::clicked, this, &MainWindow::onSelectFile);
    connect(m_browseBaseBtn, &QPushButton::clicked, this, &MainWindow::onBrowseBasePath);
    connect(m_browseCustomBtn, &QPushButton::clicked, this, &MainWindow::onBrowseCustomPath);
    connect(m_convertBtn,    &QPushButton::clicked, this, &MainWindow::onConvert);

    connect(m_basePathEdit, &QLineEdit::textChanged, this, &MainWindow::onBasePathTextChanged);
    connect(m_locationModeBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onLocationModeChanged);

    connect(m_themeBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onThemeChanged);

    connect(&m_futureWatcher, &QFutureWatcher<void>::finished,
            this, &MainWindow::onConversionFinished);
}

// ─── Slots ───────────────────────────────────────────────────────────────────

void MainWindow::onSelectFile()
{
    const QString path = QFileDialog::getOpenFileName(
        this, "Select Playlist File", {},
        "Playlist Files (*.m3u *.m3u8)"
    );

    if (path.isEmpty()) return;

    m_filePath = path;
    m_inputExt = QFileInfo(path).suffix().toLower();

    m_fileLabel->setText(QFileInfo(path).fileName());

    if (m_inputExt == "m3u") {
        m_convertBtn->setText("Convert to .m3u8");
        m_basePathWidget->setVisible(true);
        m_locationModeBox->setVisible(false);
        m_customPathWidget->setVisible(false);
    } else {
        m_convertBtn->setText("Convert to .m3u");
        m_basePathWidget->setVisible(false);
        m_locationModeBox->setVisible(true);
        m_customPathWidget->setVisible(false);
    }

    updateConvertButtonState();
}

void MainWindow::onBrowseBasePath()
{
    const QString folder = QFileDialog::getExistingDirectory(this, "Select Base Folder");
    if (!folder.isEmpty()) {
        m_basePathEdit->setText(folder);
    }
}

void MainWindow::onBrowseCustomPath()
{
    const QString folder = QFileDialog::getExistingDirectory(this, "Select Custom Base Folder");
    if (!folder.isEmpty()) {
        m_customPathEdit->setText(folder);
    }
}

void MainWindow::onConvert()
{
    if (m_filePath.isEmpty()) return;

    const QString targetExt = (m_inputExt == "m3u") ? ".m3u8" : ".m3u";

    const QString savePath = QFileDialog::getSaveFileName(
        this, "Save Converted File", {},
        targetExt.toUpper().mid(1) + " Files (*" + targetExt + ")"
    );

    if (savePath.isEmpty()) return;

    ConversionParams params;
    params.inputPath  = m_filePath;
    params.outputPath = savePath;

    if (m_inputExt == "m3u") {
        params.basePath = m_basePathEdit->text().trimmed();
    } else {
        if (m_locationModeBox->currentIndex() == 1) {
            params.locationMode = LocationMode::Custom;
            params.basePath = m_customPathEdit->text().trimmed();
            if (params.basePath.isEmpty()) {
                showError("Custom base path is required.");
                return;
            }
        } else {
            params.locationMode = LocationMode::Keep;
        }
    }

    setConversionInProgress(true);

    qCInfo(lcThread) << "Dispatching conversion to thread pool";

    auto future = QtConcurrent::run([this, params]() {
        m_converter.convert(params);
    });

    m_futureWatcher.setFuture(future);
}

void MainWindow::onConversionFinished()
{
    qCInfo(lcThread) << "Conversion thread finished";

    try {
        m_futureWatcher.future().waitForFinished();
    } catch (const std::exception& e) {
        setConversionInProgress(false);
        showError(QString::fromStdString(e.what()));
        return;
    }

    // Animate progress bar to 100 then hide
    animateProgressTo(100);
    m_statusLabel->setText("Completed");

    QTimer::singleShot(900, this, [this]() {
        setConversionInProgress(false);
    });
}

void MainWindow::onLocationModeChanged(int index)
{
    m_customPathWidget->setVisible(index == 1);
    updateConvertButtonState();
}

void MainWindow::onBasePathTextChanged()
{
    updateConvertButtonState();
}

void MainWindow::onThemeChanged(int index)
{
    const Theme themes[] = {Theme::System, Theme::Light, Theme::Dark, Theme::AMOLED};
    m_themeManager.applyTheme(themes[index]);
}

// ─── State Helpers ───────────────────────────────────────────────────────────

void MainWindow::updateConvertButtonState()
{
    if (m_filePath.isEmpty()) {
        m_convertBtn->setEnabled(false);
        return;
    }

    if (m_inputExt == "m3u") {
        m_convertBtn->setEnabled(!m_basePathEdit->text().trimmed().isEmpty());
    } else {
        m_convertBtn->setEnabled(true);
    }
}

void MainWindow::setConversionInProgress(bool inProgress)
{
    m_convertBtn->setEnabled(!inProgress);
    m_progressBar->setVisible(inProgress);
    m_statusLabel->setVisible(inProgress);

    if (inProgress) {
        m_progressBar->setValue(0);
        m_statusLabel->setText("Processing…");
    } else {
        m_progressBar->setValue(0);
        m_statusLabel->setVisible(false);
        m_convertBtn->setEnabled(true);
    }
}

void MainWindow::showError(const QString& message)
{
    QMessageBox::critical(this, "Error", message);
}

void MainWindow::animateProgressTo(int targetPercent)
{
    // Drive a simple timer-based animation — non-blocking, purely cosmetic
    auto* timer = new QTimer(this);
    timer->setInterval(20);
    connect(timer, &QTimer::timeout, this, [this, timer, targetPercent]() {
        const int current = m_progressBar->value();
        if (current >= targetPercent) {
            timer->stop();
            timer->deleteLater();
            return;
        }
        m_progressBar->setValue(current + 2);
    });
    timer->start();
}

// ─── changeEvent ─────────────────────────────────────────────────────────────

void MainWindow::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::WindowStateChange) {
        m_maximizeBtn->setText(isMaximized() ? "❐" : "□");
    }
    QMainWindow::changeEvent(event);
}

// ─── Win32 Native Event ──────────────────────────────────────────────────────

bool MainWindow::nativeEvent(const QByteArray& eventType, void* message, qintptr* result)
{
    if (eventType != "windows_generic_MSG") {
        return QMainWindow::nativeEvent(eventType, message, result);
    }

    auto* msg = static_cast<MSG*>(message);

    switch (msg->message) {

    case WM_NCCALCSIZE: {
        // When wParam is TRUE, returning 0 removes the standard frame
        // while preserving client area maximized state handling correctly.
        if (msg->wParam == TRUE) {
            auto* params = reinterpret_cast<NCCALCSIZE_PARAMS*>(msg->lParam);
            if (isMaximized()) {
                // When maximized, Windows adds invisible borders; compensate.
                const int border = GetSystemMetrics(SM_CXSIZEFRAME) +
                                   GetSystemMetrics(SM_CXPADDEDBORDER);
                params->rgrc[0].left   += border;
                params->rgrc[0].right  -= border;
                params->rgrc[0].top    += border;
                params->rgrc[0].bottom -= border;
            }
            *result = 0;
            return true;
        }
        break;
    }

    case WM_NCHITTEST: {
        // Let DWM first evaluate — this handles snap layouts (Win11)
        LRESULT dwmResult = 0;
        if (DwmDefWindowProc(msg->hwnd, msg->message, msg->wParam, msg->lParam, &dwmResult)) {
            *result = dwmResult;
            return true;
        }

        const POINT ptScreen{GET_X_LPARAM(msg->lParam), GET_Y_LPARAM(msg->lParam)};
        RECT rcWindow;
        GetWindowRect(msg->hwnd, &rcWindow);

        const int x = ptScreen.x - rcWindow.left;
        const int y = ptScreen.y - rcWindow.top;
        const int w = rcWindow.right  - rcWindow.left;
        const int h = rcWindow.bottom - rcWindow.top;

        const bool left   = x < kResizeBorderWidth;
        const bool right  = x > w - kResizeBorderWidth;
        const bool top    = y < kResizeBorderWidth;
        const bool bottom = y > h - kResizeBorderWidth;

        if (top    && left)  { *result = HTTOPLEFT;     return true; }
        if (top    && right) { *result = HTTOPRIGHT;    return true; }
        if (bottom && left)  { *result = HTBOTTOMLEFT;  return true; }
        if (bottom && right) { *result = HTBOTTOMRIGHT; return true; }
        if (left)            { *result = HTLEFT;        return true; }
        if (right)           { *result = HTRIGHT;       return true; }
        if (top)             { *result = HTTOP;         return true; }
        if (bottom)          { *result = HTBOTTOM;      return true; }

        // Title bar drag region
        if (y >= 0 && y < kTitleBarHeight) {
            // Check whether the cursor is over a child widget (button/combobox)
            // If so, pass through to client area so buttons remain clickable.
            const QPoint localPos = mapFromGlobal(QPoint(ptScreen.x, ptScreen.y));
            QWidget* child = childAt(localPos);

            const bool overInteractive = child &&
                (qobject_cast<QPushButton*>(child) || qobject_cast<QComboBox*>(child));

            if (!overInteractive) {
                *result = HTCAPTION;
                return true;
            }
        }

        *result = HTCLIENT;
        return true;
    }

    case WM_GETMINMAXINFO: {
        // Ensure maximized window doesn't cover the taskbar
        auto* mmi = reinterpret_cast<MINMAXINFO*>(msg->lParam);
        const HMONITOR monitor = MonitorFromWindow(msg->hwnd, MONITOR_DEFAULTTONEAREST);
        MONITORINFO mi{};
        mi.cbSize = sizeof(mi);
        if (GetMonitorInfo(monitor, &mi)) {
            mmi->ptMaxPosition.x = mi.rcWork.left   - mi.rcMonitor.left;
            mmi->ptMaxPosition.y = mi.rcWork.top    - mi.rcMonitor.top;
            mmi->ptMaxSize.x     = mi.rcWork.right  - mi.rcWork.left;
            mmi->ptMaxSize.y     = mi.rcWork.bottom - mi.rcWork.top;
        }
        *result = 0;
        return true;
    }

    default:
        break;
    }

    return QMainWindow::nativeEvent(eventType, message, result);
}

} // namespace LE