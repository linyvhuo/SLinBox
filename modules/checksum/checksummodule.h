#ifndef CHECKSUMMODULE_H
#define CHECKSUMMODULE_H

#include <QWidget>
#include "core/configmanager.h"
#include "core/logger.h"

QT_BEGIN_NAMESPACE
namespace Ui { class ChecksumModule; }
QT_END_NAMESPACE

class ChecksumModule : public QWidget
{
    Q_OBJECT

public:
    explicit ChecksumModule(QWidget *parent = nullptr);
    ~ChecksumModule();

    void loadConfig();
    void saveConfig();

private slots:
    void onCalculate();
    void onClear();
    void onCopyResult();
    void onInputChanged();
    void onChecksumTypeChanged(int index);

private:
    void initUI();
    quint8 calculateChecksum8(const QByteArray &data);
    quint16 calculateChecksum16(const QByteArray &data);
    quint16 calculateChecksum16TwosComplement(const QByteArray &data);
    quint32 calculateChecksum32(const QByteArray &data);
    quint8 calculateXORChecksum(const QByteArray &data);
    quint8 calculateLRC(const QByteArray &data);
    quint16 calculateFletcher16(const QByteArray &data);
    quint32 calculateAdler32(const QByteArray &data);

    Ui::ChecksumModule *ui;

    ConfigManager *m_config;
    Logger *m_logger;
};

#endif // CHECKSUMMODULE_H