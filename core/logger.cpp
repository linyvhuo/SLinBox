#include "logger.h"
#include <QDir>
#include <QStandardPaths>
#include <QCoreApplication>
#include <QDebug>

Logger* Logger::s_instance = nullptr;

Logger::Logger(QObject *parent)
    : QObject(parent)
    , m_logFile(nullptr)
    , m_logStream(nullptr)
    , m_logLevel(LogLevel::DEBUG)
{
    ensureLogDirectory();
    
    QString documentsPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    QString logDir = documentsPath + "/SLinBox";
    
    QDir dir;
    if (!dir.exists(logDir)) {
        dir.mkpath(logDir);
    }
    
    QString logFileName = QString("SLinBox_%1.log")
        .arg(QDateTime::currentDateTime().toString("yyyyMMdd"));
    m_logFilePath = logDir + "/" + logFileName;
    
    m_logFile = new QFile(m_logFilePath, this);
    if (m_logFile->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        m_logStream = new QTextStream(m_logFile);
        m_logStream->setCodec("UTF-8");
    }
}

Logger::~Logger()
{
    if (m_logStream) {
        delete m_logStream;
    }
    if (m_logFile) {
        m_logFile->close();
        delete m_logFile;
    }
}

Logger* Logger::instance()
{
    if (!s_instance) {
        s_instance = new Logger();
    }
    return s_instance;
}

void Logger::log(LogLevel level, const QString &module, const QString &message)
{
    if (level < m_logLevel) {
        return;
    }

    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    QString levelStr = levelToString(level);
    
    QString logMessage = QString("[%1] [%2] [%3] %4")
        .arg(timestamp)
        .arg(levelStr)
        .arg(module)
        .arg(message);
    
    {
        QMutexLocker locker(&m_mutex);
        writeToFile(logMessage);
    }
    
    emit logAdded(logMessage, level);
}

void Logger::debug(const QString &module, const QString &message)
{
    log(LogLevel::DEBUG, module, message);
}

void Logger::info(const QString &module, const QString &message)
{
    log(LogLevel::INFO, module, message);
}

void Logger::warn(const QString &module, const QString &message)
{
    log(LogLevel::WARN, module, message);
}

void Logger::error(const QString &module, const QString &message)
{
    log(LogLevel::ERROR_LEVEL, module, message);
}

void Logger::setLogLevel(LogLevel level)
{
    m_logLevel = level;
}

LogLevel Logger::getLogLevel() const
{
    return m_logLevel;
}

void Logger::setLogFilePath(const QString &path)
{
    QMutexLocker locker(&m_mutex);
    
    if (m_logStream) {
        delete m_logStream;
        m_logStream = nullptr;
    }
    if (m_logFile) {
        m_logFile->close();
        delete m_logFile;
        m_logFile = nullptr;
    }
    
    m_logFilePath = path;
    m_logFile = new QFile(m_logFilePath, this);
    if (m_logFile->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        m_logStream = new QTextStream(m_logFile);
        m_logStream->setCodec("UTF-8");
    }
}

QString Logger::getLogFilePath() const
{
    return m_logFilePath;
}

void Logger::clearLog()
{
    QMutexLocker locker(&m_mutex);
    
    if (m_logFile) {
        m_logFile->close();
        m_logFile->open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text);
        
        if (m_logStream) {
            delete m_logStream;
            m_logStream = new QTextStream(m_logFile);
            m_logStream->setCodec("UTF-8");
        }
    }
    
    emit logCleared();
}

bool Logger::exportLog(const QString &exportPath)
{
    QMutexLocker locker(&m_mutex);
    
    QFile sourceFile(m_logFilePath);
    if (!sourceFile.exists()) {
        return false;
    }
    
    return sourceFile.copy(exportPath);
}

QString Logger::levelToString(LogLevel level) const
{
    switch (level) {
        case LogLevel::DEBUG:
            return "调试";
        case LogLevel::INFO:
            return "信息";
        case LogLevel::WARN:
            return "警告";
        case LogLevel::ERROR_LEVEL:
            return "错误";
        default:
            return "未知";
    }
}

LogLevel Logger::stringToLevel(const QString &levelStr) const
{
    QString upperStr = levelStr.toUpper();
    
    if (upperStr == "DEBUG" || upperStr == "调试") {
        return LogLevel::DEBUG;
    } else if (upperStr == "INFO" || upperStr == "信息") {
        return LogLevel::INFO;
    } else if (upperStr == "WARN" || upperStr == "警告") {
        return LogLevel::WARN;
    } else if (upperStr == "ERROR" || upperStr == "错误") {
        return LogLevel::ERROR_LEVEL;
    }
    
    return LogLevel::INFO;
}

void Logger::writeToFile(const QString &logMessage)
{
    if (m_logStream && m_logFile && m_logFile->isOpen()) {
        *m_logStream << logMessage << "\n";
        m_logStream->flush();
    }
}

void Logger::ensureLogDirectory()
{
    QString documentsPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    QString logDir = documentsPath + "/SLinBox";
    
    QDir dir;
    if (!dir.exists(logDir)) {
        dir.mkpath(logDir);
    }
}