#ifndef SERIALPORTMODULE_H
#define SERIALPORTMODULE_H

#include <QWidget>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QTimer>
#include "core/configmanager.h"
#include "core/logger.h"

QT_BEGIN_NAMESPACE
namespace Ui { class SerialPortModule; }
QT_END_NAMESPACE

class SerialPortModule : public QWidget
{
    Q_OBJECT

public:
    explicit SerialPortModule(QWidget *parent = nullptr);
    ~SerialPortModule();

    void loadConfig();
    void saveConfig();

private slots:
    void onRefreshPorts();
    void onOpenPort();
    void onClosePort();
    void onSendData();
    void onClearReceive();
    void onClearSend();
    void onSendHistorySelected(int index);
    void onDataReceived();
    void onErrorOccurred(QSerialPort::SerialPortError error);
    void onDisplayModeChanged(int index);
    void onAutoScrollChanged(int state);
    void onSaveLog();
    void onSendIntervalChanged(int value);
    void onAutoSendChanged(int state);

private:
    void initUI();
    void refreshPortList();
    void sendData(const QString &data);
    void appendLog(const QString &data, bool isReceived);
    QString formatData(const QByteArray &data);

    Ui::SerialPortModule *ui;

    QSerialPort *m_serialPort;
    QTimer *m_autoSendTimer;

    ConfigManager *m_config;
    Logger *m_logger;

    bool m_isOpen;
    bool m_hexDisplay;
    bool m_autoScroll;
};

#endif // SERIALPORTMODULE_H