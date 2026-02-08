#include "configmanager.h"
#include <QDir>
#include <QStandardPaths>
#include <QCoreApplication>
#include <QDebug>

ConfigManager* ConfigManager::s_instance = nullptr;

ConfigManager::ConfigManager(QObject *parent)
    : QObject(parent)
{
    ensureConfigDirectory();
    
    QString documentsPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    m_configPath = documentsPath + "/SLinBox/config.ini";
    
    m_settings = new QSettings(m_configPath, QSettings::IniFormat, this);
    m_settings->setIniCodec("UTF-8");
    
    initDefaultConfig();
}

ConfigManager::~ConfigManager()
{
    if (m_settings) {
        delete m_settings;
    }
}

ConfigManager* ConfigManager::instance()
{
    if (!s_instance) {
        s_instance = new ConfigManager();
    }
    return s_instance;
}

void ConfigManager::setValue(const QString &group, const QString &key, const QVariant &value)
{
    m_settings->beginGroup(group);
    m_settings->setValue(key, value);
    m_settings->endGroup();
    m_settings->sync();
}

QVariant ConfigManager::getValue(const QString &group, const QString &key, const QVariant &defaultValue)
{
    m_settings->beginGroup(group);
    QVariant value = m_settings->value(key, defaultValue);
    m_settings->endGroup();
    return value;
}

void ConfigManager::setGroup(const QString &group)
{
    m_settings->beginGroup(group);
}

void ConfigManager::beginGroup(const QString &group)
{
    m_settings->beginGroup(group);
}

void ConfigManager::endGroup()
{
    m_settings->endGroup();
}

void ConfigManager::sync()
{
    m_settings->sync();
}

void ConfigManager::clear()
{
    m_settings->clear();
    m_settings->sync();
}

bool ConfigManager::backup(const QString &backupPath)
{
    QFile configFile(m_configPath);
    if (configFile.exists()) {
        return configFile.copy(backupPath);
    }
    return false;
}

bool ConfigManager::restore(const QString &backupPath)
{
    QFile backupFile(backupPath);
    if (backupFile.exists()) {
        QFile::remove(m_configPath);
        return backupFile.copy(m_configPath);
    }
    return false;
}

void ConfigManager::resetToDefault()
{
    clear();
    initDefaultConfig();
}

QString ConfigManager::getConfigPath() const
{
    return m_configPath;
}

void ConfigManager::initDefaultConfig()
{
    if (!m_settings->allKeys().isEmpty()) {
        return;
    }

    setValue("General", "Theme", "Light");
    setValue("General", "Language", "zh_CN");
    setValue("General", "CloseAction", "Exit");
    setValue("General", "FloatWindowEnable", false);

    setValue("WeChatAuto", "WeChatPath", "");
    setValue("WeChatAuto", "WindowTopMost", false);
    setValue("WeChatAuto", "RecognizeThreshold", 0.8);
    setValue("WeChatAuto", "MaxRecognizeTry", 3);
    setValue("WeChatAuto", "PageLoadTimeout", 5000);
    setValue("WeChatAuto", "LoopCount", 10);
    setValue("WeChatAuto", "AnswerTimeout", 30000);
    setValue("WeChatAuto", "RoundInterval", 2000);
    setValue("WeChatAuto", "ContinueOnError", true);
    setValue("WeChatAuto", "ContinueOnTimeout", true);
    setValue("WeChatAuto", "AnswerLimitTip", "");
    setValue("WeChatAuto", "MainDisplayIndex", 0);
    setValue("WeChatAuto", "QuestionMode", "Loop");
    setValue("WeChatAuto", "QuestionLibraryPath", "");

    setValue("ScreenshotOCR", "ScreenshotHotkey", "Ctrl+Shift+S");
    setValue("ScreenshotOCR", "OcrConfidence", 0.7);
    setValue("ScreenshotOCR", "DefaultSaveFormat", "PNG");
    setValue("ScreenshotOCR", "SavePath", QStandardPaths::writableLocation(QStandardPaths::DesktopLocation));
    setValue("ScreenshotOCR", "AutoOcrAfterScreenshot", true);

    setValue("SerialPort", "PortName", "");
    setValue("SerialPort", "BaudRate", 9600);
    setValue("SerialPort", "DataBits", 8);
    setValue("SerialPort", "StopBits", 1);
    setValue("SerialPort", "Parity", "None");
    setValue("SerialPort", "FlowControl", "None");
    setValue("SerialPort", "DisplayMode", "ASCII");
    setValue("SerialPort", "AutoScroll", true);

    setValue("BaseConvert", "SourceBase", 10);
    setValue("BaseConvert", "TargetBase", 16);
    setValue("BaseConvert", "PaddingZero", false);

    setValue("CRCCheck", "CRCType", "CRC-16");
    setValue("CRCCheck", "Polynomial", "0x8005");
    setValue("CRCCheck", "InitValue", "0x0000");
    setValue("CRCCheck", "XorOut", "0x0000");
    setValue("CRCCheck", "RefIn", false);
    setValue("CRCCheck", "RefOut", false);

    setValue("TimingDiagram", "ChannelCount", 4);
    setValue("TimingDiagram", "TimeScale", 1.0);

    setValue("DelayCalc", "ClockFrequency", 100);
    setValue("DelayCalc", "InstructionCycles", 1);

    setValue("DataSheet", "OpenInNewWindow", true);

    m_settings->sync();
}

void ConfigManager::ensureConfigDirectory()
{
    QString documentsPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    QString configDir = documentsPath + "/SLinBox";
    
    QDir dir;
    if (!dir.exists(configDir)) {
        dir.mkpath(configDir);
    }
    
    QString templatesDir = configDir + "/templates";
    if (!dir.exists(templatesDir)) {
        dir.mkpath(templatesDir);
    }
}