#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTabWidget>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QTextEdit>
#include <QTimer>
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
#include "settingsdialog.h"
#include "ui/useragreementdialog.h"
#include "ui/userguidedialog.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void loadConfig();
    void saveConfig();
    void applyTheme(const QString &themeName);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void onMinimizeClicked();
    void onMaximizeClicked();
    void onCloseClicked();
    void onTrayIconActivated(QSystemTrayIcon::ActivationReason reason);
    void onShowMainWindow();
    void onHideToTray();
    void onExitApplication();
    void onLogAdded(const QString &logMessage, LogLevel level);
    void onClearLog();
    void onExportLog();
    void onToggleLog();
    void onLogLevelChanged(int index);

    void onWeChatAutoTab();
    void onScreenshotOCRTab();
    void onSerialPortTab();
    void onBaseConvertTab();
    void onCRCCheckTab();
    void onTimingDiagramTab();
    void onDelayCalcTab();
    void onASCIIQueryTab();
    void onChecksumTab();
    void onDataSheetTab();
    void onSettingsClicked();
    void onUserAgreement();
    void onUserGuide();
    void onSettingsApplied();
    
private:
    void initUI();
    void initTitleBar();
    void initTabWidget();
    void initLogPanel();
    void initSystemTray();
    void updateTitleBarButtons();
    int getResizeEdge(const QPoint &pos);

    Ui::MainWindow *ui;

    QWidget *m_titleBar;
    QLabel *m_titleLabel;
    QPushButton *m_minimizeBtn;
    QPushButton *m_maximizeBtn;
    QPushButton *m_closeBtn;
    QPushButton *m_settingsBtn;

    QTabWidget *m_tabWidget;
    QWidget *m_logPanel;
    QTextEdit *m_logText;
    QPushButton *m_expandLogBtn;
    QComboBox *m_logLevelCombo;
    QPushButton *m_clearLogBtn;
    QPushButton *m_exportLogBtn;

    QSystemTrayIcon *m_trayIcon;
    QMenu *m_trayMenu;
    QAction *m_showAction;
    QAction *m_hideAction;
    QAction *m_settingsAction;
    QAction *m_exitAction;
    
    WeChatAutoModule *m_weChatAutoModule;
    ScreenshotOCRModule *m_screenshotOCRModule;
    SerialPortModule *m_serialPortModule;
    BaseConvertModule *m_baseConvertModule;
    CRCCheckModule *m_crcCheckModule;
    TimingDiagramModule *m_timingDiagramModule;
    DelayCalcModule *m_delayCalcModule;
    ASCIIQueryModule *m_asciiQueryModule;
    ChecksumModule *m_checksumModule;
    DataSheetModule *m_dataSheetModule;
    UserAgreementDialog *m_userAgreementDialog;
    
    QPoint m_dragPosition;
    bool m_isMaximized;
    bool m_isResizing;
    int m_resizeEdge;
    QPoint m_resizeStartPos;
    QSize m_resizeStartSize;
    
    ConfigManager *m_config;
    Logger *m_logger;
};

#endif // MAINWINDOW_H