#include "asciiquerymodule.h"
#include "ui_asciiquerymodule.h"
#include <QClipboard>
#include <QApplication>
#include <QMessageBox>
#include <QDebug>


ASCIIQueryModule::ASCIIQueryModule(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ASCIIQueryModule)
    , m_config(ConfigManager::instance())
    , m_logger(Logger::instance())
{
    ui->setupUi(this);
    initUI();
    loadConfig();
}

ASCIIQueryModule::~ASCIIQueryModule()
{
    delete ui;
}

void ASCIIQueryModule::initUI()
{
    connect(ui->btnCharToCode, &QPushButton::clicked, this, &ASCIIQueryModule::onCharToCode);
    connect(ui->btnCodeToChar, &QPushButton::clicked, this, &ASCIIQueryModule::onCodeToChar);
    connect(ui->btnClear, &QPushButton::clicked, this, &ASCIIQueryModule::onClear);
    connect(ui->btnShowTable, &QPushButton::clicked, this, &ASCIIQueryModule::onShowTable);
}

void ASCIIQueryModule::loadConfig()
{
}

void ASCIIQueryModule::saveConfig()
{
}

void ASCIIQueryModule::onCharToCode()
{
    QString input = ui->editChar->text().trimmed();
    if (input.isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入一个字符！");
        return;
    }
    
    QChar ch = input[0];
    int decimal = ch.unicode();
    QString hex = QString("%1").arg(decimal, 2, 16, QChar('0')).toUpper();
    QString octal = QString("%1").arg(decimal, 3, 8, QChar('0'));
    QString binary = QString("%1").arg(decimal, 8, 2, QChar('0'));
    
    QString result = QString("Character: %1\nDecimal: %2\nHex: %3\nOctal: %4\nBinary: %5")
        .arg(ch).arg(decimal).arg(hex).arg(octal).arg(binary);
    
    ui->editResult->setPlainText(result);
    
    m_logger->info("ASCIIQuery", QString("Character to code: %1 -> %2").arg(ch).arg(decimal));
}

void ASCIIQueryModule::onCodeToChar()
{
    QString input = ui->editCode->text().trimmed();
    if (input.isEmpty()) {
        QMessageBox::warning(this, "Warning", "Please enter a code!");
        return;
    }
    
    bool ok;
    int code;
    
    if (input.startsWith("0x") || input.startsWith("0X")) {
        code = input.toInt(&ok, 16);
    } else if (input.startsWith("0b") || input.startsWith("0B")) {
        code = input.mid(2).toInt(&ok, 2);
    } else if (input.startsWith("0o") || input.startsWith("0O")) {
        code = input.mid(2).toInt(&ok, 8);
    } else {
        code = input.toInt(&ok, 10);
    }
    
    if (!ok || code < 0 || code > 255) {
        QMessageBox::warning(this, "错误", "无效的ASCII代码（范围0-255）！");
        m_logger->error("ASCIIQuery", "Invalid ASCII code");
        return;
    }
    
    QChar ch(code);
    QString hex = QString("%1").arg(code, 2, 16, QChar('0')).toUpper();
    QString octal = QString("%1").arg(code, 3, 8, QChar('0'));
    QString binary = QString("%1").arg(code, 8, 2, QChar('0'));
    
    QString type;
    QString encoding;
    if (code < 32) {
        type = "Control";
        encoding = "ASCII";
    } else if (code < 127) {
        type = "Printable";
        encoding = "ASCII";
    } else if (code < 160) {
        type = "Extended";
        encoding = "Extended ASCII";
    } else if (code < 192) {
        type = "Latin-1 Supplement";
        encoding = "ISO-8859-1";
    } else {
        type = "Latin-1 Extended";
        encoding = "ISO-8859-1";
    }
    
    QString displayChar;
    if (code < 32) {
        displayChar = QString("CTRL+%1").arg(QChar('A' + code));
    } else if (code == 127) {
        displayChar = "DEL";
    } else if (code < 160) {
        displayChar = QString(ch);
    } else {
        displayChar = QString(ch);
    }
    
    QString result = QString("Character: %1\nDecimal: %2\nHex: %3\nOctal: %4\nBinary: %5\nType: %6\nEncoding: %7")
        .arg(displayChar).arg(code).arg(hex).arg(octal).arg(binary).arg(type).arg(encoding);
    
    ui->editResult->setPlainText(result);
    
    m_logger->info("ASCIIQuery", QString("Code to character: %1 -> %2").arg(code).arg(ch));
}

void ASCIIQueryModule::onClear()
{
    ui->editChar->clear();
    ui->editCode->clear();
    ui->editResult->clear();
    m_logger->info("ASCIIQuery", "Clear input and result");
}

void ASCIIQueryModule::onShowTable()
{
    showASCIITable();
}

void ASCIIQueryModule::showASCIITable()
{
    QString table;
    table += "ASCII Table (Extended)\n";
    table += "------------------------------------------------------------\n";
    table += "DEC  HEX  OCT  BIN    CHAR  TYPE        ENCODING\n";
    table += "------------------------------------------------------------\n";
    
    for (int i = 0; i < 256; ++i) {
        QChar ch(i);
        QString hex = QString("%1").arg(i, 2, 16, QChar('0')).toUpper();
        QString octal = QString("%1").arg(i, 3, 8, QChar('0'));
        QString binary = QString("%1").arg(i, 8, 2, QChar('0'));
        
        QString type;
        QString encoding;
        if (i < 32) {
            type = "Control";
            encoding = "ASCII";
        } else if (i < 127) {
            type = "Printable";
            encoding = "ASCII";
        } else if (i < 160) {
            type = "Extended";
            encoding = "Extended ASCII";
        } else if (i < 192) {
            type = "Latin-1 Supplement";
            encoding = "ISO-8859-1";
        } else {
            type = "Latin-1 Extended";
            encoding = "ISO-8859-1";
        }
        
        QString displayChar;
        if (i < 32) {
            displayChar = QString("CTRL+%1").arg(QChar('A' + i));
        } else if (i == 127) {
            displayChar = "DEL";
        } else if (i < 160) {
            displayChar = QString(ch);
        } else {
            displayChar = QString(ch);
        }
        
        table += QString("%1    %2    %3    %4     %5     %6     %7\n")
            .arg(i, 3).arg(hex, 3).arg(octal, 3).arg(binary, 8).arg(displayChar, 5).arg(type, 18).arg(encoding, 15);
    }
    
    ui->editResult->setPlainText(table);
    m_logger->info("ASCIIQuery", "Show extended ASCII table");
}
