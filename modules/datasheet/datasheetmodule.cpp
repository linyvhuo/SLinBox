#include "datasheetmodule.h"
#include "ui_datasheetmodule.h"
#include <QDesktopServices>
#include <QUrl>
#include <QMessageBox>
#include <QDebug>


DataSheetModule::DataSheetModule(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::DataSheetModule)
    , m_config(ConfigManager::instance())
    , m_logger(Logger::instance())
{
    ui->setupUi(this);
    initUI();
    loadConfig();
}

DataSheetModule::~DataSheetModule()
{
    delete ui;
}

void DataSheetModule::initUI()
{
    connect(ui->btnOpenDataSheet, &QPushButton::clicked, this, &DataSheetModule::onOpenDataSheet);
}

void DataSheetModule::loadConfig()
{
}

void DataSheetModule::saveConfig()
{
    m_config->sync();
}

void DataSheetModule::onOpenDataSheet()
{
    QString partNumber = ui->editPartNumber->text().trimmed();
    if (partNumber.isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入元器件型号！");
        return;
    }
    
    QString url = QString("https://www.semiee.com/search?searchModel=%1").arg(partNumber);
    
    QDesktopServices::openUrl(QUrl(url));
    
    m_logger->info("DataSheet", QString("打开数据手册: %1").arg(partNumber));
}
