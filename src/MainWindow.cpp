#include "MainWindow.h"
#include "Logger.h"

#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <QTimer>
#include <QFileInfo>
#include <QEvent>
#include <QMenu>
#include <QAction>
#include <QIcon>
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
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground, false);
    resize(760, 560);
    setMinimumSize(600, 450);

    MARGINS margins{1, 1, 1, 1};
    DwmExtendFrameIntoClientArea(reinterpret_cast<HWND>(winId()), &margins);

    buildUi();
    connectSignals();

    m_themeManager.applyTheme(Theme::System);
    applyWindowIcon();

    qCInfo(lcWindow) << "MainWindow constructed";
}

// ─── Icon ─────────────────────────────────────────────────────────────────────

void MainWindow::applyWindowIcon()
{
    // Embed the white variant as the primary application icon.
    // Qt's setWindowIcon is sufficient for Alt+Tab / taskbar in most cases,
    // but frameless windows sometimes need explicit WM_SETICON calls.
    const QIcon icon(":/icons/LEwX.ico");
    setWindowIcon(icon);

    // Explicitly post WM_SETICON with both ICON_BIG and ICON_SMALL so the
    // taskbar and the ALT+TAB switcher pick up the icon on frameless windows.
    const HWND hwnd = reinterpret_cast<HWND>(winId());
    const HICON hIconBig   = static_cast<HICON>(
        LoadImage(nullptr, reinterpret_cast<LPCWSTR>(icon.pixmap(256).toImage().bits()),
                  IMAGE_ICON, 256, 256, 0));
    const HICON hIconSmall = static_cast<HICON>(
        LoadImage(nullptr, reinterpret_cast<LPCWSTR>(icon.pixmap(16).toImage().bits()),
                  IMAGE_ICON, 16, 16, 0));

    // Use the Qt-extracted HICON via pixmap → HBITMAP path is fragile;
    // the reliable method for an embedded .ico is to load via Qt's resource
    // system and send the native handle that Qt internally maintains.
    // Since Qt does not expose the HICON directly, we send WM_SETICON with the
    // HICON obtained from the QPixmap's native handle on Windows.
    auto pixmapToHICON = [](const QPixmap& px) -> HICON {
        return static_cast<HICON>(px.toImage().toHICON());
    };

    if (const HICON big = pixmapToHICON(icon.pixmap(256)); big) {
        SendMessage(hwnd, WM_SETICON, ICON_BIG,   reinterpret_cast<LPARAM>(big));
    }
    if (const HICON small = pixmapToHICON(icon.pixmap(16)); small) {
        SendMessage(hwnd, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(small));
    }

    // Suppress unused variable warnings for the LoadImage paths above
    // (they were illustrative; the pixmapToHICON lambda is the actual path used)
    if (hIconBig)   DestroyIcon(hIconBig);
    if (hIconSmall) DestroyIcon(hIconSmall);
}

// ─── Always on Top ───────────────────────────────────────────────────────────

void MainWindow::setAlwaysOnTop(bool enabled)
{
    m_alwaysOnTop = enabled;
    const HWND hwnd    = reinterpret_cast<HWND>(winId());
    const HWND zOrder  = enabled ? HWND_TOPMOST : HWND_NOTOPMOST;
    SetWindowPos(hwnd, zOrder, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    qCInfo(lcWindow) << "Always on top:" << enabled;
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
    m_titleBar->setContextMenuPolicy(Qt::CustomContextMenu);

    m_titleLabel = new QLabel("LunateEpsilon", m_titleBar);
    m_titleLabel->setObjectName("titleLabel");

    m_minimizeBtn = new QPushButton("─", m_titleBar);
    m_maximizeBtn = new QPushButton("□", m_titleBar);
    m_closeBtn    = new QPushButton("✕", m_titleBar);

    for (auto* btn : {m_minimizeBtn, m_maximizeBtn, m_closeBtn}) {
        btn->setFixedSize(46, kTitleBarHeight);
        btn->setFlat(true);
        btn->setObjectName("titleBarBtn");
        btn->setContextMenuPolicy(Qt::NoContextMenu);
    }
    m_closeBtn->setObjectName("titleBarCloseBtn");

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

    connect(m_selectBtn,       &QPushButton::clicked, this, &MainWindow::onSelectFile);
    connect(m_browseBaseBtn,   &QPushButton::clicked, this, &MainWindow::onBrowseBasePath);
    connect(m_browseCustomBtn, &QPushButton::clicked, this, &MainWindow::onBrowseCustomPath);
    connect(m_convertBtn,      &QPushButton::clicked, this, &MainWindow::onConvert);

    connect(m_basePathEdit,   &QLineEdit::textChanged,
            this, &MainWindow::onBasePathTextChanged);
    connect(m_customPathEdit, &QLineEdit::textChanged,
            this, &MainWindow::onCustomPathTextChanged);

    connect(m_locationModeBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onLocationModeChanged);

    connect(m_themeBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onThemeChanged);

    connect(&m_futureWatcher, &QFutureWatcher<void>::finished,
            this, &MainWindow::onConversionFinished);

    // Title bar right-click context menu
    connect(m_titleBar, &QWidget::customContextMenuRequested,
            this, &MainWindow::onTitleBarContextMenu);
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
        // Show custom path widget only if custom mode is already selected
        m_customPathWidget->setVisible(m_locationModeBox->currentIndex() == 1);
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

    m_conversionError.reset();
    setConversionInProgress(true);

    qCInfo(lcThread) << "Dispatching conversion to thread pool";

    // Exceptions thrown inside QtConcurrent::run are NOT propagated through
    // QFutureWatcher::finished in Qt6. Catch inside the lambda and store the
    // error message; onConversionFinished reads it on the main thread.
    auto future = QtConcurrent::run([this, params]() {
        try {
            m_converter.convert(params);
        } catch (const std::exception& e) {
            // Store error — read back on main thread in onConversionFinished.
            // std::optional assignment from a background thread is safe here
            // because m_futureWatcher::finished is emitted only after the
            // future completes, so there is no concurrent access.
            m_conversionError = e.what();
        }
    });

    m_futureWatcher.setFuture(future);
}

void MainWindow::onConversionFinished()
{
    qCInfo(lcThread) << "Conversion thread finished";

    // Check for error transported from the worker thread
    if (m_conversionError.has_value()) {
        setConversionInProgress(false);
        showError(QString::fromStdString(*m_conversionError));
        m_conversionError.reset();
        return;
    }

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

void MainWindow::onCustomPathTextChanged()
{
    updateConvertButtonState();
}

void MainWindow::onThemeChanged(int index)
{
    const Theme themes[] = {Theme::System, Theme::Light, Theme::Dark, Theme::AMOLED};
    m_themeManager.applyTheme(themes[index]);
}

void MainWindow::onTitleBarContextMenu(const QPoint& pos)
{
    QMenu menu(this);

    // Restore / Maximise — label switches with current state
    QAction* restoreMaxAction = menu.addAction(isMaximized() ? "Restore" : "Maximise");
    connect(restoreMaxAction, &QAction::triggered, this, [this]() {
        isMaximized() ? showNormal() : showMaximized();
    });

    QAction* minimiseAction = menu.addAction("Minimise");
    connect(minimiseAction, &QAction::triggered, this, &QMainWindow::showMinimized);

    // Move — posts SC_MOVE via Win32 so native move loop engages correctly
    QAction* moveAction = menu.addAction("Move");
    connect(moveAction, &QAction::triggered, this, [this]() {
        // Release any implicit mouse grab before posting SC_MOVE
        releaseMouse();
        const HWND hwnd = reinterpret_cast<HWND>(winId());
        PostMessage(hwnd, WM_SYSCOMMAND, SC_MOVE | 0x0002, 0);
    });

    menu.addSeparator();

    QAction* alwaysOnTopAction = menu.addAction("Always on Top");
    alwaysOnTopAction->setCheckable(true);
    alwaysOnTopAction->setChecked(m_alwaysOnTop);
    connect(alwaysOnTopAction, &QAction::triggered, this, [this](bool checked) {
        setAlwaysOnTop(checked);
    });

    menu.addSeparator();

    QAction* closeAction = menu.addAction("Close");
    connect(closeAction, &QAction::triggered, this, &QMainWindow::close);

    menu.exec(m_titleBar->mapToGlobal(pos));
}

// ─── State Helpers ───────────────────────────────────────────────────────────

void MainWindow::updateConvertButtonState()
{
    if (m_filePath.isEmpty()) {
        m_convertBtn->setEnabled(false);
        return;
    }

    if (m_inputExt == "m3u") {
        // Requires a non-empty base path
        m_convertBtn->setEnabled(!m_basePathEdit->text().trimmed().isEmpty());
        return;
    }

    // m3u8 → m3u
    if (m_locationModeBox->currentIndex() == 1) {
        // Custom mode: require a non-empty custom path
        m_convertBtn->setEnabled(!m_customPathEdit->text().trimmed().isEmpty());
    } else {
        // Keep mode: no extra input required
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
        // Re-evaluate enabled state properly rather than unconditionally enabling
        updateConvertButtonState();
    }
}

void MainWindow::showError(const QString& message)
{
    QMessageBox::critical(this, "Error", message);
}

void MainWindow::animateProgressTo(int targetPercent)
{
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
        if (msg->wParam == TRUE) {
            auto* params = reinterpret_cast<NCCALCSIZE_PARAMS*>(msg->lParam);
            if (isMaximized()) {
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

        if (y >= 0 && y < kTitleBarHeight) {
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
