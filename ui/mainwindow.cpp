#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QApplication>
#include <QScreen>
#include <QFileDialog>
#include <QMessageBox>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QCloseEvent>
#include <QResizeEvent>
#include <QSettings>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QDebug>
#include <QScrollBar>
#include <QStyle>
#include "core/configmanager.h"
#include "core/logger.h"
#include "modules/wechatauto/wechatautomodule.h"
#include "modules/screenshotocr/screenshotocrmodule.h"
#include "modules/serialport/serialportmodule.h"
#include "modules/baseconvert/baseconvertmodule.h"
#include "modules/crccheck/crccheckmodule.h"
#include "modules/timingdiagram/timingdiagrammodule.h"
#include "modules/delaycalc/delaycalcmodule.h"
#include "modules/asciiquery/asciiquerymodule.h"
#include "modules/checksum/checksummodule.h"
#include "modules/datasheet/datasheetmodule.h"
#include "ui/settingsdialog.h"
#include "ui/useragreementdialog.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_isMaximized(false)
    , m_isResizing(false)
    , m_resizeEdge(0)
    , m_config(ConfigManager::instance())
    , m_logger(Logger::instance())
    , m_userAgreementDialog(nullptr)
{
    ui->setupUi(this);
    
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint);
    setAttribute(Qt::WA_TranslucentBackground);
    
    initUI();
    initTitleBar();
    initTabWidget();
    initLogPanel();
    initSystemTray();
    loadConfig();
    
    connect(m_logger, &Logger::logAdded, this, &MainWindow::onLogAdded);
    
    m_logger->info("MainWindow", "SLinBox 智能工具箱启动成功");
}

MainWindow::~MainWindow()
{
    if (m_userAgreementDialog) {
        delete m_userAgreementDialog;
        m_userAgreementDialog = nullptr;
    }
    saveConfig();
    delete ui;
}

void MainWindow::initUI()
{
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(centralWidget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    
    setCentralWidget(centralWidget);
    
    setAttribute(Qt::WA_TranslucentBackground);
    
    QPalette palette = this->palette();
    palette.setColor(QPalette::Window, Qt::transparent);
    setPalette(palette);
    
    setStyleSheet("QMainWindow { background: transparent; border-radius: 10px; }");
    
    m_expandLogBtn = new QPushButton("▲ 日志", centralWidget);
    m_expandLogBtn->setFixedHeight(30);
    m_expandLogBtn->setStyleSheet("QPushButton { background-color: #f5f5f5; border-top: 1px solid #ddd; padding: 5px 10px; } QPushButton:hover { background-color: #e0e0e0; }");
    m_expandLogBtn->hide();
    connect(m_expandLogBtn, &QPushButton::clicked, this, &MainWindow::onToggleLog);
}

void MainWindow::initTitleBar()
{
    m_titleBar = new QWidget(this);
    m_titleBar->setFixedHeight(40);
    m_titleBar->setStyleSheet("background-color: #2c3e50;");
    
    QHBoxLayout *titleLayout = new QHBoxLayout(m_titleBar);
    titleLayout->setContentsMargins(10, 0, 10, 0);
    
    m_titleLabel = new QLabel("SLinBox 智能工具箱", m_titleBar);
    m_titleLabel->setStyleSheet("color: white; font-size: 14px; font-weight: bold;");
    
    QWidget *buttonWidget = new QWidget(m_titleBar);
    QHBoxLayout *buttonLayout = new QHBoxLayout(buttonWidget);
    buttonLayout->setContentsMargins(0, 0, 0, 0);
    buttonLayout->setSpacing(0);
    
    m_minimizeBtn = new QPushButton("➖", buttonWidget);
    m_minimizeBtn->setFixedSize(40, 30);
    m_minimizeBtn->setStyleSheet("QPushButton { background-color: transparent; color: white; border: none; font-size: 16px; } QPushButton:hover { background-color: #34495e; }");
    connect(m_minimizeBtn, &QPushButton::clicked, this, &MainWindow::onMinimizeClicked);
    
    m_maximizeBtn = new QPushButton("⏹️", buttonWidget);
    m_maximizeBtn->setFixedSize(40, 30);
    m_maximizeBtn->setStyleSheet("QPushButton { background-color: transparent; color: white; border: none; font-size: 16px; } QPushButton:hover { background-color: #34495e; }");
    connect(m_maximizeBtn, &QPushButton::clicked, this, &MainWindow::onMaximizeClicked);
    
    m_settingsBtn = new QPushButton("⚙", buttonWidget);
    m_settingsBtn->setFixedSize(40, 30);
    m_settingsBtn->setStyleSheet("QPushButton { background-color: transparent; color: white; border: none; font-size: 16px; } QPushButton:hover { background-color: #34495e; }");
    connect(m_settingsBtn, &QPushButton::clicked, this, &MainWindow::onSettingsClicked);

    m_closeBtn = new QPushButton("❎", buttonWidget);
    m_closeBtn->setFixedSize(40, 30);
    m_closeBtn->setStyleSheet("QPushButton { background-color: transparent; color: white; border: none; font-size: 18px; } QPushButton:hover { background-color: #e74c3c; }");
    connect(m_closeBtn, &QPushButton::clicked, this, &MainWindow::onCloseClicked);
    
    buttonLayout->addWidget(m_settingsBtn);
    buttonLayout->addWidget(m_minimizeBtn);
    buttonLayout->addWidget(m_maximizeBtn);
    buttonLayout->addWidget(m_closeBtn);
    
    buttonWidget->setLayout(buttonLayout);
    
    titleLayout->addWidget(m_titleLabel);
    titleLayout->addStretch();
    titleLayout->addWidget(buttonWidget);
    
    m_titleBar->setLayout(titleLayout);
}

void MainWindow::initTabWidget()
{
    m_tabWidget = new QTabWidget(this);
    m_tabWidget->setStyleSheet("QTabWidget::pane { border: none; } QTabBar::tab { background: #ecf0f1; color: #333; padding: 8px 16px; } QTabBar::tab:selected { background: #3498db; color: white; }");
    m_tabWidget->tabBar()->setMovable(true);
    m_tabWidget->tabBar()->setTabsClosable(false);
    
    m_weChatAutoModule = new WeChatAutoModule(this);
    m_screenshotOCRModule = new ScreenshotOCRModule(this);
    m_serialPortModule = new SerialPortModule(this);
    m_baseConvertModule = new BaseConvertModule(this);
    m_crcCheckModule = new CRCCheckModule(this);
    m_timingDiagramModule = new TimingDiagramModule(this);
    m_delayCalcModule = new DelayCalcModule(this);
    m_asciiQueryModule = new ASCIIQueryModule(this);
    m_checksumModule = new ChecksumModule(this);
    m_dataSheetModule = new DataSheetModule(this);
    
    m_tabWidget->addTab(m_weChatAutoModule, "自动化");
    m_tabWidget->addTab(m_screenshotOCRModule, "截图+OCR");
    m_tabWidget->addTab(m_serialPortModule, "串口调试");
    m_tabWidget->addTab(m_baseConvertModule, "进制转换");
    m_tabWidget->addTab(m_crcCheckModule, "CRC校验");
    m_tabWidget->addTab(m_timingDiagramModule, "时序图");
    m_tabWidget->addTab(m_delayCalcModule, "延时计算");
    m_tabWidget->addTab(m_asciiQueryModule, "ASCII查询");
    m_tabWidget->addTab(m_checksumModule, "校验和");
    m_tabWidget->addTab(m_dataSheetModule, "数据手册");
    
    connect(m_tabWidget, &QTabWidget::currentChanged, this, [this](int index) {
        switch (index) {
            case 0: onWeChatAutoTab(); break;
            case 1: onScreenshotOCRTab(); break;
            case 2: onSerialPortTab(); break;
            case 3: onBaseConvertTab(); break;
            case 4: onCRCCheckTab(); break;
            case 5: onTimingDiagramTab(); break;
            case 6: onDelayCalcTab(); break;
            case 7: onASCIIQueryTab(); break;
            case 8: onChecksumTab(); break;
            case 9: onDataSheetTab(); break;
        }
    });
}
void MainWindow::initLogPanel()
{
    // 初始化日志面板主控件，避免重复创建导致内存泄漏
    if (m_logPanel) {
        delete m_logPanel;
        m_logPanel = nullptr;
    }

    m_logPanel = new QWidget(this);
    m_logPanel->setFixedHeight(200);
    m_logPanel->setStyleSheet("background-color: #f5f5f5; border-top: 1px solid #ddd;");

    // 1. 创建顶层垂直布局（主布局），不指定父对象，避免自动设置
    QVBoxLayout *mainLogLayout = new QVBoxLayout();
    mainLogLayout->setContentsMargins(0, 0, 0, 0); // 主布局取消外边距，由子布局控制

    // 2. 创建日志控制栏的水平布局，同样不指定父对象
    QHBoxLayout *logLayout = new QHBoxLayout();
    logLayout->setContentsMargins(10, 10, 10, 10); // 控制栏的内边距

    // 折叠按钮
    QPushButton *collapseBtn = new QPushButton("▼", m_logPanel);
    collapseBtn->setFixedSize(30, 25);
    collapseBtn->setStyleSheet("QPushButton { padding: 2px; }");
    connect(collapseBtn, &QPushButton::clicked, this, &MainWindow::onToggleLog);

    // 日志级别标签和下拉框
    QLabel *levelLabel = new QLabel("日志级别:", m_logPanel);
    // 初始化下拉框，避免重复创建
    if (m_logLevelCombo) {
        delete m_logLevelCombo;
        m_logLevelCombo = nullptr;
    }
    m_logLevelCombo = new QComboBox(m_logPanel);
    m_logLevelCombo->addItem("调试");
    m_logLevelCombo->addItem("信息");
    m_logLevelCombo->addItem("警告");
    m_logLevelCombo->addItem("错误");
    m_logLevelCombo->setCurrentIndex(1);
    connect(m_logLevelCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onLogLevelChanged);

    // 清空日志按钮
    if (m_clearLogBtn) {
        delete m_clearLogBtn;
        m_clearLogBtn = nullptr;
    }
    m_clearLogBtn = new QPushButton("清空", m_logPanel);
    m_clearLogBtn->setStyleSheet("QPushButton { padding: 5px 10px; }");
    connect(m_clearLogBtn, &QPushButton::clicked, this, &MainWindow::onClearLog);

    // 导出日志按钮
    if (m_exportLogBtn) {
        delete m_exportLogBtn;
        m_exportLogBtn = nullptr;
    }
    m_exportLogBtn = new QPushButton("导出", m_logPanel);
    m_exportLogBtn->setStyleSheet("QPushButton { padding: 5px 10px; }");
    connect(m_exportLogBtn, &QPushButton::clicked, this, &MainWindow::onExportLog);

    // 将控件添加到水平布局
    logLayout->addWidget(collapseBtn);
    logLayout->addWidget(levelLabel);
    logLayout->addWidget(m_logLevelCombo);
    logLayout->addStretch(); // 拉伸项，将清空/导出按钮推到右侧
    logLayout->addWidget(m_clearLogBtn);
    logLayout->addWidget(m_exportLogBtn);

    // 日志文本显示框
    if (m_logText) {
        delete m_logText;
        m_logText = nullptr;
    }
    m_logText = new QTextEdit(m_logPanel);
    m_logText->setReadOnly(true);
    m_logText->setStyleSheet("QTextEdit { background: white; border: 1px solid #ccc; padding: 5px; }");

    // 3. 将水平布局和文本框添加到垂直主布局
    mainLogLayout->addLayout(logLayout); // 嵌套水平布局
    mainLogLayout->addWidget(m_logText); // 添加日志文本框

    // 4. 最后将主布局设置给日志面板（唯一一次 setLayout）
    m_logPanel->setLayout(mainLogLayout);
}

void MainWindow::initSystemTray()
{
    m_trayIcon = new QSystemTrayIcon(this);
    
    // 尝试加载图标，如果失败则使用默认图标
    QIcon icon(":/icons/app_icon.ico");
    if (icon.isNull()) {
        qWarning() << "无法加载托盘图标，使用默认图标";
        icon = style()->standardIcon(QStyle::SP_ComputerIcon);
    }
    m_trayIcon->setIcon(icon);
    
    QMenu *trayMenu = new QMenu(this);
    m_showAction = new QAction("显示主窗口", this);
    m_hideAction = new QAction("隐藏到托盘", this);
    m_settingsAction = new QAction("设置", this);
    m_exitAction = new QAction("退出", this);
    
    connect(m_showAction, &QAction::triggered, this, &MainWindow::onShowMainWindow);
    connect(m_hideAction, &QAction::triggered, this, &MainWindow::onHideToTray);
    connect(m_settingsAction, &QAction::triggered, this, &MainWindow::onSettingsClicked);
    connect(m_exitAction, &QAction::triggered, this, &MainWindow::onExitApplication);
    
    trayMenu->addAction(m_showAction);
    trayMenu->addAction(m_hideAction);
    trayMenu->addSeparator();
    trayMenu->addAction(m_settingsAction);
    trayMenu->addSeparator();
    trayMenu->addAction(m_exitAction);
    
    m_trayIcon->setContextMenu(trayMenu);
    
    connect(m_trayIcon, &QSystemTrayIcon::activated, this, &MainWindow::onTrayIconActivated);
    
    m_trayIcon->show();
}

void MainWindow::loadConfig()
{
    QString theme = m_config->getValue("General", "Theme", "浅色").toString();
    applyTheme(theme);
    
    bool userAgreed = m_config->getValue("General", "UserAgreed", false).toBool();
    if (!userAgreed) {
        m_userAgreementDialog = new UserAgreementDialog(this);
        m_userAgreementDialog->setModal(true);
        m_userAgreementDialog->exec();
        delete m_userAgreementDialog;
        m_userAgreementDialog = nullptr;
    }
    
    QVBoxLayout *layout = qobject_cast<QVBoxLayout*>(centralWidget()->layout());
    if (layout) {
        layout->addWidget(m_titleBar);
        layout->addWidget(m_tabWidget);
        layout->addWidget(m_logPanel);
        layout->addWidget(m_expandLogBtn);
    }
}

void MainWindow::saveConfig()
{
    m_config->sync();
}

void MainWindow::onSettingsApplied()
{
    QString theme = m_config->getValue("General", "Theme", "浅色").toString();
    applyTheme(theme);
    
    m_screenshotOCRModule->loadConfig();
    
    m_logger->info("MainWindow", "设置已应用，模块配置已重新加载");
}

void MainWindow::applyTheme(const QString &themeName)
{
    QString styleSheet;
    
    if (themeName == "浅色") {
        styleSheet = "QTabWidget::pane { border: 1px solid #ddd; } "
                     "QTabBar::tab { background: #ecf0f1; color: #333; padding: 8px 16px; } "
                     "QTabBar::tab:selected { background: #3498db; color: white; } "
                     "QWidget { background-color: #ffffff; color: #333333; } "
                     "QTextEdit { background-color: #ffffff; color: #333333; } "
                     "QLineEdit { background-color: #ffffff; color: #333333; border: 1px solid #ccc; padding: 5px; } "
                     "QComboBox { background-color: #ffffff; color: #333333; border: 1px solid #ccc; padding: 5px; } "
                     "QPushButton { background-color: #3498db; color: white; border: none; padding: 8px 16px; } "
                     "QPushButton:hover { background-color: #2980b9; } "
                     "QGroupBox { border: 1px solid #ccc; border-radius: 5px; margin-top: 10px; padding: 10px; } "
                     "QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 5px; } "
                     "QLabel { color: #333333; } "
                     "QCheckBox { color: #333333; } "
                     "QSpinBox { background-color: #ffffff; color: #333333; border: 1px solid #ccc; padding: 5px; } "
                     "QTableWidget { background-color: #ffffff; color: #333333; gridline-color: #e0e0e0; } "
                     "QHeaderView::section { background-color: #f0f0f0; color: #333333; padding: 5px; border: none; } "
                     "QHeaderView::section:hover { background-color: #e0e0e0; }";
    } else if (themeName == "深色") {
        styleSheet = "QTabWidget::pane { border: 1px solid #444; } "
                     "QTabBar::tab { background: #2c3e50; color: #ecf0f1; padding: 8px 16px; } "
                     "QTabBar::tab:selected { background: #34495e; color: white; } "
                     "QWidget { background-color: #1a1a1a; color: #ecf0f1; } "
                     "QTextEdit { background-color: #2c3e50; color: #ecf0f1; } "
                     "QLineEdit { background-color: #2c3e50; color: #ecf0f1; border: 1px solid #444; padding: 5px; } "
                     "QComboBox { background-color: #2c3e50; color: #ecf0f1; border: 1px solid #444; padding: 5px; } "
                     "QPushButton { background-color: #34495e; color: white; border: none; padding: 8px 16px; } "
                     "QPushButton:hover { background-color: #2c3e50; } "
                     "QGroupBox { border: 1px solid #444; border-radius: 5px; margin-top: 10px; padding: 10px; } "
                     "QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 5px; } "
                     "QLabel { color: #ecf0f1; } "
                     "QCheckBox { color: #ecf0f1; } "
                     "QSpinBox { background-color: #2c3e50; color: #ecf0f1; border: 1px solid #444; padding: 5px; } "
                     "QTableWidget { background-color: #1a1a1a; color: #ecf0f1; gridline-color: #333; } "
                     "QHeaderView::section { background-color: #2c3e50; color: #ecf0f1; padding: 5px; border: none; } "
                     "QHeaderView::section:hover { background-color: #34495e; }";
    } else if (themeName == "科技") {
        styleSheet = "QTabWidget::pane { border: 1px solid #00ff00; } "
                     "QTabBar::tab { background: #0a0e27; color: #00ff00; padding: 8px 16px; } "
                     "QTabBar::tab:selected { background: #00ff00; color: #0a0e27; } "
                     "QWidget { background-color: #0a0e27; color: #00ff00; } "
                     "QTextEdit { background-color: #0a0e27; color: #00ff00; } "
                     "QLineEdit { background-color: #0a0e27; color: #00ff00; border: 1px solid #00ff00; padding: 5px; } "
                     "QComboBox { background-color: #0a0e27; color: #00ff00; border: 1px solid #00ff00; padding: 5px; } "
                     "QPushButton { background-color: #00ff00; color: #0a0e27; border: none; padding: 8px 16px; } "
                     "QPushButton:hover { background-color: #00cc00; } "
                     "QGroupBox { border: 1px solid #00ff00; border-radius: 5px; margin-top: 10px; padding: 10px; } "
                     "QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 5px; } "
                     "QLabel { color: #00ff00; } "
                     "QCheckBox { color: #00ff00; } "
                     "QSpinBox { background-color: #0a0e27; color: #00ff00; border: 1px solid #00ff00; padding: 5px; } "
                     "QTableWidget { background-color: #0a0e27; color: #00ff00; gridline-color: #008800; } "
                     "QHeaderView::section { background-color: #00ff00; color: #0a0e27; padding: 5px; border: none; } "
                     "QHeaderView::section:hover { background-color: #00cc00; }";
    } else if (themeName == "樱花") {
        styleSheet = "QTabWidget::pane { border: 1px solid #ffb6c1; } "
                     "QTabBar::tab { background: #ffe4e1; color: #d63384; padding: 8px 16px; } "
                     "QTabBar::tab:selected { background: #ffb6c1; color: white; } "
                     "QWidget { background-color: #fff5f7; color: #d63384; } "
                     "QTextEdit { background-color: #fff5f7; color: #d63384; } "
                     "QLineEdit { background-color: #fff5f7; color: #d63384; border: 1px solid #ffb6c1; padding: 5px; } "
                     "QComboBox { background-color: #fff5f7; color: #d63384; border: 1px solid #ffb6c1; padding: 5px; } "
                     "QPushButton { background-color: #ffb6c1; color: white; border: none; padding: 8px 16px; } "
                     "QPushButton:hover { background-color: #ff9eb3; } "
                     "QGroupBox { border: 1px solid #ffb6c1; border-radius: 5px; margin-top: 10px; padding: 10px; } "
                     "QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 5px; } "
                     "QLabel { color: #d63384; } "
                     "QCheckBox { color: #d63384; } "
                     "QSpinBox { background-color: #fff5f7; color: #d63384; border: 1px solid #ffb6c1; padding: 5px; } "
                     "QTableWidget { background-color: #fff5f7; color: #d63384; gridline-color: #ffd1dc; } "
                     "QHeaderView::section { background-color: #ffb6c1; color: #fff5f7; padding: 5px; border: none; } "
                     "QHeaderView::section:hover { background-color: #ff9eb3; }";
    } else if (themeName == "暖色") {
        styleSheet = "QTabWidget::pane { border: 1px solid #ff9800; } "
                     "QTabBar::tab { background: #ffe4b5; color: #d35400; padding: 8px 16px; } "
                     "QTabBar::tab:selected { background: #ff9800; color: white; } "
                     "QWidget { background-color: #fff3e0; color: #d35400; } "
                     "QTextEdit { background-color: #fff3e0; color: #d35400; } "
                     "QLineEdit { background-color: #fff3e0; color: #d35400; border: 1px solid #ff9800; padding: 5px; } "
                     "QComboBox { background-color: #fff3e0; color: #d35400; border: 1px solid #ff9800; padding: 5px; } "
                     "QPushButton { background-color: #ff9800; color: white; border: none; padding: 8px 16px; } "
                     "QPushButton:hover { background-color: #f57c00; } "
                     "QGroupBox { border: 1px solid #ff9800; border-radius: 5px; margin-top: 10px; padding: 10px; } "
                     "QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 5px; } "
                     "QLabel { color: #d35400; } "
                     "QCheckBox { color: #d35400; } "
                     "QSpinBox { background-color: #fff3e0; color: #d35400; border: 1px solid #ff9800; padding: 5px; } "
                     "QTableWidget { background-color: #fff3e0; color: #d35400; gridline-color: #ffcc00; } "
                     "QHeaderView::section { background-color: #ff9800; color: #fff3e0; padding: 5px; border: none; } "
                     "QHeaderView::section:hover { background-color: #f57c00; }";
    }
    
    m_tabWidget->setStyleSheet(styleSheet);
    m_logPanel->setStyleSheet(styleSheet);
    
    QString titleBarColor;
    if (themeName == "浅色") {
        titleBarColor = "#e0e0e0";
    } else if (themeName == "深色") {
        titleBarColor = "#2c3e50";
    } else if (themeName == "科技") {
        titleBarColor = "#0a0e27";
    } else if (themeName == "樱花") {
        titleBarColor = "#ffe4e1";
    } else if (themeName == "暖色") {
        titleBarColor = "#ffe4b5";
    }
    
    m_titleBar->setStyleSheet(QString("background-color: %1;").arg(titleBarColor));
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        if (m_titleBar && m_titleBar->underMouse()) {
            QWidget *child = m_titleBar->childAt(m_titleBar->mapFromGlobal(event->globalPos()));
            
            if (child && child != m_titleBar) {
                if (child->inherits("QPushButton") || child->inherits("QLabel") || 
                    child->inherits("QComboBox") || child->inherits("QLineEdit") ||
                    child->inherits("QSpinBox") || child->inherits("QCheckBox") ||
                    child->inherits("QRadioButton") || child->inherits("QToolButton")) {
                    event->ignore();
                    return;
                }
            }
            
            m_dragPosition = event->globalPos() - frameGeometry().topLeft();
            event->accept();
            return;
        }
        
        QWidget *child = childAt(event->pos());
        if (child && (child->inherits("QComboBox") || child->inherits("QLineEdit") || 
                       child->inherits("QTextEdit") || child->inherits("QSpinBox") ||
                       child->inherits("QCheckBox") || child->inherits("QRadioButton") ||
                       child->inherits("QListWidget") || child->inherits("QTableWidget") ||
                       child->inherits("QTreeWidget") || child->inherits("QPushButton"))) {
            event->ignore();
            return;
        }
        
        int edge = getResizeEdge(event->pos());
        if (edge != 0) {
            m_isResizing = true;
            m_resizeEdge = edge;
            m_resizeStartPos = event->globalPos();
            m_resizeStartSize = size();
            event->accept();
        } else {
            event->ignore();
        }
    }
}

int MainWindow::getResizeEdge(const QPoint &pos)
{
    const int edgeSize = 10;
    QRect rect = QMainWindow::rect();
    
    if (pos.x() <= edgeSize && pos.y() <= edgeSize) return 1;
    if (pos.x() >= rect.width() - edgeSize && pos.y() <= edgeSize) return 2;
    if (pos.x() <= edgeSize && pos.y() >= rect.height() - edgeSize) return 3;
    if (pos.x() >= rect.width() - edgeSize && pos.y() >= rect.height() - edgeSize) return 4;
    if (pos.y() <= edgeSize) return 5;
    if (pos.y() >= rect.height() - edgeSize) return 6;
    if (pos.x() <= edgeSize) return 7;
    if (pos.x() >= rect.width() - edgeSize) return 8;
    
    return 0;
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (m_isResizing) {
        QPoint delta = event->globalPos() - m_resizeStartPos;
        QSize newSize = m_resizeStartSize;
        
        if (m_resizeEdge & 1) {
            newSize.setWidth(newSize.width() - delta.x());
            newSize.setHeight(newSize.height() - delta.y());
        } else if (m_resizeEdge & 2) {
            newSize.setWidth(newSize.width() + delta.x());
            newSize.setHeight(newSize.height() - delta.y());
        } else if (m_resizeEdge & 3) {
            newSize.setWidth(newSize.width() - delta.x());
            newSize.setHeight(newSize.height() + delta.y());
        } else if (m_resizeEdge & 4) {
            newSize.setWidth(newSize.width() + delta.x());
            newSize.setHeight(newSize.height() + delta.y());
        } else if (m_resizeEdge & 5) {
            newSize.setHeight(newSize.height() - delta.y());
        } else if (m_resizeEdge & 6) {
            newSize.setHeight(newSize.height() + delta.y());
        } else if (m_resizeEdge & 7) {
            newSize.setWidth(newSize.width() - delta.x());
        } else if (m_resizeEdge & 8) {
            newSize.setWidth(newSize.width() + delta.x());
        }
        
        if (newSize.width() >= minimumWidth() && newSize.height() >= minimumHeight()) {
            resize(newSize);
        }
        event->accept();
    } else if (event->buttons() & Qt::LeftButton) {
        if (m_isMaximized) {
            return;
        }
        move(event->globalPos() - m_dragPosition);
        event->accept();
    }
}

void MainWindow::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        onMaximizeClicked();
    }
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_isResizing = false;
        m_resizeEdge = 0;
    }
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
}

void MainWindow::onMinimizeClicked()
{
    showMinimized();
}

void MainWindow::onMaximizeClicked()
{
    if (m_isMaximized) {
        showNormal();
        m_isMaximized = false;
    } else {
        showMaximized();
        m_isMaximized = true;
    }
}

void MainWindow::onCloseClicked()
{
    QString closeAction = m_config->getValue("General", "CloseAction", "退出").toString();
    
    if (closeAction == "退出") {
        onExitApplication();
    } else {
        onHideToTray();
    }
}

void MainWindow::onExitApplication()
{
    m_trayIcon->hide();
    qApp->quit();
}

void MainWindow::onUserAgreement()
{
    m_userAgreementDialog = new UserAgreementDialog(this);
    m_userAgreementDialog->setModal(true);
    m_userAgreementDialog->exec();
    delete m_userAgreementDialog;
    m_userAgreementDialog = nullptr;
}

void MainWindow::onUserGuide()
{
    UserGuideDialog dialog(this);
    dialog.exec();
}

void MainWindow::onTrayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::DoubleClick) {
        onShowMainWindow();
    }
}

void MainWindow::onShowMainWindow()
{
    showNormal();
    activateWindow();
}

void MainWindow::onHideToTray()
{
    hide();
}

void MainWindow::onLogAdded(const QString &logMessage, LogLevel level)
{
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    QString levelText;
    
    switch (level) {
        case LogLevel::DEBUG:
            levelText = "[DEBUG]";
            break;
        case LogLevel::INFO:
            levelText = "[INFO]";
            break;
        case LogLevel::WARN:
            levelText = "[WARNING]";
            break;
        case LogLevel::ERROR_LEVEL:
            levelText = "[ERROR]";
            break;
    }
    
    QString fullMessage = QString("%1 %2 %3").arg(timestamp, levelText, logMessage);
    m_logText->append(fullMessage);
    
    QScrollBar *scrollBar = m_logText->verticalScrollBar();
    scrollBar->setValue(scrollBar->maximum());
}

void MainWindow::onClearLog()
{
    m_logText->clear();
    m_logger->info("MainWindow", "日志已清空");
}

void MainWindow::onToggleLog()
{
    if (m_logPanel->isVisible()) {
        m_logPanel->hide();
        m_expandLogBtn->show();
        m_expandLogBtn->setText("▼ 日志");
    } else {
        m_logPanel->show();
        m_expandLogBtn->hide();
    }
}

void MainWindow::onExportLog()
{
    QString filePath = QFileDialog::getSaveFileName(this, "导出日志", "", "日志文件 (*.log);;所有文件 (*)");
    
    if (!filePath.isEmpty()) {
        QFile file(filePath);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            out << m_logText->toPlainText();
            file.close();
            
            QMessageBox::information(this, "成功", "日志导出成功！");
            m_logger->info("MainWindow", QString("日志已导出到: %1").arg(filePath));
        } else {
            QMessageBox::warning(this, "失败", "日志导出失败！");
            m_logger->error("MainWindow", "日志导出失败");
        }
    }
}

void MainWindow::onLogLevelChanged(int index)
{
    LogLevel level = static_cast<LogLevel>(index);
    m_logger->setLogLevel(level);
    m_logger->info("MainWindow", QString("日志级别已设置为: %1").arg(index));
}

void MainWindow::onWeChatAutoTab()
{
    m_logger->info("MainWindow", "切换到企业微信自动化模块");
}

void MainWindow::onScreenshotOCRTab()
{
    m_logger->info("MainWindow", "切换到截图+OCR模块");
}

void MainWindow::onSerialPortTab()
{
    m_logger->info("MainWindow", "切换到串口调试模块");
}

void MainWindow::onBaseConvertTab()
{
    m_logger->info("MainWindow", "切换到进制转换模块");
}

void MainWindow::onCRCCheckTab()
{
    m_logger->info("MainWindow", "切换到CRC校验模块");
}

void MainWindow::onTimingDiagramTab()
{
    m_logger->info("MainWindow", "切换到时序图模块");
}

void MainWindow::onDelayCalcTab()
{
    m_logger->info("MainWindow", "切换到延时计算模块");
}

void MainWindow::onASCIIQueryTab()
{
    m_logger->info("MainWindow", "切换到ASCII查询模块");
}

void MainWindow::onChecksumTab()
{
    m_logger->info("MainWindow", "切换到校验和模块");
}

void MainWindow::onDataSheetTab()
{
    m_logger->info("MainWindow", "切换到数据手册模块");
}

void MainWindow::onSettingsClicked()
{
    SettingsDialog dialog(this);
    connect(&dialog, &SettingsDialog::settingsApplied, this, &MainWindow::onSettingsApplied);
    dialog.exec();
    
    m_logger->info("MainWindow", "设置已应用");
}
