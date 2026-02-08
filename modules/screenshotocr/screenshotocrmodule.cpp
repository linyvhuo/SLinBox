#include "screenshotocrmodule.h"
#include "ui_screenshotocrmodule.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QClipboard>
#include <QApplication>
#include <QPainter>
#include <QMouseEvent>
#include <QProcess>
#include <QStandardPaths>
#include <QDebug>
#include <QDesktopWidget>
#include <QScreen>
#include <QShortcut>
#include <QShowEvent>
#include <QKeySequenceEdit>
#include <QMenu>
#include "ui/screenshoteditorwindow.h"


CaptureDialog::CaptureDialog(QWidget *parent)
    : QDialog(parent)
    , m_isDrawing(false)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);
    setCursor(Qt::CrossCursor);
}

CaptureDialog::~CaptureDialog()
{
}

void CaptureDialog::setBackground(const QPixmap &background)
{
    m_background = background;
    setGeometry(background.rect());
}

QRect CaptureDialog::getCaptureRect() const
{
    QRect rect(m_startPos, m_endPos);
    return rect.normalized();
}

void CaptureDialog::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    
    QPainter painter(this);
    painter.drawPixmap(0, 0, m_background);
    
    if (m_isDrawing) {
        QPen pen;
        pen.setColor(Qt::red);
        pen.setWidth(2);
        pen.setStyle(Qt::DashLine);
        painter.setPen(pen);
        
        QRect selectionRect(m_startPos, m_endPos);
        painter.drawRect(selectionRect);
        painter.fillRect(selectionRect, QColor(0, 0, 0, 50));
        
        QString sizeText = QString("%1 x %2").arg(qAbs(m_endPos.x() - m_startPos.x())).arg(qAbs(m_endPos.y() - m_startPos.y()));
        painter.setPen(Qt::white);
        painter.drawText(selectionRect.bottomLeft() + QPoint(5, -5), sizeText);
    }
}

void CaptureDialog::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_startPos = event->pos();
        m_endPos = m_startPos;
        m_isDrawing = true;
    }
}

void CaptureDialog::mouseMoveEvent(QMouseEvent *event)
{
    if (m_isDrawing) {
        m_endPos = event->pos();
        update();
    }
}

void CaptureDialog::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && m_isDrawing) {
        m_isDrawing = false;
        
        QRect selectionRect(m_startPos, m_endPos);
        selectionRect = selectionRect.normalized();
        
        if (selectionRect.width() > 10 && selectionRect.height() > 10) {
            emit captureCompleted(selectionRect);
            close();
        } else {
            emit captureCancelled();
        }
    }
}

void CaptureDialog::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        emit captureCancelled();
        close();
    }
}

ScreenshotOCRModule::ScreenshotOCRModule(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ScreenshotOCRModule)
    , m_config(ConfigManager::instance())
    , m_logger(Logger::instance())
    , m_isCapturing(false)
    , m_isDrawing(false)
    , m_annotationType(0)
    , m_screenshotShortcut(nullptr)
    , m_captureDialog(nullptr)
    , m_hotkeyRegistered(false)
{
    ui->setupUi(this);
    initUI();
    loadConfig();
}

ScreenshotOCRModule::~ScreenshotOCRModule()
{
    if (m_captureDialog) {
        delete m_captureDialog;
    }
    delete ui;
}

void ScreenshotOCRModule::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    
    if (!m_hotkeyRegistered) {
        GlobalHotkeyManager *hotkeyManager = GlobalHotkeyManager::instance();
        hotkeyManager->setWindowHandle(this);
        setupHotkey();
    }
}

void ScreenshotOCRModule::initUI()
{
    connect(ui->btnCaptureFullScreen, &QPushButton::clicked, this, &ScreenshotOCRModule::onCaptureFullScreen);
    connect(ui->btnCaptureRegion, &QPushButton::clicked, this, &ScreenshotOCRModule::onCaptureRegion);
    connect(ui->btnOcr, &QPushButton::clicked, this, &ScreenshotOCRModule::onOcrScreenshot);
    connect(ui->btnClipboardOcr, &QPushButton::clicked, this, &ScreenshotOCRModule::onClipboardOcr);
    connect(ui->btnSave, &QPushButton::clicked, this, &ScreenshotOCRModule::onSaveScreenshot);
    connect(ui->btnCopy, &QPushButton::clicked, this, &ScreenshotOCRModule::onCopyScreenshot);
    connect(ui->btnClearAnnotation, &QPushButton::clicked, this, &ScreenshotOCRModule::onClearAnnotation);
    connect(ui->btnExportResult, &QPushButton::clicked, this, &ScreenshotOCRModule::onExportResult);
    connect(ui->keySequenceHotkey, &QKeySequenceEdit::keySequenceChanged, this, &ScreenshotOCRModule::onHotkeyChanged);
    
    ui->labelScreenshot->setAlignment(Qt::AlignCenter);
    ui->labelScreenshot->setStyleSheet("border: 2px dashed #cccccc; background-color: #f5f5f5;");
    ui->labelScreenshot->setText("请先截图");
    
    ui->editOcrResult->setReadOnly(true);
}

void ScreenshotOCRModule::setupHotkey()
{
    GlobalHotkeyManager *hotkeyManager = GlobalHotkeyManager::instance();
    
    QKeySequence hotkey = ui->keySequenceHotkey->keySequence();
    
    if (!hotkey.isEmpty()) {
        m_logger->info("ScreenshotOCR", QString("尝试注册全局热键: %1").arg(hotkey.toString()));
        if (hotkeyManager->registerHotkey(hotkey, this, SLOT(onHotkeyTriggered()))) {
            m_hotkeyRegistered = true;
            m_logger->info("ScreenshotOCR", QString("全局热键已设置: %1").arg(hotkey.toString()));
        } else {
            m_hotkeyRegistered = false;
            m_logger->error("ScreenshotOCR", QString("全局热键注册失败: %1").arg(hotkey.toString()));
            QMessageBox::warning(this, "热键注册失败", 
                QString("无法注册快捷键 %1，请尝试其他快捷键组合。\n\n可能原因：\n1. 快捷键已被其他程序占用\n2. 快捷键格式不正确\n3. 窗口句柄未初始化").arg(hotkey.toString()));
        }
    } else {
        m_logger->warn("ScreenshotOCR", "快捷键为空，跳过注册");
    }
    
    QKeySequence ocrHotkey = QKeySequence(m_config->getValue("ScreenshotOCR", "OcrHotkey", "Ctrl+Shift+O").toString());
    if (!ocrHotkey.isEmpty()) {
        m_logger->info("ScreenshotOCR", QString("尝试注册OCR识别快捷键: %1").arg(ocrHotkey.toString()));
        if (hotkeyManager->registerHotkey(ocrHotkey, this, SLOT(onOcrHotkeyTriggered()))) {
            m_logger->info("ScreenshotOCR", QString("OCR识别快捷键已设置: %1").arg(ocrHotkey.toString()));
        } else {
            m_logger->error("ScreenshotOCR", QString("OCR识别快捷键注册失败: %1").arg(ocrHotkey.toString()));
        }
    }
    
    QKeySequence clipboardOcrHotkey = QKeySequence(m_config->getValue("ScreenshotOCR", "ClipboardOcrHotkey", "Ctrl+Shift+C").toString());
    if (!clipboardOcrHotkey.isEmpty()) {
        m_logger->info("ScreenshotOCR", QString("尝试注册剪贴板识别快捷键: %1").arg(clipboardOcrHotkey.toString()));
        if (hotkeyManager->registerHotkey(clipboardOcrHotkey, this, SLOT(onClipboardOcrHotkeyTriggered()))) {
            m_logger->info("ScreenshotOCR", QString("剪贴板识别快捷键已设置: %1").arg(clipboardOcrHotkey.toString()));
        } else {
            m_logger->error("ScreenshotOCR", QString("剪贴板识别快捷键注册失败: %1").arg(clipboardOcrHotkey.toString()));
        }
    }
}

void ScreenshotOCRModule::loadConfig()
{
    QString hotkeyStr = m_config->getValue("ScreenshotOCR", "ScreenshotHotkey", "Ctrl+Shift+S").toString();
    ui->keySequenceHotkey->setKeySequence(QKeySequence(hotkeyStr));
    QString ocrHotkeyStr = m_config->getValue("ScreenshotOCR", "OcrHotkey", "Ctrl+Shift+O").toString();
    QString clipboardOcrHotkeyStr = m_config->getValue("ScreenshotOCR", "ClipboardOcrHotkey", "Ctrl+Shift+C").toString();
    ui->spinOcrConfidence->setValue(m_config->getValue("ScreenshotOCR", "OcrConfidence", 0.7).toDouble());
    ui->comboSaveFormat->setCurrentText(m_config->getValue("ScreenshotOCR", "DefaultSaveFormat", "PNG").toString());
    ui->editSavePath->setText(m_config->getValue("ScreenshotOCR", "SavePath", QStandardPaths::writableLocation(QStandardPaths::DesktopLocation)).toString());
    ui->checkAutoOcr->setChecked(m_config->getValue("ScreenshotOCR", "AutoOcrAfterScreenshot", true).toBool());
    
    m_logger->info("ScreenshotOCR", QString("配置已加载，快捷键: %1").arg(hotkeyStr));
}

void ScreenshotOCRModule::saveConfig()
{
    m_config->setValue("ScreenshotOCR", "ScreenshotHotkey", ui->keySequenceHotkey->keySequence().toString());
    m_config->setValue("ScreenshotOCR", "OcrConfidence", ui->spinOcrConfidence->value());
    m_config->setValue("ScreenshotOCR", "DefaultSaveFormat", ui->comboSaveFormat->currentText());
    m_config->setValue("ScreenshotOCR", "SavePath", ui->editSavePath->text());
    m_config->setValue("ScreenshotOCR", "AutoOcrAfterScreenshot", ui->checkAutoOcr->isChecked());
    m_config->sync();
}

void ScreenshotOCRModule::onCaptureFullScreen()
{
    saveConfig();
    captureScreen();
}

void ScreenshotOCRModule::onCaptureRegion()
{
    saveConfig();
    captureRegion();
}

void ScreenshotOCRModule::onOcrScreenshot()
{
    QString ocrPath = m_config->getValue("ScreenshotOCR", "OCRPath", "").toString();
    
    if (ocrPath.isEmpty()) {
        QMessageBox::warning(this, "警告", "请先在设置中配置Umi-OCR路径！");
        m_logger->warn("ScreenshotOCR", "Umi-OCR路径未配置");
        return;
    }
    
    QProcess process;
    process.setProgram(ocrPath);
    process.setArguments(QStringList() << "--screenshot");
    
    m_logger->info("ScreenshotOCR", "启动Umi-OCR截图识别");
    
    process.startDetached();
}

void ScreenshotOCRModule::onClipboardOcr()
{
    QString ocrPath = m_config->getValue("ScreenshotOCR", "OCRPath", "").toString();
    
    if (ocrPath.isEmpty()) {
        QMessageBox::warning(this, "警告", "请先在设置中配置Umi-OCR路径！");
        m_logger->warn("ScreenshotOCR", "Umi-OCR路径未配置");
        return;
    }
    
    QProcess process;
    process.setProgram(ocrPath);
    process.setArguments(QStringList() << "--clipboard");
    
    m_logger->info("ScreenshotOCR", "启动Umi-OCR剪贴板识别");
    
    process.startDetached();
}

void ScreenshotOCRModule::onSaveScreenshot()
{
    if (m_screenshot.isNull()) {
        QMessageBox::warning(this, "Warning", "No screenshot to save!");
        return;
    }
    
    QString format = ui->comboSaveFormat->currentText();
    QString defaultPath = ui->editSavePath->text() + "/screenshot." + format.toLower();
    
    QString filePath = QFileDialog::getSaveFileName(this, "Save Screenshot", defaultPath, 
        QString("%1 Files (*.%2);;All Files (*)").arg(format, format.toLower()));
    
    if (!filePath.isEmpty()) {
        if (m_screenshot.save(filePath, format.toUtf8().constData())) {
            QMessageBox::information(this, "Success", "Screenshot saved successfully!");
            m_logger->info("ScreenshotOCR", QString("Screenshot saved successfully: %1").arg(filePath));
        } else {
            QMessageBox::warning(this, "Failed", "Failed to save screenshot!");
            m_logger->error("ScreenshotOCR", "Failed to save screenshot");
        }
    }
}

void ScreenshotOCRModule::onCopyScreenshot()
{
    if (m_screenshot.isNull()) {
        QMessageBox::warning(this, "Warning", "No screenshot to copy!");
        return;
    }
    
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setPixmap(m_screenshot);
    
    QMessageBox::information(this, "Success", "Screenshot copied to clipboard!");
    m_logger->info("ScreenshotOCR", "Screenshot copied to clipboard");
}

void ScreenshotOCRModule::onClearAnnotation()
{
    m_annotations.clear();
    if (!m_originalScreenshot.isNull()) {
        m_screenshot = m_originalScreenshot;
        ui->labelScreenshot->setPixmap(m_screenshot.scaled(ui->labelScreenshot->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    
    QMessageBox::information(this, "Success", "Annotations cleared!");
    m_logger->info("ScreenshotOCR", "Annotations cleared");
}

void ScreenshotOCRModule::onExportResult()
{
    QString result = ui->editOcrResult->toPlainText();
    if (result.isEmpty()) {
        QMessageBox::warning(this, "Warning", "No OCR result to export!");
        return;
    }
    
    QString filePath = QFileDialog::getSaveFileName(this, "Export OCR Result", "", 
        "Text Files (*.txt);;CSV Files (*.csv);;All Files (*)");
    
    if (!filePath.isEmpty()) {
        QFile file(filePath);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            out << result;
            file.close();
            
            QMessageBox::information(this, "Success", "OCR result exported successfully!");
            m_logger->info("ScreenshotOCR", QString("OCR result exported successfully: %1").arg(filePath));
        } else {
            QMessageBox::warning(this, "Failed", "Failed to export OCR result!");
            m_logger->error("ScreenshotOCR", "Failed to export OCR result");
        }
    }
}

void ScreenshotOCRModule::onHotkeyTriggered()
{
    m_logger->info("ScreenshotOCR", "全局热键已触发");
    onCaptureRegion();
}

void ScreenshotOCRModule::onHotkeyChanged(const QKeySequence &keySequence)
{
    m_logger->info("ScreenshotOCR", QString("快捷键已更改为: %1").arg(keySequence.toString()));
    
    GlobalHotkeyManager *hotkeyManager = GlobalHotkeyManager::instance();
    
    if (m_hotkeyRegistered) {
        QKeySequence oldHotkey = ui->keySequenceHotkey->keySequence();
        if (!oldHotkey.isEmpty()) {
            hotkeyManager->unregisterHotkey(oldHotkey);
            m_logger->info("ScreenshotOCR", QString("已注销旧快捷键: %1").arg(oldHotkey.toString()));
        }
    }
    
    saveConfig();
    setupHotkey();
}

void ScreenshotOCRModule::onOcrHotkeyTriggered()
{
    m_logger->info("ScreenshotOCR", "OCR识别快捷键已触发");
    onOcrScreenshot();
}

void ScreenshotOCRModule::onClipboardOcrHotkeyTriggered()
{
    m_logger->info("ScreenshotOCR", "剪贴板识别快捷键已触发");
    onClipboardOcr();
}

void ScreenshotOCRModule::captureScreen()
{
    ScreenHelper *screenHelper = ScreenHelper::instance();
    m_screenshot = screenHelper->captureScreen();
    m_originalScreenshot = m_screenshot;
    
    if (!m_screenshot.isNull()) {
        ui->labelScreenshot->setPixmap(m_screenshot.scaled(ui->labelScreenshot->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        m_logger->info("ScreenshotOCR", "Full screen screenshot captured successfully");
    } else {
        QMessageBox::warning(this, "Failed", "Screenshot capture failed!");
        m_logger->error("ScreenshotOCR", "Screenshot capture failed");
    }
}

void ScreenshotOCRModule::captureRegion()
{
    m_isCapturing = true;
    m_annotations.clear();
    
    startRegionCapture();
    
    m_logger->info("ScreenshotOCR", "Starting region capture");
}

void ScreenshotOCRModule::drawAnnotation(const QPoint &start, const QPoint &end, int type)
{
    if (m_screenshot.isNull()) {
        return;
    }
    
    QPixmap annotatedPixmap = m_screenshot.copy();
    QPainter painter(&annotatedPixmap);
    
    QPen pen;
    pen.setColor(Qt::red);
    pen.setWidth(2);
    painter.setPen(pen);
    
    if (type == 0) {
        painter.drawLine(start, end);
    } else if (type == 1) {
        painter.drawRect(QRect(start, end));
    }
    
    m_screenshot = annotatedPixmap;
    ui->labelScreenshot->setPixmap(m_screenshot.scaled(ui->labelScreenshot->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void ScreenshotOCRModule::updateAnnotationPreview()
{
    if (m_screenshot.isNull()) {
        return;
    }
    
    QPixmap annotatedPixmap = m_screenshot.copy();
    QPainter painter(&annotatedPixmap);
    
    QPen pen;
    pen.setColor(Qt::red);
    pen.setWidth(2);
    painter.setPen(pen);
    
    for (const auto &annotation : m_annotations) {
        const QPoint &start = annotation.first;
        const QPoint &end = annotation.second;
        
        if (m_annotationType == 0) {
            painter.drawLine(start, end);
        } else if (m_annotationType == 1) {
            painter.drawRect(QRect(start, end));
        }
    }
    
    m_screenshot = annotatedPixmap;
    ui->labelScreenshot->setPixmap(m_screenshot.scaled(ui->labelScreenshot->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void ScreenshotOCRModule::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        if (!m_screenshot.isNull()) {
            m_drawStart = event->pos();
            m_isDrawing = true;
        }
    }
}

void ScreenshotOCRModule::mouseMoveEvent(QMouseEvent *event)
{
    if (!m_screenshot.isNull() && m_isDrawing) {
        m_drawEnd = event->pos();
        
        QPixmap tempPixmap = m_screenshot.isNull() ? QPixmap(ui->labelScreenshot->size()) : m_screenshot;
        QPainter painter(&tempPixmap);
        
        if (!m_screenshot.isNull()) {
            painter.fillRect(tempPixmap.rect(), Qt::transparent);
        } else {
            painter.drawPixmap(0, 0, m_screenshot);
        }
        
        QPen pen;
        pen.setColor(Qt::red);
        pen.setWidth(2);
        painter.setPen(pen);
        
        if (m_annotationType == 0) {
            painter.drawLine(m_drawStart, m_drawEnd);
        } else if (m_annotationType == 1) {
            painter.drawRect(QRect(m_drawStart, m_drawEnd));
        }
        
        ui->labelScreenshot->setPixmap(tempPixmap);
    }
}

void ScreenshotOCRModule::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        if (!m_screenshot.isNull() && m_isDrawing) {
            m_isDrawing = false;
            
            QRect selectionRect(m_drawStart, m_drawEnd);
            selectionRect = selectionRect.normalized();
            
            if (selectionRect.width() > 10 && selectionRect.height() > 10) {
                m_annotations.append(qMakePair(m_drawStart, m_drawEnd));
                drawAnnotation(m_drawStart, m_drawEnd, m_annotationType);
                updateAnnotationPreview();
                m_logger->info("ScreenshotOCR", "Annotation added");
            }
        }
    }
}

bool ScreenshotOCRModule::eventFilter(QObject *watched, QEvent *event)
{
    Q_UNUSED(watched);
    Q_UNUSED(event);
    
    return QWidget::eventFilter(watched, event);
}

void ScreenshotOCRModule::startRegionCapture()
{
    if (m_captureDialog) {
        delete m_captureDialog;
        m_captureDialog = nullptr;
    }
    
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    
    m_captureBackground = screen->grabWindow(0);
    
    m_captureDialog = new CaptureDialog(nullptr);
    m_captureDialog->setBackground(m_captureBackground);
    
    connect(m_captureDialog, &CaptureDialog::captureCompleted, this, &ScreenshotOCRModule::onCaptureCompleted);
    connect(m_captureDialog, &CaptureDialog::captureCancelled, this, &ScreenshotOCRModule::onCaptureCancelled);
    connect(m_captureDialog, &CaptureDialog::destroyed, this, [this]() {
        m_captureDialog = nullptr;
    });
    
    m_captureDialog->show();
    m_captureDialog->activateWindow();
    
    m_logger->info("ScreenshotOCR", "区域截图选择对话框已显示");
}

void ScreenshotOCRModule::finishRegionCapture()
{
    if (!m_captureDialog) {
        return;
    }
    
    m_captureDialog->close();
    delete m_captureDialog;
    m_captureDialog = nullptr;
    
    m_logger->info("ScreenshotOCR", "Region capture selection dialog closed");
}

void ScreenshotOCRModule::onCaptureCompleted(const QRect &rect)
{
    m_screenshot = m_captureBackground.copy(rect);
    m_originalScreenshot = m_screenshot;
    
    ui->labelScreenshot->setPixmap(m_screenshot.scaled(ui->labelScreenshot->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    m_logger->info("ScreenshotOCR", "区域截图成功");
    
    ScreenshotEditorWindow *editorWindow = new ScreenshotEditorWindow(m_screenshot, this);
    editorWindow->setAttribute(Qt::WA_DeleteOnClose);
    editorWindow->show();
}

void ScreenshotOCRModule::onCaptureCancelled()
{
    m_logger->info("ScreenshotOCR", "区域截图已取消");
}
