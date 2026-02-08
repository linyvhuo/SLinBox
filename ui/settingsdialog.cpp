#include "settingsdialog.h"
#include "ui_settingsdialog.h"
#include <QFileDialog>
#include <QStandardPaths>
#include <QDir>
#include <QMessageBox>
#include <QDebug>
#include <QDesktopServices>
#include <QUrl>
#include "useragreementdialog.h"
#include "userguidedialog.h"


SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SettingsDialog)
    , m_config(ConfigManager::instance())
    , m_logger(Logger::instance())
{
    ui->setupUi(this);
    initUI();
    loadSettings();
    
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &SettingsDialog::onAccepted);
    connect(ui->btnBrowseLogPath, &QPushButton::clicked, this, &SettingsDialog::onBrowseLogPath);
    connect(ui->btnBrowseDataPath, &QPushButton::clicked, this, &SettingsDialog::onBrowseDataPath);
    connect(ui->btnClearData, &QPushButton::clicked, this, &SettingsDialog::onClearData);
    connect(ui->btnSoftwareInfo, &QPushButton::clicked, this, &SettingsDialog::onSoftwareInfoClicked);
    connect(ui->btnUserGuide, &QPushButton::clicked, this, &SettingsDialog::onUserGuideClicked);
    connect(ui->btnUserAgreement, &QPushButton::clicked, this, &SettingsDialog::onUserAgreementClicked);
    connect(ui->btnHomePage, &QPushButton::clicked, this, &SettingsDialog::onHomePageClicked);
    connect(ui->btnUpdate, &QPushButton::clicked, this, &SettingsDialog::onUpdateClicked);
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

void SettingsDialog::initUI()
{
    QString documentsPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    QString logPath = documentsPath + "/SLinBox/Logs";
    QString dataPath = documentsPath + "/SLinBox/Data";
    
    QDir().mkpath(logPath);
    QDir().mkpath(dataPath);
    
    ui->editLogPath->setText(logPath);
    ui->editDataPath->setText(dataPath);
    
    connect(ui->comboTheme, &QComboBox::currentTextChanged, this, [this](const QString &theme) {
        m_config->setValue("General", "Theme", theme);
        m_config->sync();
        emit settingsApplied();
    });
    
    connect(ui->comboLanguage, &QComboBox::currentTextChanged, this, [this](const QString &language) {
        m_config->setValue("General", "Language", language);
        m_config->sync();
        emit settingsApplied();
    });
    
    connect(ui->comboCloseAction, &QComboBox::currentTextChanged, this, [this](const QString &action) {
        m_config->setValue("General", "CloseAction", action);
        m_config->sync();
    });
    
    connect(ui->checkStartup, &QCheckBox::stateChanged, this, [this](int state) {
        m_config->setValue("General", "StartupShow", state == Qt::Checked);
        m_config->sync();
    });
    
    connect(ui->checkAutoReply, &QCheckBox::stateChanged, this, [this](int state) {
        m_config->setValue("WeChatAuto", "AutoReply", state == Qt::Checked);
        m_config->sync();
    });
    
    connect(ui->keySequenceScreenshot, &QKeySequenceEdit::keySequenceChanged, this, [this](const QKeySequence &keySequence) {
        m_config->setValue("ScreenshotOCR", "ScreenshotHotkey", keySequence.toString());
        m_config->sync();
    });
    
    connect(ui->keySequenceStop, &QKeySequenceEdit::keySequenceChanged, this, [this](const QKeySequence &keySequence) {
        m_config->setValue("WeChatAuto", "StopHotkey", keySequence.toString());
        m_config->sync();
    });
    
    connect(ui->keySequenceOcr, &QKeySequenceEdit::keySequenceChanged, this, [this](const QKeySequence &keySequence) {
        m_config->setValue("ScreenshotOCR", "OcrHotkey", keySequence.toString());
        m_config->sync();
    });
    
    connect(ui->keySequenceClipboardOcr, &QKeySequenceEdit::keySequenceChanged, this, [this](const QKeySequence &keySequence) {
        m_config->setValue("ScreenshotOCR", "ClipboardOcrHotkey", keySequence.toString());
        m_config->sync();
    });
    
    connect(ui->editOCRPath, &QLineEdit::textChanged, this, [this](const QString &path) {
        m_config->setValue("ScreenshotOCR", "OCRPath", path);
        m_config->sync();
    });
    
    connect(ui->btnBrowseOCRPath, &QPushButton::clicked, this, &SettingsDialog::onBrowseOCRPath);
    
    connect(ui->comboLogLevel, &QComboBox::currentTextChanged, this, [this](const QString &level) {
        m_config->setValue("Logger", "LogLevel", level);
        m_config->sync();
        emit settingsApplied();
    });
}

void SettingsDialog::onBrowseOCRPath()
{
    QString filePath = QFileDialog::getOpenFileName(this, "选择Umi-OCR程序", "", 
        "可执行文件 (*.exe);;所有文件 (*)");
    
    if (!filePath.isEmpty()) {
        ui->editOCRPath->setText(filePath);
        m_config->setValue("ScreenshotOCR", "OCRPath", filePath);
        m_config->sync();
    }
}

QString SettingsDialog::getStopHotkey() const
{
    return ui->keySequenceStop->keySequence().toString();
}

QString SettingsDialog::getOcrHotkey() const
{
    return ui->keySequenceOcr->keySequence().toString();
}

QString SettingsDialog::getClipboardOcrHotkey() const
{
    return ui->keySequenceClipboardOcr->keySequence().toString();
}

QString SettingsDialog::getOCRPath() const
{
    return ui->editOCRPath->text();
}

void SettingsDialog::loadSettings()
{
    QString theme = m_config->getValue("General", "Theme", "浅色").toString();
    int themeIndex = ui->comboTheme->findText(theme);
    if (themeIndex >= 0) {
        ui->comboTheme->setCurrentIndex(themeIndex);
    }
    
    QString language = m_config->getValue("General", "Language", "简体中文").toString();
    int languageIndex = ui->comboLanguage->findText(language);
    if (languageIndex >= 0) {
        ui->comboLanguage->setCurrentIndex(languageIndex);
    }
    
    QString closeAction = m_config->getValue("General", "CloseAction", "退出").toString();
    int closeActionIndex = ui->comboCloseAction->findText(closeAction);
    if (closeActionIndex >= 0) {
        ui->comboCloseAction->setCurrentIndex(closeActionIndex);
    }
    
    bool startupShow = m_config->getValue("General", "StartupShow", true).toBool();
    ui->checkStartup->setChecked(startupShow);
    
    bool autoReply = m_config->getValue("WeChatAuto", "AutoReply", false).toBool();
    ui->checkAutoReply->setChecked(autoReply);
    
    QString screenshotHotkey = m_config->getValue("ScreenshotOCR", "ScreenshotHotkey", "Ctrl+Shift+A").toString();
    ui->keySequenceScreenshot->setKeySequence(QKeySequence(screenshotHotkey));
    
    QString stopHotkey = m_config->getValue("WeChatAuto", "StopHotkey", "Ctrl+Shift+X").toString();
    ui->keySequenceStop->setKeySequence(QKeySequence(stopHotkey));
    
    QString ocrHotkey = m_config->getValue("ScreenshotOCR", "OcrHotkey", "Ctrl+Shift+O").toString();
    ui->keySequenceOcr->setKeySequence(QKeySequence(ocrHotkey));
    
    QString clipboardOcrHotkey = m_config->getValue("ScreenshotOCR", "ClipboardOcrHotkey", "Ctrl+Shift+C").toString();
    ui->keySequenceClipboardOcr->setKeySequence(QKeySequence(clipboardOcrHotkey));
    
    QString ocrEngine = m_config->getValue("ScreenshotOCR", "OCREngine", "Umi-OCR").toString();
    
    QString ocrPath = m_config->getValue("ScreenshotOCR", "OCRPath", "").toString();
    ui->editOCRPath->setText(ocrPath);
    
    QString logLevel = m_config->getValue("Logger", "LogLevel", "信息").toString();
    int logLevelIndex = ui->comboLogLevel->findText(logLevel);
    if (logLevelIndex >= 0) {
        ui->comboLogLevel->setCurrentIndex(logLevelIndex);
    }
    
    QString logPath = m_config->getValue("Logger", "LogPath", ui->editLogPath->text()).toString();
    ui->editLogPath->setText(logPath);
    
    int maxLogSize = m_config->getValue("Logger", "MaxLogSize", 10).toInt();
    ui->spinMaxLogSize->setValue(maxLogSize);
    
    QString dataPath = m_config->getValue("General", "DataPath", ui->editDataPath->text()).toString();
    ui->editDataPath->setText(dataPath);
}

void SettingsDialog::saveSettings()
{
    m_config->setValue("General", "Theme", ui->comboTheme->currentText());
    m_config->setValue("General", "Language", ui->comboLanguage->currentText());
    m_config->setValue("General", "CloseAction", ui->comboCloseAction->currentText());
    m_config->setValue("General", "StartupShow", ui->checkStartup->isChecked());
    m_config->setValue("WeChatAuto", "AutoReply", ui->checkAutoReply->isChecked());
    m_config->setValue("WeChatAuto", "StopHotkey", ui->keySequenceStop->keySequence().toString());
    m_config->setValue("ScreenshotOCR", "ScreenshotHotkey", ui->keySequenceScreenshot->keySequence().toString());
    m_config->setValue("ScreenshotOCR", "OcrHotkey", ui->keySequenceOcr->keySequence().toString());
    m_config->setValue("ScreenshotOCR", "ClipboardOcrHotkey", ui->keySequenceClipboardOcr->keySequence().toString());
    m_config->setValue("ScreenshotOCR", "OCRPath", ui->editOCRPath->text());
    m_config->setValue("Logger", "LogLevel", ui->comboLogLevel->currentText());
    m_config->setValue("Logger", "LogPath", ui->editLogPath->text());
    m_config->setValue("Logger", "MaxLogSize", ui->spinMaxLogSize->value());
    m_config->setValue("General", "DataPath", ui->editDataPath->text());
    m_config->sync();
    
    m_logger->info("SettingsDialog", "设置已保存");
    
    emit settingsApplied();
}

void SettingsDialog::onAccepted()
{
    saveSettings();
    accept();
}

void SettingsDialog::onBrowseLogPath()
{
    QString path = QFileDialog::getExistingDirectory(this, "选择日志路径", ui->editLogPath->text());
    if (!path.isEmpty()) {
        ui->editLogPath->setText(path);
    }
}

void SettingsDialog::onBrowseDataPath()
{
    QString path = QFileDialog::getExistingDirectory(this, "选择数据路径", ui->editDataPath->text());
    if (!path.isEmpty()) {
        ui->editDataPath->setText(path);
    }
}

void SettingsDialog::onClearData()
{
    QMessageBox::StandardButton reply = QMessageBox::question(this, "确认", "确定要清除所有数据吗？此操作不可撤销！", 
        QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        QString dataPath = ui->editDataPath->text();
        QDir dir(dataPath);
        if (dir.exists()) {
            dir.removeRecursively();
            QMessageBox::information(this, "成功", "数据已清除！");
            m_logger->info("SettingsDialog", "数据已清除");
        }
    }
}

QString SettingsDialog::getTheme() const
{
    return ui->comboTheme->currentText();
}

QString SettingsDialog::getLanguage() const
{
    return ui->comboLanguage->currentText();
}

QString SettingsDialog::getCloseAction() const
{
    return ui->comboCloseAction->currentText();
}

bool SettingsDialog::getStartupShow() const
{
    return ui->checkStartup->isChecked();
}

bool SettingsDialog::getAutoReply() const
{
    return ui->checkAutoReply->isChecked();
}

QString SettingsDialog::getScreenshotHotkey() const
{
    return ui->keySequenceScreenshot->keySequence().toString();
}

QString SettingsDialog::getLogLevel() const
{
    return ui->comboLogLevel->currentText();
}

QString SettingsDialog::getLogPath() const
{
    return ui->editLogPath->text();
}

int SettingsDialog::getMaxLogSize() const
{
    return ui->spinMaxLogSize->value();
}

QString SettingsDialog::getDataPath() const
{
    return ui->editDataPath->text();
}

void SettingsDialog::onSoftwareInfoClicked()
{
    QMessageBox::information(this, "软件说明", 
        "SLinBox 智能工具箱是一款功能强大的多功能工具软件。\n\n"
        "主要功能包括：\n"
        "• 微信自动化：支持自动回复、消息发送等功能\n"
        "• 截图+OCR：快速截图并识别文字\n"
        "• 串口通信：支持多种波特率和数据格式\n"
        "• 进制转换：支持二进制、八进制、十进制、十六进制转换\n"
        "• CRC校验：支持多种CRC算法\n"
        "• 时序图：支持SPI、I2C、USART、UART、I2S等协议\n"
        "• 延时计算：计算代码执行延时\n"
        "• ASCII查询：查询ASCII码表\n"
        "• 校验和计算：计算数据的校验和\n"
        "• 数据手册：查看常用芯片数据手册\n\n"
        "感谢您使用 SLinBox 智能工具箱！");
}

void SettingsDialog::onUserGuideClicked()
{
    UserGuideDialog dialog(this);
    dialog.exec();
}

void SettingsDialog::onUserAgreementClicked()
{
    UserGuideDialog dialog(this);
    dialog.exec();
}

void SettingsDialog::onHomePageClicked()
{
    QDesktopServices::openUrl(QUrl("https://github.com/slinbox/slinbox"));
}

void SettingsDialog::onUpdateClicked()
{
    QMessageBox::information(this, "检查更新", 
        "当前已是最新版本！\n\n"
        "版本：1.0.0\n"
        "发布日期：2024-01-01\n\n"
        "感谢您使用 SLinBox 智能工具箱！");
}
