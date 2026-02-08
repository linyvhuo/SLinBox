#ifndef SCREENSHOTOCRMODULE_H
#define SCREENSHOTOCRMODULE_H

#include <QWidget>
#include <QPixmap>
#include <QPoint>
#include <QRect>
#include <QTimer>
#include <QShortcut>
#include <QPainter>
#include <QPixmap>
#include <QDialog>
#include <QShowEvent>
#include "core/configmanager.h"
#include "core/logger.h"
#include "core/screenhelper.h"
#include "core/globalhotkeymanager.h"

QT_BEGIN_NAMESPACE
namespace Ui { class ScreenshotOCRModule; }
QT_END_NAMESPACE

class CaptureDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CaptureDialog(QWidget *parent = nullptr);
    ~CaptureDialog();

    void setBackground(const QPixmap &background);
    QRect getCaptureRect() const;

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

signals:
    void captureCompleted(const QRect &rect);
    void captureCancelled();

private:
    QPixmap m_background;
    QPoint m_startPos;
    QPoint m_endPos;
    bool m_isDrawing;
};

class ScreenshotOCRModule : public QWidget
{
    Q_OBJECT

public:
    explicit ScreenshotOCRModule(QWidget *parent = nullptr);
    ~ScreenshotOCRModule();

    void loadConfig();
    void saveConfig();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;
    void showEvent(QShowEvent *event) override;

private slots:
    void onCaptureFullScreen();
    void onCaptureRegion();
    void onOcrScreenshot();
    void onClipboardOcr();
    void onSaveScreenshot();
    void onCopyScreenshot();
    void onClearAnnotation();
    void onExportResult();
    void onHotkeyTriggered();
    void onHotkeyChanged(const QKeySequence &keySequence);
    void onOcrHotkeyTriggered();
    void onClipboardOcrHotkeyTriggered();
    void onCaptureCompleted(const QRect &rect);
    void onCaptureCancelled();

private:
    void initUI();
    void captureScreen();
    void captureRegion();
    void drawAnnotation(const QPoint &start, const QPoint &end, int type);
    void setupHotkey();
    void updateAnnotationPreview();
    void startRegionCapture();
    void finishRegionCapture();

    Ui::ScreenshotOCRModule *ui;

    ConfigManager *m_config;
    Logger *m_logger;

    QPixmap m_screenshot;
    QPixmap m_originalScreenshot;
    bool m_isCapturing;
    QPoint m_captureStart;
    QPoint m_captureEnd;
    bool m_isDrawing;
    QPoint m_drawStart;
    QPoint m_drawEnd;
    int m_annotationType;
    QList<QPair<QPoint, QPoint>> m_annotations;
    QShortcut *m_screenshotShortcut;
    QShortcut *m_ocrShortcut;
    QShortcut *m_clipboardOcrShortcut;
    
    CaptureDialog *m_captureDialog;
    QPixmap m_captureBackground;
    QRect m_captureRect;
    bool m_hotkeyRegistered;
};

#endif // SCREENSHOTOCRMODULE_H