#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>
#include <QFile>
#include <QTextStream>
#include <QMutex>
#include <QString>
#include <QDateTime>

enum class LogLevel {
    DEBUG,
    INFO,
    WARN,
    ERROR_LEVEL
};

class Logger : public QObject
{
    Q_OBJECT

public:
    static Logger* instance();

    void log(LogLevel level, const QString &module, const QString &message);
    void debug(const QString &module, const QString &message);
    void info(const QString &module, const QString &message);
    void warn(const QString &module, const QString &message);
    void error(const QString &module, const QString &message);

    void setLogLevel(LogLevel level);
    LogLevel getLogLevel() const;

    void setLogFilePath(const QString &path);
    QString getLogFilePath() const;

    void clearLog();
    bool exportLog(const QString &exportPath);

    QString levelToString(LogLevel level) const;
    LogLevel stringToLevel(const QString &levelStr) const;

signals:
    void logAdded(const QString &logMessage, LogLevel level);
    void logCleared();

private:
    explicit Logger(QObject *parent = nullptr);
    ~Logger();

    void writeToFile(const QString &logMessage);
    void ensureLogDirectory();

    QFile *m_logFile;
    QTextStream *m_logStream;
    QMutex m_mutex;
    LogLevel m_logLevel;
    QString m_logFilePath;
    static Logger *s_instance;
};

#endif // LOGGER_H