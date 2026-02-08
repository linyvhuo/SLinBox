#include "hotkeysettingdialog.h"
#include "ui_hotkeysettingdialog.h"
#include <QKeyEvent>
#include <QPushButton>

HotkeySettingDialog::HotkeySettingDialog(const QKeySequence &currentHotkey, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::HotkeySettingDialog)
    , m_hotkey(currentHotkey)
    , m_recording(false)
{
    ui->setupUi(this);
    
    ui->editHotkey->setText(m_hotkey.toString());
    ui->editHotkey->setReadOnly(true);
    
    connect(ui->btnRecord, &QPushButton::clicked, this, &HotkeySettingDialog::onRecordHotkey);
    connect(ui->btnClear, &QPushButton::clicked, this, &HotkeySettingDialog::onClearHotkey);
    connect(ui->btnOK, &QPushButton::clicked, this, &HotkeySettingDialog::onAccept);
    connect(ui->btnCancel, &QPushButton::clicked, this, &HotkeySettingDialog::onCancel);
}

HotkeySettingDialog::~HotkeySettingDialog()
{
    delete ui;
}

QKeySequence HotkeySettingDialog::getHotkey() const
{
    return m_hotkey;
}

void HotkeySettingDialog::keyPressEvent(QKeyEvent *event)
{
    if (!m_recording) {
        QDialog::keyPressEvent(event);
        return;
    }
    
    if (event->key() == Qt::Key_Escape) {
        onCancel();
        return;
    }
    
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        onAccept();
        return;
    }
    
    if (event->modifiers() != Qt::NoModifier || 
        (event->key() >= Qt::Key_F1 && event->key() <= Qt::Key_F35) ||
        event->key() == Qt::Key_Space ||
        event->key() == Qt::Key_Tab) {
        
        QKeySequence keySequence(event->key() | event->modifiers());
        m_hotkey = keySequence;
        ui->editHotkey->setText(m_hotkey.toString());
        m_recording = false;
        ui->btnRecord->setText("录制快捷键");
    }
}

void HotkeySettingDialog::onRecordHotkey()
{
    m_recording = true;
    ui->editHotkey->setText("请按下快捷键...");
    ui->btnRecord->setText("正在录制...");
    ui->editHotkey->setFocus();
}

void HotkeySettingDialog::onClearHotkey()
{
    m_hotkey = QKeySequence();
    ui->editHotkey->clear();
}

void HotkeySettingDialog::onAccept()
{
    accept();
}

void HotkeySettingDialog::onCancel()
{
    reject();
}
