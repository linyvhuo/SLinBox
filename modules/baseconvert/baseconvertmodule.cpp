#include "baseconvertmodule.h"
#include "ui_baseconvertmodule.h"
#include <QClipboard>
#include <QApplication>
#include <QMessageBox>
#include <QDebug>


BaseConvertModule::BaseConvertModule(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::BaseConvertModule)
    , m_config(ConfigManager::instance())
    , m_logger(Logger::instance())
{
    ui->setupUi(this);
    initUI();
    loadConfig();
}

BaseConvertModule::~BaseConvertModule()
{
    delete ui;
}

void BaseConvertModule::initUI()
{
    connect(ui->btnConvert, &QPushButton::clicked, this, &BaseConvertModule::onConvert);
    connect(ui->btnClear, &QPushButton::clicked, this, &BaseConvertModule::onClear);
    connect(ui->btnCopyResult, &QPushButton::clicked, this, &BaseConvertModule::onCopyResult);
    connect(ui->comboSourceBase, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &BaseConvertModule::onSourceBaseChanged);
    connect(ui->comboTargetBase, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &BaseConvertModule::onTargetBaseChanged);
    connect(ui->editInput, &QLineEdit::textChanged, this, &BaseConvertModule::onInputChanged);
}

void BaseConvertModule::loadConfig()
{
    int sourceBase = m_config->getValue("BaseConvert", "SourceBase", 10).toInt();
    int targetBase = m_config->getValue("BaseConvert", "TargetBase", 16).toInt();
    bool paddingZero = m_config->getValue("BaseConvert", "PaddingZero", false).toBool();
    
    ui->comboSourceBase->setCurrentIndex(sourceBase == 2 ? 0 : sourceBase == 8 ? 1 : sourceBase == 10 ? 2 : 3);
    ui->comboTargetBase->setCurrentIndex(targetBase == 2 ? 0 : targetBase == 8 ? 1 : targetBase == 10 ? 2 : 3);
    ui->checkPaddingZero->setChecked(paddingZero);
}

void BaseConvertModule::saveConfig()
{
    int sourceBase = ui->comboSourceBase->currentIndex() == 0 ? 2 : 
                     ui->comboSourceBase->currentIndex() == 1 ? 8 : 
                     ui->comboSourceBase->currentIndex() == 2 ? 10 : 16;
    int targetBase = ui->comboTargetBase->currentIndex() == 0 ? 2 : 
                     ui->comboTargetBase->currentIndex() == 1 ? 8 : 
                     ui->comboTargetBase->currentIndex() == 2 ? 10 : 16;
    
    m_config->setValue("BaseConvert", "SourceBase", sourceBase);
    m_config->setValue("BaseConvert", "TargetBase", targetBase);
    m_config->setValue("BaseConvert", "PaddingZero", ui->checkPaddingZero->isChecked());
    m_config->sync();
}

void BaseConvertModule::onConvert()
{
    QString input = ui->editInput->text().trimmed();
    if (input.isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入一个值来转换！");
        return;
    }
    
    int sourceBase = ui->comboSourceBase->currentIndex() == 0 ? 2 : 
                     ui->comboSourceBase->currentIndex() == 1 ? 8 : 
                     ui->comboSourceBase->currentIndex() == 2 ? 10 : 16;
    int targetBase = ui->comboTargetBase->currentIndex() == 0 ? 2 : 
                     ui->comboTargetBase->currentIndex() == 1 ? 8 : 
                     ui->comboTargetBase->currentIndex() == 2 ? 10 : 16;
    
    if (!isValidNumber(input, sourceBase)) {
        QMessageBox::warning(this, "错误", "无效的输入格式！");
        m_logger->error("BaseConvert", "Invalid input format");
        return;
    }
    
    QString result = convertBase(input, sourceBase, targetBase);
    ui->editResult->setText(result);
    
    m_logger->info("BaseConvert", QString("Convert successful: %1 (base %2) -> %3 (base %4)")
        .arg(input).arg(sourceBase).arg(result).arg(targetBase));
}

void BaseConvertModule::onClear()
{
    ui->editInput->clear();
    ui->editResult->clear();
    m_logger->info("BaseConvert", "Clear input and result");
}

void BaseConvertModule::onCopyResult()
{
    QString result = ui->editResult->text();
    if (result.isEmpty()) {
        QMessageBox::warning(this, "警告", "没有结果可以复制！");
        return;
    }
    
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(result);
    
    QMessageBox::information(this, "成功", "结果已复制到剪贴板！");
    m_logger->info("BaseConvert", "Result copied to clipboard");
}

void BaseConvertModule::onSourceBaseChanged(int index)
{
    Q_UNUSED(index);
    saveConfig();
}

void BaseConvertModule::onTargetBaseChanged(int index)
{
    Q_UNUSED(index);
    saveConfig();
}

void BaseConvertModule::onInputChanged(const QString &text)
{
    if (text.isEmpty()) {
        ui->editResult->clear();
        return;
    }
    
    int sourceBase = ui->comboSourceBase->currentIndex() == 0 ? 2 : 
                     ui->comboSourceBase->currentIndex() == 1 ? 8 : 
                     ui->comboSourceBase->currentIndex() == 2 ? 10 : 16;
    int targetBase = ui->comboTargetBase->currentIndex() == 0 ? 2 : 
                     ui->comboTargetBase->currentIndex() == 1 ? 8 : 
                     ui->comboTargetBase->currentIndex() == 2 ? 10 : 16;
    
    if (!isValidNumber(text, sourceBase)) {
        return;
    }
    
    QString result = convertBase(text, sourceBase, targetBase);
    ui->editResult->setText(result);
}

QString BaseConvertModule::convertBase(const QString &input, int fromBase, int toBase)
{
    bool ok;
    qint64 value = input.toLongLong(&ok, fromBase);
    
    if (!ok) {
        return QString();
    }
    
    QString result = QString::number(value, toBase).toUpper();
    
    if (ui->checkPaddingZero->isChecked() && toBase == 2) {
        while (result.length() % 8 != 0) {
            result = "0" + result;
        }
    } else if (ui->checkPaddingZero->isChecked() && toBase == 16) {
        while (result.length() % 2 != 0) {
            result = "0" + result;
        }
    }
    
    return result;
}

bool BaseConvertModule::isValidNumber(const QString &input, int base)
{
    if (input.isEmpty()) {
        return false;
    }
    
    for (int i = 0; i < input.length(); ++i) {
        QChar ch = input[i].toUpper();
        int digitValue;
        
        if (ch >= '0' && ch <= '9') {
            digitValue = ch.digitValue();
        } else if (ch >= 'A' && ch <= 'F') {
            digitValue = ch.toLatin1() - 'A' + 10;
        } else {
            return false;
        }
        
        if (digitValue >= base) {
            return false;
        }
    }
    
    return true;
}
