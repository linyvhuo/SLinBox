#ifndef CRCCHECKMODULE_H
#define CRCCHECKMODULE_H

#include <QWidget>
#include "core/configmanager.h"
#include "core/logger.h"

QT_BEGIN_NAMESPACE
namespace Ui { class CRCCheckModule; }
QT_END_NAMESPACE

class CRCCheckModule : public QWidget
{
    Q_OBJECT

public:
    explicit CRCCheckModule(QWidget *parent = nullptr);
    ~CRCCheckModule();

    void loadConfig();
    void saveConfig();

private slots:
    void onCalculate();
    void onClear();
    void onCopyResult();

private:
    void initUI();
    quint16 calculateCRC16(const QByteArray &data, quint16 polynomial, quint16 initValue, bool refIn, bool refOut, quint16 xorOut);
    quint32 calculateCRC32(const QByteArray &data, quint32 polynomial, quint32 initValue, bool refIn, bool refOut, quint32 xorOut);
    quint8 calculateCRC8(const QByteArray &data, quint8 polynomial, quint8 initValue, bool refIn, bool refOut, quint8 xorOut);
    void applyCRCPreset(const QString &presetName);
    quint8 reverseBits(quint8 byte);
    quint16 reverseBits16(quint16 value);
    quint32 reverseBits32(quint32 value);

    Ui::CRCCheckModule *ui;

    ConfigManager *m_config;
    Logger *m_logger;
};

#endif // CRCCHECKMODULE_H