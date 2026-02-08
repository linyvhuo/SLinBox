#ifndef HOTKEYSETTINGDIALOG_H
#define HOTKEYSETTINGDIALOG_H

#include <QDialog>
#include <QKeySequence>

namespace Ui {
class HotkeySettingDialog;
}

class HotkeySettingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit HotkeySettingDialog(const QKeySequence &currentHotkey, QWidget *parent = nullptr);
    ~HotkeySettingDialog();
    
    QKeySequence getHotkey() const;

protected:
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void onRecordHotkey();
    void onClearHotkey();
    void onAccept();
    void onCancel();

private:
    Ui::HotkeySettingDialog *ui;
    QKeySequence m_hotkey;
    bool m_recording;
};

#endif // HOTKEYSETTINGDIALOG_H
