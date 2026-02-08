#include "checksummodule.h"
#include "ui_checksummodule.h"
#include <QClipboard>
#include <QApplication>
#include <QMessageBox>
#include <QDebug>


ChecksumModule::ChecksumModule(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ChecksumModule)
    , m_config(ConfigManager::instance())
    , m_logger(Logger::instance())
{
    ui->setupUi(this);
    initUI();
    loadConfig();
}

ChecksumModule::~ChecksumModule()
{
    delete ui;
}

void ChecksumModule::initUI()
{
    connect(ui->btnCalculate, &QPushButton::clicked, this, &ChecksumModule::onCalculate);
    connect(ui->btnClear, &QPushButton::clicked, this, &ChecksumModule::onClear);
    connect(ui->btnCopyResult, &QPushButton::clicked, this, &ChecksumModule::onCopyResult);
    connect(ui->editInput, &QPlainTextEdit::textChanged, this, &ChecksumModule::onInputChanged);
    connect(ui->comboChecksumType, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ChecksumModule::onChecksumTypeChanged);
}

void ChecksumModule::loadConfig()
{
}

void ChecksumModule::saveConfig()
{
}

void ChecksumModule::onCalculate()
{
    QString input = ui->editInput->toPlainText().trimmed();
    if (input.isEmpty()) {
        QMessageBox::warning(this, "Warning", "Please enter data to calculate!");
        return;
    }
    
    QByteArray data;
    
    if (ui->checkHexInput->isChecked()) {
        QStringList hexList = input.split(' ', Qt::SkipEmptyParts);
        for (const QString &hex : hexList) {
            bool ok;
            data.append(static_cast<char>(hex.toInt(&ok, 16)));
            if (!ok) {
                QMessageBox::warning(this, "错误", "无效的十六进制数据！");
                m_logger->error("Checksum", "Invalid hex data");
                return;
            }
        }
    } else {
        data = input.toUtf8();
    }
    
    QString checksumType = ui->comboChecksumType->currentText();
    QString result;
    
    if (checksumType == "8位累加和") {
        quint8 checksum = calculateChecksum8(data);
        result = QString("0x%1").arg(checksum, 2, 16, QChar('0')).toUpper();
    } else if (checksumType == "16位累加和") {
        quint16 checksum = calculateChecksum16(data);
        result = QString("0x%1").arg(checksum, 4, 16, QChar('0')).toUpper();
    } else if (checksumType == "16位补码和") {
        quint16 checksum = calculateChecksum16TwosComplement(data);
        result = QString("0x%1").arg(checksum, 4, 16, QChar('0')).toUpper();
    } else if (checksumType == "32位累加和") {
        quint32 checksum = calculateChecksum32(data);
        result = QString("0x%1").arg(checksum, 8, 16, QChar('0')).toUpper();
    } else if (checksumType == "异或和") {
        quint8 checksum = calculateXORChecksum(data);
        result = QString("0x%1").arg(checksum, 2, 16, QChar('0')).toUpper();
    } else if (checksumType == "LRC校验") {
        quint8 checksum = calculateLRC(data);
        result = QString("0x%1").arg(checksum, 2, 16, QChar('0')).toUpper();
    } else if (checksumType == "Fletcher-16") {
        quint16 checksum = calculateFletcher16(data);
        result = QString("0x%1").arg(checksum, 4, 16, QChar('0')).toUpper();
    } else if (checksumType == "Adler-32") {
        quint32 checksum = calculateAdler32(data);
        result = QString("0x%1").arg(checksum, 8, 16, QChar('0')).toUpper();
    }
    
    ui->editResult->setText(result);
    
    m_logger->info("Checksum", QString("Checksum calculation successful: %1 -> %2").arg(input).arg(result));
}

void ChecksumModule::onClear()
{
    ui->editInput->clear();
    ui->editResult->clear();
    m_logger->info("Checksum", "Clear input and result");
}

void ChecksumModule::onCopyResult()
{
    QString result = ui->editResult->text();
    if (result.isEmpty()) {
        QMessageBox::warning(this, "警告", "没有结果可以复制！");
        return;
    }
    
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(result);
    
    QMessageBox::information(this, "成功", "结果已复制到剪贴板！");
    m_logger->info("Checksum", "Result copied to clipboard");
}

void ChecksumModule::onInputChanged()
{
    if (!ui->checkRealTime->isChecked()) {
        return;
    }
    
    QString input = ui->editInput->toPlainText().trimmed();
    if (input.isEmpty()) {
        ui->editResult->clear();
        return;
    }
    
    QByteArray data;
    
    if (ui->checkHexInput->isChecked()) {
        QStringList hexList = input.split(' ', Qt::SkipEmptyParts);
        for (const QString &hex : hexList) {
            bool ok;
            data.append(static_cast<char>(hex.toInt(&ok, 16)));
            if (!ok) {
                return;
            }
        }
    } else {
        data = input.toUtf8();
    }
    
    QString checksumType = ui->comboChecksumType->currentText();
    QString result;
    
    if (checksumType == "8位累加和") {
        quint8 checksum = calculateChecksum8(data);
        result = QString("0x%1").arg(checksum, 2, 16, QChar('0')).toUpper();
    } else if (checksumType == "16位累加和") {
        quint16 checksum = calculateChecksum16(data);
        result = QString("0x%1").arg(checksum, 4, 16, QChar('0')).toUpper();
    } else if (checksumType == "16位补码和") {
        quint16 checksum = calculateChecksum16TwosComplement(data);
        result = QString("0x%1").arg(checksum, 4, 16, QChar('0')).toUpper();
    } else if (checksumType == "32位累加和") {
        quint32 checksum = calculateChecksum32(data);
        result = QString("0x%1").arg(checksum, 8, 16, QChar('0')).toUpper();
    } else if (checksumType == "异或和") {
        quint8 checksum = calculateXORChecksum(data);
        result = QString("0x%1").arg(checksum, 2, 16, QChar('0')).toUpper();
    } else if (checksumType == "LRC校验") {
        quint8 checksum = calculateLRC(data);
        result = QString("0x%1").arg(checksum, 2, 16, QChar('0')).toUpper();
    } else if (checksumType == "Fletcher-16") {
        quint16 checksum = calculateFletcher16(data);
        result = QString("0x%1").arg(checksum, 4, 16, QChar('0')).toUpper();
    } else if (checksumType == "Adler-32") {
        quint32 checksum = calculateAdler32(data);
        result = QString("0x%1").arg(checksum, 8, 16, QChar('0')).toUpper();
    }
    
    ui->editResult->setText(result);
}

void ChecksumModule::onChecksumTypeChanged(int index)
{
    Q_UNUSED(index);
    if (ui->checkRealTime->isChecked()) {
        onInputChanged();
    }
}

quint8 ChecksumModule::calculateChecksum8(const QByteArray &data)
{
    quint8 checksum = 0;
    
    for (int i = 0; i < data.size(); ++i) {
        checksum += static_cast<quint8>(data[i]);
    }
    
    return checksum;
}

quint16 ChecksumModule::calculateChecksum16(const QByteArray &data)
{
    quint16 checksum = 0;
    
    for (int i = 0; i < data.size(); ++i) {
        checksum += static_cast<quint8>(data[i]);
    }
    
    return checksum;
}

quint8 ChecksumModule::calculateXORChecksum(const QByteArray &data)
{
    quint8 checksum = 0;
    
    for (int i = 0; i < data.size(); ++i) {
        checksum ^= static_cast<quint8>(data[i]);
    }
    
    return checksum;
}

quint16 ChecksumModule::calculateChecksum16TwosComplement(const QByteArray &data)
{
    quint16 checksum = 0;
    
    for (int i = 0; i < data.size(); ++i) {
        checksum += static_cast<quint8>(data[i]);
    }
    
    return ~checksum + 1;
}

quint32 ChecksumModule::calculateChecksum32(const QByteArray &data)
{
    quint32 checksum = 0;
    
    for (int i = 0; i < data.size(); ++i) {
        checksum += static_cast<quint8>(data[i]);
    }
    
    return checksum;
}

quint8 ChecksumModule::calculateLRC(const QByteArray &data)
{
    quint8 lrc = 0;
    
    for (int i = 0; i < data.size(); ++i) {
        lrc = (lrc + static_cast<quint8>(data[i])) & 0xFF;
    }
    
    return (~lrc + 1) & 0xFF;
}

quint16 ChecksumModule::calculateFletcher16(const QByteArray &data)
{
    quint16 sum1 = 0;
    quint16 sum2 = 0;
    
    for (int i = 0; i < data.size(); ++i) {
        quint8 byte = static_cast<quint8>(data[i]);
        sum1 = (sum1 + byte) & 0xFF;
        sum2 = (sum2 + sum1) & 0xFF;
    }
    
    return (sum2 << 8) | sum1;
}

quint32 ChecksumModule::calculateAdler32(const QByteArray &data)
{
    const quint32 MOD_ADLER = 65521;
    quint32 a = 1;
    quint32 b = 0;
    
    for (int i = 0; i < data.size(); ++i) {
        quint8 byte = static_cast<quint8>(data[i]);
        a = (a + byte) % MOD_ADLER;
        b = (b + a) % MOD_ADLER;
    }
    
    return (b << 16) | a;
}
