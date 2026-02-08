#include "serialportmodule.h"
#include "ui_serialportmodule.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QDateTime>
#include <QDebug>


SerialPortModule::SerialPortModule(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SerialPortModule)
    , m_serialPort(nullptr)
    , m_autoSendTimer(new QTimer(this))
    , m_config(ConfigManager::instance())
    , m_logger(Logger::instance())
    , m_isOpen(false)
    , m_hexDisplay(false)
    , m_autoScroll(true)
{
    ui->setupUi(this);
    initUI();
    loadConfig();
}

SerialPortModule::~SerialPortModule()
{
    if (m_serialPort && m_serialPort->isOpen()) {
        m_serialPort->close();
    }
    delete m_serialPort;
    delete ui;
}

void SerialPortModule::initUI()
{
    m_serialPort = new QSerialPort(this);
    
    connect(m_serialPort, &QSerialPort::readyRead, this, &SerialPortModule::onDataReceived);
    connect(m_serialPort, QOverload<QSerialPort::SerialPortError>::of(&QSerialPort::errorOccurred),
            this, &SerialPortModule::onErrorOccurred);
    
    connect(m_autoSendTimer, &QTimer::timeout, this, &SerialPortModule::onSendData);
    
    connect(ui->btnRefresh, &QPushButton::clicked, this, &SerialPortModule::onRefreshPorts);
    connect(ui->btnOpen, &QPushButton::clicked, this, &SerialPortModule::onOpenPort);
    connect(ui->btnClose, &QPushButton::clicked, this, &SerialPortModule::onClosePort);
    connect(ui->btnSend, &QPushButton::clicked, this, &SerialPortModule::onSendData);
    connect(ui->btnClearReceive, &QPushButton::clicked, this, &SerialPortModule::onClearReceive);
    connect(ui->btnClearSend, &QPushButton::clicked, this, &SerialPortModule::onClearSend);
    connect(ui->comboSendHistory, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &SerialPortModule::onSendHistorySelected);
    connect(ui->comboDisplayMode, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &SerialPortModule::onDisplayModeChanged);
    connect(ui->checkAutoScroll, &QCheckBox::stateChanged, this, &SerialPortModule::onAutoScrollChanged);
    connect(ui->btnSaveLog, &QPushButton::clicked, this, &SerialPortModule::onSaveLog);
    connect(ui->spinSendInterval, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &SerialPortModule::onSendIntervalChanged);
    connect(ui->checkAutoSend, &QCheckBox::stateChanged, this, &SerialPortModule::onAutoSendChanged);
    
    ui->btnClose->setEnabled(false);
    ui->btnSend->setEnabled(false);
    
    refreshPortList();
}

void SerialPortModule::loadConfig()
{
    ui->comboPortName->setCurrentText(m_config->getValue("SerialPort", "PortName", "").toString());
    ui->comboBaudRate->setCurrentText(QString::number(m_config->getValue("SerialPort", "BaudRate", 9600).toInt()));
    ui->comboDataBits->setCurrentText(QString::number(m_config->getValue("SerialPort", "DataBits", 8).toInt()));
    ui->comboStopBits->setCurrentText(QString::number(m_config->getValue("SerialPort", "StopBits", 1).toInt()));
    ui->comboParity->setCurrentText(m_config->getValue("SerialPort", "Parity", "None").toString());
    ui->comboFlowControl->setCurrentText(m_config->getValue("SerialPort", "FlowControl", "None").toString());
    ui->comboDisplayMode->setCurrentText(m_config->getValue("SerialPort", "DisplayMode", "ASCII").toString());
    ui->checkAutoScroll->setChecked(m_config->getValue("SerialPort", "AutoScroll", true).toBool());
}

void SerialPortModule::saveConfig()
{
    m_config->setValue("SerialPort", "PortName", ui->comboPortName->currentText());
    m_config->setValue("SerialPort", "BaudRate", ui->comboBaudRate->currentText().toInt());
    m_config->setValue("SerialPort", "DataBits", ui->comboDataBits->currentText().toInt());
    m_config->setValue("SerialPort", "StopBits", ui->comboStopBits->currentText().toInt());
    m_config->setValue("SerialPort", "Parity", ui->comboParity->currentText());
    m_config->setValue("SerialPort", "FlowControl", ui->comboFlowControl->currentText());
    m_config->setValue("SerialPort", "DisplayMode", ui->comboDisplayMode->currentText());
    m_config->setValue("SerialPort", "AutoScroll", ui->checkAutoScroll->isChecked());
    m_config->sync();
}

void SerialPortModule::onRefreshPorts()
{
    refreshPortList();
    m_logger->info("SerialPort", "Refresh port list");
}

void SerialPortModule::onOpenPort()
{
    if (m_isOpen) {
        return;
    }
    
    QString portName = ui->comboPortName->currentText();
    if (portName.isEmpty()) {
        QMessageBox::warning(this, "警告", "请选择一个串口！");
        return;
    }
    
    m_serialPort->setPortName(portName);
    m_serialPort->setBaudRate(ui->comboBaudRate->currentText().toInt());
    m_serialPort->setDataBits(static_cast<QSerialPort::DataBits>(ui->comboDataBits->currentText().toInt()));
    m_serialPort->setStopBits(static_cast<QSerialPort::StopBits>(ui->comboStopBits->currentText().toInt()));
    
    QString parity = ui->comboParity->currentText();
    if (parity == "None") {
        m_serialPort->setParity(QSerialPort::NoParity);
    } else if (parity == "Even") {
        m_serialPort->setParity(QSerialPort::EvenParity);
    } else if (parity == "Odd") {
        m_serialPort->setParity(QSerialPort::OddParity);
    } else if (parity == "Mark") {
        m_serialPort->setParity(QSerialPort::MarkParity);
    } else if (parity == "Space") {
        m_serialPort->setParity(QSerialPort::SpaceParity);
    }
    
    QString flowControl = ui->comboFlowControl->currentText();
    if (flowControl == "None") {
        m_serialPort->setFlowControl(QSerialPort::NoFlowControl);
    } else if (flowControl == "Hardware") {
        m_serialPort->setFlowControl(QSerialPort::HardwareControl);
    } else if (flowControl == "Software") {
        m_serialPort->setFlowControl(QSerialPort::SoftwareControl);
    }
    
    if (m_serialPort->open(QIODevice::ReadWrite)) {
        m_isOpen = true;
        ui->btnOpen->setEnabled(false);
        ui->btnClose->setEnabled(true);
        ui->btnSend->setEnabled(true);
        ui->labelStatus->setText("状态: 打开");
        ui->labelStatus->setStyleSheet("color: green;");
        
        saveConfig();
        m_logger->info("SerialPort", QString("Port %1 opened successfully").arg(portName));
    } else {
        QMessageBox::critical(this, "错误", "打开端口失败: " + m_serialPort->errorString());
        m_logger->error("SerialPort", QString("Failed to open port: %1").arg(m_serialPort->errorString()));
    }
}

void SerialPortModule::onClosePort()
{
    if (!m_isOpen) {
        return;
    }
    
    if (m_autoSendTimer->isActive()) {
        m_autoSendTimer->stop();
        ui->checkAutoSend->setChecked(false);
    }
    
    m_serialPort->close();
    m_isOpen = false;
    
    ui->btnOpen->setEnabled(true);
    ui->btnClose->setEnabled(false);
    ui->btnSend->setEnabled(false);
    ui->labelStatus->setText("状态: 关闭");
    ui->labelStatus->setStyleSheet("color: red;");
    
    m_logger->info("串口", "端口关闭");
}

void SerialPortModule::onSendData()
{
    if (!m_isOpen) {
        QMessageBox::warning(this, "警告", "请先打开端口！");
        return;
    }
    
    QString data = ui->editSend->toPlainText();
    if (data.isEmpty()) {
        return;
    }
    
    sendData(data);
    
    QString historyItem = ui->editSend->toPlainText();
    if (ui->comboSendHistory->findText(historyItem) == -1) {
        ui->comboSendHistory->addItem(historyItem);
        if (ui->comboSendHistory->count() > 10) {
            ui->comboSendHistory->removeItem(0);
        }
    }
    
    m_logger->info("SerialPort", QString("Sent data: %1").arg(data));
}

void SerialPortModule::onClearReceive()
{
    ui->editReceive->clear();
    m_logger->info("SerialPort", "Clear receive area");
}

void SerialPortModule::onClearSend()
{
    ui->editSend->clear();
    m_logger->info("SerialPort", "Clear send area");
}

void SerialPortModule::onSendHistorySelected(int index)
{
    if (index >= 0) {
        ui->editSend->setPlainText(ui->comboSendHistory->itemText(index));
    }
}

void SerialPortModule::onDataReceived()
{
    QByteArray data = m_serialPort->readAll();
    appendLog(QString::fromUtf8(data), true);
}

void SerialPortModule::onErrorOccurred(QSerialPort::SerialPortError error)
{
    if (error != QSerialPort::NoError) {
        m_logger->error("SerialPort", QString("Serial port error: %1").arg(m_serialPort->errorString()));
    }
}

void SerialPortModule::onDisplayModeChanged(int index)
{
    m_hexDisplay = (ui->comboDisplayMode->currentText() == "Hex");
    saveConfig();
}

void SerialPortModule::onAutoScrollChanged(int state)
{
    m_autoScroll = (state == Qt::Checked);
    saveConfig();
}

void SerialPortModule::onSaveLog()
{
    QString logText = ui->editReceive->toPlainText();
    if (logText.isEmpty()) {
        QMessageBox::warning(this, "警告", "没有日志可以保存！");
        return;
    }
    
    QString filePath = QFileDialog::getSaveFileName(this, "保存日志", "", "文本文件 (*.txt);;所有文件 (*)");
    if (!filePath.isEmpty()) {
        QFile file(filePath);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            out << logText;
            file.close();
            
            QMessageBox::information(this, "成功", "日志保存成功！");
            m_logger->info("SerialPort", QString("Log saved successfully: %1").arg(filePath));
        } else {
            QMessageBox::warning(this, "失败", "日志保存失败！");
            m_logger->error("SerialPort", "Failed to save log");
        }
    }
}

void SerialPortModule::onSendIntervalChanged(int value)
{
    if (m_autoSendTimer->isActive()) {
        m_autoSendTimer->setInterval(value);
    }
}

void SerialPortModule::onAutoSendChanged(int state)
{
    if (state == Qt::Checked) {
        if (!m_isOpen) {
            QMessageBox::warning(this, "警告", "请先打开端口！");
            ui->checkAutoSend->setChecked(false);
            return;
        }
        
        int interval = ui->spinSendInterval->value();
        m_autoSendTimer->start(interval);
        m_logger->info("SerialPort", QString("Start auto send, interval: %1ms").arg(interval));
    } else {
        if (m_autoSendTimer->isActive()) {
            m_autoSendTimer->stop();
            m_logger->info("SerialPort", "Stop auto send");
        }
    }
}

void SerialPortModule::sendData(const QString &data)
{
    QByteArray sendData;
    
    if (ui->checkHexSend->isChecked()) {
        QStringList hexList = data.split(' ', Qt::SkipEmptyParts);
        for (const QString &hex : hexList) {
            bool ok;
            sendData.append(static_cast<char>(hex.toInt(&ok, 16)));
            if (!ok) {
                m_logger->error("SerialPort", QString("Invalid hex data: %1").arg(hex));
                return;
            }
        }
    } else {
        sendData = data.toUtf8();
    }
    
    qint64 bytesWritten = m_serialPort->write(sendData);
    if (bytesWritten == -1) {
        m_logger->error("SerialPort", QString("Send failed: %1").arg(m_serialPort->errorString()));
    } else if (bytesWritten != sendData.size()) {
        m_logger->warn("SerialPort", QString("Send incomplete: %1/%2").arg(bytesWritten).arg(sendData.size()));
    }
}

void SerialPortModule::appendLog(const QString &data, bool isReceived)
{
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
    QString prefix = isReceived ? "[RX]" : "[TX]";
    QString logLine = QString("%1 %2 %3").arg(timestamp, prefix, data);
    
    ui->editReceive->appendPlainText(logLine);
    
    if (m_autoScroll) {
        QTextCursor cursor = ui->editReceive->textCursor();
        cursor.movePosition(QTextCursor::End);
        ui->editReceive->setTextCursor(cursor);
    }
}

QString SerialPortModule::formatData(const QByteArray &data)
{
    if (m_hexDisplay) {
        QString hexString;
        for (int i = 0; i < data.size(); ++i) {
            hexString += QString("%1 ").arg(static_cast<unsigned char>(data[i]), 2, 16, QChar('0')).toUpper();
        }
        return hexString.trimmed();
    } else {
        return QString::fromUtf8(data);
    }
}

void SerialPortModule::refreshPortList()
{
    ui->comboPortName->clear();
    
    const auto infos = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &info : infos) {
        ui->comboPortName->addItem(info.portName());
    }
    
    if (ui->comboPortName->count() == 0) {
        ui->comboPortName->addItem("No available ports");
    }
}
