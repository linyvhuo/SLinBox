/********************************************************************************
** Form generated from reading UI file 'useragreementdialog.ui'
**
** Created by: Qt User Interface Compiler version 5.14.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_USERAGREEMENTDIALOG_H
#define UI_USERAGREEMENTDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_UserAgreementDialog
{
public:
    QVBoxLayout *verticalLayout;
    QLabel *labelTitle;
    QTextEdit *textAgreement;
    QCheckBox *checkAgree;
    QWidget *buttonWidget;
    QHBoxLayout *buttonLayout;
    QSpacerItem *spacerItem;
    QPushButton *btnAgree;
    QPushButton *btnDisagree;

    void setupUi(QDialog *UserAgreementDialog)
    {
        if (UserAgreementDialog->objectName().isEmpty())
            UserAgreementDialog->setObjectName(QString::fromUtf8("UserAgreementDialog"));
        UserAgreementDialog->resize(600, 500);
        verticalLayout = new QVBoxLayout(UserAgreementDialog);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        labelTitle = new QLabel(UserAgreementDialog);
        labelTitle->setObjectName(QString::fromUtf8("labelTitle"));
        labelTitle->setWordWrap(true);

        verticalLayout->addWidget(labelTitle);

        textAgreement = new QTextEdit(UserAgreementDialog);
        textAgreement->setObjectName(QString::fromUtf8("textAgreement"));
        textAgreement->setReadOnly(true);

        verticalLayout->addWidget(textAgreement);

        checkAgree = new QCheckBox(UserAgreementDialog);
        checkAgree->setObjectName(QString::fromUtf8("checkAgree"));

        verticalLayout->addWidget(checkAgree);

        buttonWidget = new QWidget(UserAgreementDialog);
        buttonWidget->setObjectName(QString::fromUtf8("buttonWidget"));
        buttonLayout = new QHBoxLayout(buttonWidget);
        buttonLayout->setObjectName(QString::fromUtf8("buttonLayout"));
        buttonLayout->setContentsMargins(0, 0, 0, 0);
        spacerItem = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        buttonLayout->addItem(spacerItem);

        btnAgree = new QPushButton(buttonWidget);
        btnAgree->setObjectName(QString::fromUtf8("btnAgree"));
        btnAgree->setEnabled(false);

        buttonLayout->addWidget(btnAgree);

        btnDisagree = new QPushButton(buttonWidget);
        btnDisagree->setObjectName(QString::fromUtf8("btnDisagree"));

        buttonLayout->addWidget(btnDisagree);


        verticalLayout->addWidget(buttonWidget);


        retranslateUi(UserAgreementDialog);

        QMetaObject::connectSlotsByName(UserAgreementDialog);
    } // setupUi

    void retranslateUi(QDialog *UserAgreementDialog)
    {
        UserAgreementDialog->setWindowTitle(QCoreApplication::translate("UserAgreementDialog", "User Agreement", nullptr));
        labelTitle->setText(QCoreApplication::translate("UserAgreementDialog", "<h2 style=\"color: #2c3e50; margin-bottom: 20px;\">SLinBox User Agreement</h2>", nullptr));
        textAgreement->setHtml(QCoreApplication::translate("UserAgreementDialog", "<!DOCTYPE html>\n"
"<html>\n"
"<head>\n"
"<style>\n"
"  body { font-family: Arial, sans-serif; font-size: 12px; line-height: 1.6; color: #333; }\n"
"  h3 { color: #2c3e50; margin-bottom: 15px; }\n"
"  ul { margin-left: 20px; }\n"
"  li { margin-bottom: 10px; }\n"
"</style>\n"
"</head>\n"
"<body>\n"
"<h3>1. Introduction</h3>\n"
"<p>SLinBox is a comprehensive intelligent toolbox software designed to provide various utilities for developers and users.</p>\n"
"\n"
"<h3>2. Usage</h3>\n"
"<ul>\n"
"  <li>You may use this software for personal and commercial purposes.</li>\n"
"  <li>You may not reverse engineer, decompile, or disassemble this software.</li>\n"
"  <li>You may not use this software for any illegal purposes.</li>\n"
"</ul>\n"
"\n"
"<h3>3. Privacy Policy</h3>\n"
"<ul>\n"
"  <li>This software does not collect any personal data without your explicit consent.</li>\n"
"  <li>All data is stored locally on your device.</li>\n"
"  <li>No data is transmitted to any third-party servers.</li>\n"
"</ul>\n"
"\n"
"<h3"
                        ">4. Disclaimer</h3>\n"
"<ul>\n"
"  <li>This software is provided \"as is\" without warranty of any kind.</li>\n"
"  <li>The authors are not responsible for any damages arising from the use of this software.</li>\n"
"  <li>In no event shall the authors be liable for any claim, damages or other liability.</li>\n"
"</ul>\n"
"\n"
"<h3>5. Agreement</h3>\n"
"<p>By clicking \"I Agree\" below, you acknowledge that you have read, understood, and agree to be bound by the terms and conditions of this agreement.</p>\n"
"</body>\n"
"</html>", nullptr));
        checkAgree->setText(QCoreApplication::translate("UserAgreementDialog", "I have read and agree to the terms and conditions", nullptr));
        btnAgree->setText(QCoreApplication::translate("UserAgreementDialog", "I Agree", nullptr));
        btnDisagree->setText(QCoreApplication::translate("UserAgreementDialog", "Disagree", nullptr));
    } // retranslateUi

};

namespace Ui {
    class UserAgreementDialog: public Ui_UserAgreementDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_USERAGREEMENTDIALOG_H
