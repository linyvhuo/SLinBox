#include "crccheckmodule.h"
#include "ui_crccheckmodule.h"
#include <QClipboard>
#include <QApplication>
#include <QMessageBox>
#include <QDebug>


CRCCheckModule::CRCCheckModule(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::CRCCheckModule)
    , m_config(ConfigManager::instance())
    , m_logger(Logger::instance())
{
    ui->setupUi(this);
    initUI();
    loadConfig();
}

CRCCheckModule::~CRCCheckModule()
{
    delete ui;
}

void CRCCheckModule::initUI()
{
    connect(ui->btnCalculate, &QPushButton::clicked, this, &CRCCheckModule::onCalculate);
    connect(ui->btnClear, &QPushButton::clicked, this, &CRCCheckModule::onClear);
    connect(ui->btnCopyResult, &QPushButton::clicked, this, &CRCCheckModule::onCopyResult);
    connect(ui->comboCRCType, QOverload<const QString&>::of(&QComboBox::currentTextChanged),
            this, &CRCCheckModule::applyCRCPreset);
}

void CRCCheckModule::loadConfig()
{
    ui->comboCRCType->setCurrentText(m_config->getValue("CRCCheck", "CRCType", "CRC-16").toString());
    ui->editPolynomial->setText(m_config->getValue("CRCCheck", "Polynomial", "0x8005").toString());
    ui->editInitValue->setText(m_config->getValue("CRCCheck", "InitValue", "0x0000").toString());
    ui->editXorOut->setText(m_config->getValue("CRCCheck", "XorOut", "0x0000").toString());
    ui->checkRefIn->setChecked(m_config->getValue("CRCCheck", "RefIn", false).toBool());
    ui->checkRefOut->setChecked(m_config->getValue("CRCCheck", "RefOut", false).toBool());
}

void CRCCheckModule::saveConfig()
{
    m_config->setValue("CRCCheck", "CRCType", ui->comboCRCType->currentText());
    m_config->setValue("CRCCheck", "Polynomial", ui->editPolynomial->text());
    m_config->setValue("CRCCheck", "InitValue", ui->editInitValue->text());
    m_config->setValue("CRCCheck", "XorOut", ui->editXorOut->text());
    m_config->setValue("CRCCheck", "RefIn", ui->checkRefIn->isChecked());
    m_config->setValue("CRCCheck", "RefOut", ui->checkRefOut->isChecked());
    m_config->sync();
}

void CRCCheckModule::onCalculate()
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
                m_logger->error("CRCCheck", "Invalid hex data");
                return;
            }
        }
    } else {
        data = input.toUtf8();
    }
    
    bool ok;
    quint16 polynomial16 = ui->editPolynomial->text().toUShort(&ok, 16);
    quint16 initValue16 = ui->editInitValue->text().toUShort(&ok, 16);
    quint16 xorOut16 = ui->editXorOut->text().toUShort(&ok, 16);
    bool refIn = ui->checkRefIn->isChecked();
    bool refOut = ui->checkRefOut->isChecked();
    
    QString crcType = ui->comboCRCType->currentText();
    QString result;
    
    if (crcType == "CRC-8") {
        quint8 polynomial8 = static_cast<quint8>(polynomial16);
        quint8 initValue8 = static_cast<quint8>(initValue16);
        quint8 xorOut8 = static_cast<quint8>(xorOut16);
        quint8 crc = calculateCRC8(data, polynomial8, initValue8, refIn, refOut, xorOut8);
        result = QString("0x%1").arg(crc, 2, 16, QChar('0')).toUpper();
    } else if (crcType == "CRC-16") {
        quint16 crc = calculateCRC16(data, polynomial16, initValue16, refIn, refOut, xorOut16);
        result = QString("0x%1").arg(crc, 4, 16, QChar('0')).toUpper();
    } else if (crcType == "CRC-32") {
        quint32 polynomial32 = polynomial16;
        quint32 initValue32 = initValue16;
        quint32 xorOut32 = xorOut16;
        quint32 crc = calculateCRC32(data, polynomial32, initValue32, refIn, refOut, xorOut32);
        result = QString("0x%1").arg(crc, 8, 16, QChar('0')).toUpper();
    }
    
    ui->editResult->setText(result);
    
    m_logger->info("CRCCheck", QString("CRC calculation successful: %1 -> %2").arg(input).arg(result));
}

void CRCCheckModule::onClear()
{
    ui->editInput->clear();
    ui->editResult->clear();
    m_logger->info("CRCCheck", "Clear input and result");
}

void CRCCheckModule::onCopyResult()
{
    QString result = ui->editResult->text();
    if (result.isEmpty()) {
        QMessageBox::warning(this, "警告", "没有结果可以复制！");
        return;
    }
    
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(result);
    
    QMessageBox::information(this, "成功", "结果已复制到剪贴板！");
    m_logger->info("CRCCheck", "Result copied to clipboard");
}

quint16 CRCCheckModule::calculateCRC16(const QByteArray &data, quint16 polynomial, quint16 initValue, bool refIn, bool refOut, quint16 xorOut)
{
    quint16 crc = initValue;
    
    for (int i = 0; i < data.size(); ++i) {
        quint8 byte = static_cast<quint8>(data[i]);
        if (refIn) {
            byte = reverseBits(byte);
        }
        crc ^= byte << 8;
        for (int j = 0; j < 8; ++j) {
            if (crc & 0x8000) {
                crc = (crc << 1) ^ polynomial;
            } else {
                crc <<= 1;
            }
        }
    }
    
    if (refOut) {
        crc = reverseBits16(crc);
    }
    
    return crc ^ xorOut;
}

quint32 CRCCheckModule::calculateCRC32(const QByteArray &data, quint32 polynomial, quint32 initValue, bool refIn, bool refOut, quint32 xorOut)
{
    quint32 crc = initValue;
    
    for (int i = 0; i < data.size(); ++i) {
        quint8 byte = static_cast<quint8>(data[i]);
        if (refIn) {
            byte = reverseBits(byte);
        }
        crc ^= byte;
        for (int j = 0; j < 8; ++j) {
            if (crc & 0x80000000) {
                crc = (crc << 1) ^ polynomial;
            } else {
                crc <<= 1;
            }
        }
    }
    
    if (refOut) {
        crc = reverseBits32(crc);
    }
    
    return crc ^ xorOut;
}

quint8 CRCCheckModule::calculateCRC8(const QByteArray &data, quint8 polynomial, quint8 initValue, bool refIn, bool refOut, quint8 xorOut)
{
    quint8 crc = initValue;
    
    for (int i = 0; i < data.size(); ++i) {
        quint8 byte = static_cast<quint8>(data[i]);
        if (refIn) {
            byte = reverseBits(byte);
        }
        crc ^= byte;
        for (int j = 0; j < 8; ++j) {
            if (crc & 0x80) {
                crc = (crc << 1) ^ polynomial;
            } else {
                crc <<= 1;
            }
        }
    }
    
    if (refOut) {
        crc = reverseBits(crc);
    }
    
    return crc ^ xorOut;
}

quint8 CRCCheckModule::reverseBits(quint8 byte)
{
    quint8 result = 0;
    for (int i = 0; i < 8; ++i) {
        result = (result << 1) | (byte & 1);
        byte >>= 1;
    }
    return result;
}

quint16 CRCCheckModule::reverseBits16(quint16 value)
{
    quint16 result = 0;
    for (int i = 0; i < 16; ++i) {
        result = (result << 1) | (value & 1);
        value >>= 1;
    }
    return result;
}

quint32 CRCCheckModule::reverseBits32(quint32 value)
{
    quint32 result = 0;
    for (int i = 0; i < 32; ++i) {
        result = (result << 1) | (value & 1);
        value >>= 1;
    }
    return result;
}

void CRCCheckModule::applyCRCPreset(const QString &presetName)
{
    if (presetName == "CRC-8") {
        ui->editPolynomial->setText("0x07");
        ui->editInitValue->setText("0x00");
        ui->editXorOut->setText("0x00");
        ui->checkRefIn->setChecked(false);
        ui->checkRefOut->setChecked(false);
    } else if (presetName == "CRC-16") {
        ui->editPolynomial->setText("0x8005");
        ui->editInitValue->setText("0x0000");
        ui->editXorOut->setText("0x0000");
        ui->checkRefIn->setChecked(false);
        ui->checkRefOut->setChecked(false);
    } else if (presetName == "CRC-32") {
        ui->editPolynomial->setText("0x04C11DB7");
        ui->editInitValue->setText("0xFFFFFFFF");
        ui->editXorOut->setText("0xFFFFFFFF");
        ui->checkRefIn->setChecked(true);
        ui->checkRefOut->setChecked(true);
    }
    
    m_logger->info("CRCCheck", QString("Applied CRC preset: %1").arg(presetName));
}
