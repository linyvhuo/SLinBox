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
    bool agreed = m_config->getValue("General", "UserAgreed", false).toBool();
    
    if (agreed) {
        ui->checkAgree->setChecked(true);
        ui->btnAgree->setEnabled(true);
        m_logger->info("UserAgreement", "User has already agreed to the agreement");
    }
}

void UserAgreementDialog::saveConfig()
{
    m_config->setValue("General", "UserAgreed", ui->checkAgree->isChecked());
    m_config->sync();
}

void UserAgreementDialog::onAgreeClicked()
{
    m_config->setValue("General", "UserAgreed", true);
    m_config->sync();
    
    ui->checkAgree->setChecked(true);
    ui->btnAgree->setEnabled(true);
    
    m_logger->info("UserAgreement", "User agreed to the agreement");
    
    accept();
}

void UserAgreementDialog::onDisagreeClicked()
{
    m_logger->info("UserAgreement", "User disagreed to the agreement");
    
    reject();
}

bool UserAgreementDialog::isAgreed() const
{
    return ui->checkAgree->isChecked();
}
