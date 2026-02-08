#include "delaycalcmodule.h"
#include "ui_delaycalcmodule.h"
#include <QClipboard>
#include <QApplication>
#include <QMessageBox>
#include <QDebug>


DelayCalcModule::DelayCalcModule(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::DelayCalcModule)
    , m_config(ConfigManager::instance())
    , m_logger(Logger::instance())
{
    ui->setupUi(this);
    initUI();
    loadConfig();
}

DelayCalcModule::~DelayCalcModule()
{
    delete ui;
}

void DelayCalcModule::initUI()
{
    connect(ui->btnCalculateForward, &QPushButton::clicked, this, &DelayCalcModule::onCalculateForward);
    connect(ui->btnCalculateReverse, &QPushButton::clicked, this, &DelayCalcModule::onCalculateReverse);
    connect(ui->btnCalculateBaudRate, &QPushButton::clicked, this, &DelayCalcModule::onCalculateBaudRate);
    connect(ui->btnClear, &QPushButton::clicked, this, &DelayCalcModule::onClear);
    connect(ui->btnCopyResult, &QPushButton::clicked, this, &DelayCalcModule::onCopyResult);
}

void DelayCalcModule::loadConfig()
{
    ui->spinClockFrequency->setValue(m_config->getValue("DelayCalc", "ClockFrequency", 100).toInt());
    ui->spinInstructionCycles->setValue(m_config->getValue("DelayCalc", "InstructionCycles", 1).toInt());
}

void DelayCalcModule::saveConfig()
{
    m_config->setValue("DelayCalc", "ClockFrequency", ui->spinClockFrequency->value());
    m_config->setValue("DelayCalc", "InstructionCycles", ui->spinInstructionCycles->value());
    m_config->sync();
}

void DelayCalcModule::onCalculateForward()
{
    double clockFrequency = ui->spinClockFrequency->value();
    int instructionCycles = ui->spinInstructionCycles->value();
    
    if (clockFrequency <= 0) {
        QMessageBox::warning(this, "错误", "时钟频率必须大于0！");
        return;
    }
    
    double delayUs = instructionCycles / clockFrequency;
    double delayMs = delayUs / 1000.0;
    
    QString result = QString("Delay: %1 us = %2 ms = %3 s")
        .arg(delayUs, 0, 'f', 6)
        .arg(delayMs, 0, 'f', 6)
        .arg(delayMs, 0, 'f', 6);
    
    ui->editResult->setText(result);
    
    m_logger->info("DelayCalc", QString("Forward calculation: Clock freq=%1MHz, Instruction cycles=%2, Delay=%3us")
        .arg(clockFrequency).arg(instructionCycles).arg(delayUs, 0, 'f', 6));
}

void DelayCalcModule::onCalculateReverse()
{
    double clockFrequency = ui->spinClockFrequency->value();
    double targetDelay = ui->spinTargetDelay->value();
    QString delayUnit = ui->comboDelayUnit->currentText();
    
    if (clockFrequency <= 0) {
        QMessageBox::warning(this, "错误", "时钟频率必须大于0！");
        return;
    }
    
    double delayUs = targetDelay;
    if (delayUnit == "ms") {
        delayUs = targetDelay * 1000.0;
    } else if (delayUnit == "s") {
        delayUs = targetDelay * 1000000.0;
    }
    
    int instructionCycles = static_cast<int>(delayUs * clockFrequency);
    
    QString result = QString("Instruction cycles: %1").arg(instructionCycles);
    ui->editResult->setText(result);
    
    m_logger->info("DelayCalc", QString("Reverse calculation: Clock freq=%1MHz, Target delay=%2%3, Instruction cycles=%4")
        .arg(clockFrequency).arg(targetDelay).arg(delayUnit).arg(instructionCycles));
}

void DelayCalcModule::onClear()
{
    ui->spinClockFrequency->setValue(100);
    ui->spinInstructionCycles->setValue(1);
    ui->spinTargetDelay->setValue(1.0);
    ui->editResult->clear();
    m_logger->info("DelayCalc", "Clear input and result");
}

void DelayCalcModule::onCopyResult()
{
    QString result = ui->editResult->text();
    if (result.isEmpty()) {
        QMessageBox::warning(this, "警告", "没有结果可以复制！");
        return;
    }
    
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(result);
    
    QMessageBox::information(this, "成功", "结果已复制到剪贴板！");
    m_logger->info("DelayCalc", "Result copied to clipboard");
}

void DelayCalcModule::onCalculateBaudRate()
{
    int baudRate = ui->comboBaudRate->currentText().toInt();
    int dataBits = ui->spinDataBits->value();
    QString stopBitsStr = ui->comboStopBits->currentText();
    QString parityStr = ui->comboParity->currentText();
    
    double stopBits = stopBitsStr.toDouble();
    int parityBits = 0;
    
    if (parityStr == "奇校验" || parityStr == "偶校验") {
        parityBits = 1;
    }
    
    double totalBitsPerByte = 1 + dataBits + parityBits + stopBits;
    double bytesPerSecond = baudRate / totalBitsPerByte;
    double bitsPerSecond = baudRate;
    
    QString result = QString("波特率: %1 bps\n数据位: %2\n停止位: %3\n校验位: %4\n每字节总位数: %5\n传输速率: %6 字节/秒 = %7 bps")
        .arg(baudRate)
        .arg(dataBits)
        .arg(stopBitsStr)
        .arg(parityStr)
        .arg(totalBitsPerByte, 0, 'f', 2)
        .arg(bytesPerSecond, 0, 'f', 2)
        .arg(bitsPerSecond);
    
    ui->editResult->setText(result);
    
    m_logger->info("DelayCalc", QString("Baud rate calculation: %1 bps, %2 bits/byte, %3 bytes/s")
        .arg(baudRate).arg(totalBitsPerByte, 0, 'f', 2).arg(bytesPerSecond, 0, 'f', 2));
}
