#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <QObject>
#include <QSettings>
#include <QString>
#include <QVariant>
#include <QMap>

class ConfigManager : public QObject
{
    Q_OBJECT

public:
    explicit ConfigManager(QObject *parent = nullptr);
    ~ConfigManager();

    static ConfigManager* instance();

    void setValue(const QString &group, const QString &key, const QVariant &value);
    QVariant getValue(const QString &group, const QString &key, const QVariant &defaultValue = QVariant());

    void setGroup(const QString &group);
    void beginGroup(const QString &group);
    void endGroup();

    void sync();
    void clear();

    bool backup(const QString &backupPath);
    bool restore(const QString &backupPath);

    void resetToDefault();

    QString getConfigPath() const;

private:
    void initDefaultConfig();
    void ensureConfigDirectory();

    QSettings *m_settings;
    QString m_configPath;
    static ConfigManager *s_instance;
};

#endif // CONFIGMANAGER_H