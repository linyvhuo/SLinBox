#include "useragreementdialog.h"
#include "ui_useragreementdialog.h"
#include "core/logger.h"

UserAgreementDialog::UserAgreementDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::UserAgreementDialog)
    , m_config(ConfigManager::instance())
{
    m_logger = Logger::instance();
    
    ui->setupUi(this);
    initUI();
    loadConfig();
}

UserAgreementDialog::~UserAgreementDialog()
{
    saveConfig();
    delete ui;
}

void UserAgreementDialog::initUI()
{
    ui->btnAgree->setEnabled(false);
    connect(ui->checkAgree, &QCheckBox::stateChanged, this, [this](int state) {
        ui->btnAgree->setEnabled(state == Qt::Checked);
    });
    connect(ui->btnAgree, &QPushButton::clicked, this, &UserAgreementDialog::onAgreeClicked);
    connect(ui->btnDisagree, &QPushButton::clicked, this, &UserAgreementDialog::onDisagreeClicked);
}

void UserAgreementDialog::loadConfig()
{
    bool agreed = m_config->getValue("General", "用户同意", false).toBool();
    
    if (agreed) {
        ui->checkAgree->setChecked(true);
        ui->btnAgree->setEnabled(true);
        m_logger->info("用户协议", "用户已同意本协议");
    }
}

void UserAgreementDialog::saveConfig()
{
    m_config->setValue("General", "用户同意", ui->checkAgree->isChecked());
    m_config->sync();
}

void UserAgreementDialog::onAgreeClicked()
{
    m_config->setValue("General", "用户同意", true);
    m_config->sync();
    
    ui->checkAgree->setChecked(true);
    ui->btnAgree->setEnabled(true);
    
    m_logger->info("用户协议", "用户同意本协议");
    
    accept();
}

void UserAgreementDialog::onDisagreeClicked()
{
    m_logger->info("用户协议", "用户不同意本协议");
    
    reject();
}

bool UserAgreementDialog::isAgreed() const
{
    return ui->checkAgree->isChecked();
}
