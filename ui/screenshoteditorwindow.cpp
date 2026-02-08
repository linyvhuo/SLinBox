#include "screenshoteditorwindow.h"
#include "ui_screenshoteditorwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QClipboard>
#include <QApplication>
#include <QPainter>
#include <QCloseEvent>
#include <QProcess>
#include <QStandardPaths>
#include <QTextStream>
#include <QFile>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QGroupBox>
#include <QComboBox>
#include <QSpinBox>
#include <QLineEdit>
#include <QFontDialog>
#include <QScreen>
#include <QGuiApplication>
#include <QtMath>
#include <QTimer>
#include <QCursor>
#include <QPixmap>

ScreenshotEditorWindow::ScreenshotEditorWindow(const QPixmap &screenshot, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::ScreenshotEditorWindow)
    , m_originalScreenshot(screenshot)
    , m_editedScreenshot(screenshot)
    , m_isDrawing(false)
    , m_drawStart(QPoint(0, 0))
    , m_drawEnd(QPoint(0, 0))
    , m_annotationType(0)
    , m_isPinned(false)
    , m_isResizing(false)
    , m_resizeEdge(0)
    , m_zoomScale(1.0)
{
    setAttribute(Qt::WA_QuitOnClose, false);
    ui->setupUi(this);
    initUI();
    setupConnections();
}

ScreenshotEditorWindow::~ScreenshotEditorWindow()
{
    delete ui;
}

QPixmap ScreenshotEditorWindow::getEditedScreenshot() const
{
    return m_editedScreenshot;
}

void ScreenshotEditorWindow::initUI()
{
    setWindowTitle("截图编辑器");
    resize(1024, 768);
    
    ui->screenshotLabel->setPixmap(m_editedScreenshot);
    ui->screenshotLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    ui->screenshotLabel->setStyleSheet("border: 2px solid #cccccc; background-color: #f5f5f5;");
    ui->screenshotLabel->setScaledContents(false);
    
    ui->editOcrResult->setReadOnly(true);
    
    ui->comboAnnotationType->addItem("直线", 0);
    ui->comboAnnotationType->addItem("矩形", 1);
    ui->comboAnnotationType->addItem("箭头", 2);
    ui->comboAnnotationType->addItem("文字", 3);
    ui->comboAnnotationType->setCurrentIndex(0);
    
    ui->spinLineWidth->setRange(1, 10);
    ui->spinLineWidth->setValue(2);
    
    ui->comboColor->addItem("红色", QColor(Qt::red));
    ui->comboColor->addItem("蓝色", QColor(Qt::blue));
    ui->comboColor->addItem("绿色", QColor(Qt::green));
    ui->comboColor->addItem("黄色", QColor(Qt::yellow));
    ui->comboColor->addItem("黑色", QColor(Qt::black));
    ui->comboColor->addItem("白色", QColor(Qt::white));
    ui->comboColor->setCurrentIndex(0);
    
    ui->comboArrowType->addItem("普通箭头", 0);
    ui->comboArrowType->addItem("双向箭头", 1);
    ui->comboArrowType->setCurrentIndex(0);
    
    ui->editTextAnnotation->setPlaceholderText("输入文字标注后点击截图添加");
}

void ScreenshotEditorWindow::setupConnections()
{
    connect(ui->btnOcr, &QPushButton::clicked, this, &ScreenshotEditorWindow::onOcrClicked);
    connect(ui->btnSave, &QPushButton::clicked, this, &ScreenshotEditorWindow::onSaveClicked);
    connect(ui->btnCopy, &QPushButton::clicked, this, &ScreenshotEditorWindow::onCopyClicked);
    connect(ui->btnClearAnnotation, &QPushButton::clicked, this, &ScreenshotEditorWindow::onClearAnnotationClicked);
    connect(ui->btnExportResult, &QPushButton::clicked, this, &ScreenshotEditorWindow::onExportResultClicked);
    connect(ui->btnPinToDesktop, &QPushButton::clicked, this, &ScreenshotEditorWindow::onPinToDesktopClicked);
    connect(ui->comboAnnotationType, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int index) { m_annotationType = index; });
}

QPoint ScreenshotEditorWindow::mapToScreenshot(const QPoint &windowPos) const
{
    if (!ui->screenshotLabel->pixmap() || ui->screenshotLabel->pixmap()->isNull()) {
        return QPoint(0, 0);
    }
    
    const QPixmap *pixmap = ui->screenshotLabel->pixmap();
    QSize pixmapSize = pixmap->size();
    QSize originalSize = m_originalScreenshot.size();
    QSize labelSize = ui->screenshotLabel->size();
    
    QRect labelRect = ui->screenshotLabel->rect();
    QPoint labelGlobalPos = ui->screenshotLabel->mapTo(this, QPoint(0, 0));
    
    if (!labelRect.contains(windowPos - labelGlobalPos)) {
        return QPoint(-1, -1);
    }
    
    QPoint relativePos = windowPos - labelGlobalPos;
    
    double scaleX = static_cast<double>(originalSize.width()) / pixmapSize.width();
    double scaleY = static_cast<double>(originalSize.height()) / pixmapSize.height();
    
    int x = static_cast<int>(relativePos.x() * scaleX);
    int y = static_cast<int>(relativePos.y() * scaleY);
    
    return QPoint(x, y);
}

void ScreenshotEditorWindow::mousePressEvent(QMouseEvent *event)
{
    if (m_isPinned) {
        if (event->button() == Qt::LeftButton) {
            if (m_closeButtonRect.contains(event->pos())) {
                onCloseButtonClicked();
                return;
            }
            
            if (m_zoomInButtonRect.contains(event->pos())) {
                onZoomInClicked();
                return;
            }
            
            if (m_zoomOutButtonRect.contains(event->pos())) {
                onZoomOutClicked();
                return;
            }
            
            int edge = getResizeEdge(event->pos());
            if (edge != 0) {
                m_isResizing = true;
                m_resizeEdge = edge;
                m_resizeStartPos = event->globalPos();
                m_resizeStartSize = size();
                return;
            }
            
            m_dragPosition = event->globalPos() - frameGeometry().topLeft();
        }
        
        return QMainWindow::mousePressEvent(event);
    }
}

void ScreenshotEditorWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (m_isPinned) {
        if (m_isResizing) {
            QPoint delta = event->globalPos() - m_resizeStartPos;
            QSize newSize = m_resizeStartSize;
            QPoint newPos = pos();
            
            if (m_resizeEdge & 1) {
                newSize.setWidth(newSize.width() - delta.x());
                newSize.setHeight(newSize.height() - delta.y());
                newPos.setX(pos().x() + delta.x());
                newPos.setY(pos().y() + delta.y());
            } else if (m_resizeEdge & 2) {
                newSize.setWidth(newSize.width() + delta.x());
                newSize.setHeight(newSize.height() - delta.y());
                newPos.setY(pos().y() + delta.y());
            } else if (m_resizeEdge & 3) {
                newSize.setWidth(newSize.width() - delta.x());
                newSize.setHeight(newSize.height() + delta.y());
                newPos.setX(pos().x() + delta.x());
            } else if (m_resizeEdge & 4) {
                newSize.setWidth(newSize.width() + delta.x());
                newSize.setHeight(newSize.height() + delta.y());
            } else if (m_resizeEdge & 5) {
                newSize.setHeight(newSize.height() - delta.y());
                newPos.setY(pos().y() + delta.y());
            } else if (m_resizeEdge & 6) {
                newSize.setHeight(newSize.height() + delta.y());
            } else if (m_resizeEdge & 7) {
                newSize.setWidth(newSize.width() - delta.x());
                newPos.setX(pos().x() + delta.x());
            } else if (m_resizeEdge & 8) {
                newSize.setWidth(newSize.width() + delta.x());
            }
            
            if (newSize.width() >= 100 && newSize.height() >= 100) {
                resize(newSize);
                move(newPos);
            }
        } else if (event->buttons() & Qt::LeftButton) {
            move(event->globalPos() - m_dragPosition);
        }
        
        return;
    }
    
    if (m_isDrawing) {
        QPoint screenshotPos = mapToScreenshot(event->pos());
        
        if (screenshotPos.x() >= 0 && screenshotPos.y() >= 0) {
            m_drawEnd = screenshotPos;
            drawPreview();
        }
    }
}

void ScreenshotEditorWindow::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_isPinned) {
        m_isResizing = false;
        m_resizeEdge = 0;
        return;
    }
    
    if (event->button() == Qt::LeftButton && m_isDrawing) {
        m_isDrawing = false;
        
        QPoint screenshotPos = mapToScreenshot(event->pos());
        
        if (screenshotPos.x() >= 0 && screenshotPos.y() >= 0) {
            m_drawEnd = screenshotPos;
            
            QPoint start = m_drawStart;
            QPoint end = m_drawEnd;
            
            if (m_annotationType == 0) {
                m_lineAnnotations.append(qMakePair(start, end));
            } else if (m_annotationType == 1) {
                QRect rect(start, end);
                rect = rect.normalized();
                m_rectAnnotations.append(rect);
            } else if (m_annotationType == 2) {
                m_arrowAnnotations.append(qMakePair(start, end));
            } else if (m_annotationType == 3) {
                QString text = ui->editTextAnnotation->text();
                if (!text.isEmpty()) {
                    m_textAnnotations.append(qMakePair(start, text));
                }
            }
            
            drawAnnotations();
        }
    }
}

void ScreenshotEditorWindow::drawPreview()
{
    QPixmap tempPixmap = m_editedScreenshot;
    QPainter painter(&tempPixmap);
    
    int lineWidth = ui->spinLineWidth->value();
    QColor color = ui->comboColor->currentData().value<QColor>();
    
    QPen pen;
    pen.setColor(color);
    pen.setWidth(lineWidth);
    painter.setPen(pen);
    
    if (m_annotationType == 0) {
        painter.drawLine(m_drawStart, m_drawEnd);
    } else if (m_annotationType == 1) {
        QRect rect(m_drawStart, m_drawEnd);
        rect = rect.normalized();
        painter.drawRect(rect);
    } else if (m_annotationType == 2) {
        drawArrow(painter, m_drawStart, m_drawEnd, lineWidth, color);
    } else if (m_annotationType == 3) {
        QString text = ui->editTextAnnotation->text();
        if (!text.isEmpty()) {
            QFont font = painter.font();
            font.setPixelSize(16);
            painter.setFont(font);
            painter.setPen(color);
            painter.drawText(m_drawEnd, text);
        }
    }
    
    ui->screenshotLabel->setPixmap(tempPixmap);
}

void ScreenshotEditorWindow::drawArrow(QPainter &painter, const QPoint &start, const QPoint &end, int lineWidth, const QColor &color)
{
    int arrowType = ui->comboArrowType->currentIndex();
    QPen pen;
    pen.setColor(color);
    pen.setWidth(lineWidth);
    painter.setPen(pen);
    
    double angle = std::atan2(end.y() - start.y(), end.x() - start.x());
    double length = std::sqrt(std::pow(end.x() - start.x(), 2) + std::pow(end.y() - start.y(), 2));
    
    if (length < 10) {
        painter.drawLine(start, end);
        return;
    }
    
    double arrowLength = qMin(20.0, length * 0.3);
    double arrowWidth = arrowLength * 0.5;
    
    double cosA = std::cos(angle);
    double sinA = std::sin(angle);
    
    QPolygonF arrowPolygon;
    
    if (arrowType == 0) {
        QPoint arrowTip = end;
        QPoint arrowBase1(end.x() - arrowLength * cosA + arrowWidth * sinA,
                           end.y() - arrowLength * sinA - arrowWidth * cosA);
        QPoint arrowBase2(end.x() - arrowLength * cosA - arrowWidth * sinA,
                           end.y() - arrowLength * sinA + arrowWidth * cosA);
        
        arrowPolygon << arrowTip << arrowBase1 << arrowBase2;
    } else {
        QPoint arrowTip = end;
        QPoint arrowBase1(end.x() - arrowLength * cosA + arrowWidth * sinA,
                           end.y() - arrowLength * sinA - arrowWidth * cosA);
        QPoint arrowBase2(end.x() - arrowLength * cosA - arrowWidth * sinA,
                           end.y() - arrowLength * sinA + arrowWidth * cosA);
        QPoint arrowBackTip(start.x() + arrowLength * cosA,
                             start.y() + arrowLength * sinA);
        
        arrowPolygon << arrowTip << arrowBase1 << arrowBackTip << arrowBase2;
    }
    
    painter.setBrush(QBrush(color));
    painter.drawPolygon(arrowPolygon);
    painter.drawLine(start, end);
}

void ScreenshotEditorWindow::onOcrClicked()
{
    QString tempPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/screenshot_editor_temp.png";
    m_editedScreenshot.save(tempPath, "PNG");
    
    QProcess process;
    QString program = "umi-ocr";
    QStringList arguments;
    arguments << tempPath;
    
    process.start(program, arguments);
    
    if (!process.waitForStarted(5000)) {
        QMessageBox::warning(this, "错误", "无法启动Umi-OCR程序！请确保已安装Umi-OCR并添加到系统PATH环境变量中。");
        return;
    }
    
    if (!process.waitForFinished(30000)) {
        QMessageBox::warning(this, "错误", "OCR识别超时！");
        process.kill();
        return;
    }
    
    QString result = QString::fromUtf8(process.readAllStandardOutput());
    QString error = QString::fromUtf8(process.readAllStandardError());
    
    if (process.exitCode() != 0) {
        QMessageBox::warning(this, "错误", QString("OCR识别失败: %1").arg(error));
        return;
    }
    
    if (result.isEmpty()) {
        QMessageBox::warning(this, "警告", "未识别到文本！");
    } else {
        ui->editOcrResult->setPlainText(result);
    }
}

void ScreenshotEditorWindow::onSaveClicked()
{
    QString format = "PNG";
    QString defaultPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) + "/screenshot." + format.toLower();
    
    QString filePath = QFileDialog::getSaveFileName(this, "保存截图", defaultPath, 
        "PNG Files (*.png);;JPEG Files (*.jpg);;BMP Files (*.bmp);;All Files (*)");
    
    if (!filePath.isEmpty()) {
        if (m_editedScreenshot.save(filePath)) {
            QMessageBox::information(this, "成功", "截图保存成功！");
        } else {
            QMessageBox::warning(this, "失败", "截图保存失败！");
        }
    }
}

void ScreenshotEditorWindow::onCopyClicked()
{
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setPixmap(m_editedScreenshot);
    
    QMessageBox::information(this, "成功", "截图已复制到剪贴板！");
}

void ScreenshotEditorWindow::onClearAnnotationClicked()
{
    m_lineAnnotations.clear();
    m_rectAnnotations.clear();
    m_arrowAnnotations.clear();
    m_textAnnotations.clear();
    m_editedScreenshot = m_originalScreenshot;
    ui->screenshotLabel->setPixmap(m_editedScreenshot);
}

void ScreenshotEditorWindow::onExportResultClicked()
{
    QString result = ui->editOcrResult->toPlainText();
    if (result.isEmpty()) {
        QMessageBox::warning(this, "警告", "没有OCR结果可导出！");
        return;
    }
    
    QString filePath = QFileDialog::getSaveFileName(this, "导出OCR结果", "", 
        "文本文件 (*.txt);;CSV文件 (*.csv);;所有文件 (*)");
    
    if (!filePath.isEmpty()) {
        QFile file(filePath);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            out << result;
            file.close();
            
            QMessageBox::information(this, "成功", "OCR结果导出成功！");
        } else {
            QMessageBox::warning(this, "失败", "OCR结果导出失败！");
        }
    }
}

void ScreenshotEditorWindow::drawAnnotations()
{
    m_editedScreenshot = m_originalScreenshot;
    QPainter painter(&m_editedScreenshot);
    
    int lineWidth = ui->spinLineWidth->value();
    QColor color = ui->comboColor->currentData().value<QColor>();
    
    QPen pen;
    pen.setColor(color);
    pen.setWidth(lineWidth);
    painter.setPen(pen);
    
    for (const auto &line : m_lineAnnotations) {
        painter.drawLine(line.first, line.second);
    }
    
    for (const auto &rect : m_rectAnnotations) {
        painter.drawRect(rect);
    }
    
    for (const auto &arrow : m_arrowAnnotations) {
        drawArrow(painter, arrow.first, arrow.second, lineWidth, color);
    }
    
    QFont font = painter.font();
    font.setPixelSize(16);
    painter.setFont(font);
    painter.setPen(color);
    
    for (const auto &text : m_textAnnotations) {
        painter.drawText(text.first, text.second);
    }
    
    ui->screenshotLabel->setPixmap(m_editedScreenshot);
}

void ScreenshotEditorWindow::closeEvent(QCloseEvent *event)
{
    QMainWindow::closeEvent(event);
}

void ScreenshotEditorWindow::onPinToDesktopClicked()
{
    togglePinMode();
}

void ScreenshotEditorWindow::togglePinMode()
{
    setPinMode(!m_isPinned);
}

void ScreenshotEditorWindow::setPinMode(bool pinned)
{
    m_isPinned = pinned;
    
    if (pinned) {
        m_zoomScale = 1.0;
        setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
        QWidget *central = centralWidget();
        if (central) {
            central->hide();
        }
        resize(m_originalScreenshot.size());
        
        QScreen *screen = QGuiApplication::primaryScreen();
        if (screen) {
            QRect screenGeometry = screen->availableGeometry();
            move(screenGeometry.center() - rect().center());
        }
    } else {
        setWindowFlags(Qt::Window);
        QWidget *central = centralWidget();
        if (central) {
            central->show();
        }
        resize(1024, 768);
    }
    
    show();
    activateWindow();
}

void ScreenshotEditorWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    
    if (m_isPinned) {
        int buttonSize = 30;
        m_closeButtonRect = QRect(width() - buttonSize - 10, 10, buttonSize, buttonSize);
        
        int zoomButtonSize = 25;
        int zoomButtonSpacing = 10;
        int zoomButtonsY = height() - zoomButtonSize - 10;
        m_zoomOutButtonRect = QRect(10, zoomButtonsY, zoomButtonSize, zoomButtonSize);
        m_zoomInButtonRect = QRect(10 + zoomButtonSize + zoomButtonSpacing, zoomButtonsY, zoomButtonSize, zoomButtonSize);
    }
}

void ScreenshotEditorWindow::paintEvent(QPaintEvent *event)
{
    QMainWindow::paintEvent(event);
    
    if (m_isPinned) {
        QPainter painter(this);
        
        QSize scaledSize = m_originalScreenshot.size() * m_zoomScale;
        QPixmap scaledPixmap = m_editedScreenshot.scaled(scaledSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        painter.drawPixmap(0, 0, scaledPixmap);
        
        painter.setPen(QPen(QColor(255, 0, 0, 150), 2));
        painter.setBrush(QBrush(QColor(255, 255, 255, 200)));
        painter.drawRoundedRect(m_closeButtonRect, 5, 5);
        
        painter.setPen(Qt::red);
        painter.setFont(QFont("Arial", 16, QFont::Bold));
        painter.drawText(m_closeButtonRect, Qt::AlignCenter, "×");
        
        painter.setPen(QPen(QColor(0, 120, 255, 150), 2));
        painter.setBrush(QBrush(QColor(255, 255, 255, 200)));
        painter.drawRoundedRect(m_zoomOutButtonRect, 3, 3);
        
        painter.setPen(QColor(0, 120, 255));
        painter.setFont(QFont("Arial", 14, QFont::Bold));
        painter.drawText(m_zoomOutButtonRect, Qt::AlignCenter, "-");
        
        painter.setPen(QPen(QColor(0, 120, 255, 150), 2));
        painter.setBrush(QBrush(QColor(255, 255, 255, 200)));
        painter.drawRoundedRect(m_zoomInButtonRect, 3, 3);
        
        painter.setPen(QColor(0, 120, 255));
        painter.setFont(QFont("Arial", 14, QFont::Bold));
        painter.drawText(m_zoomInButtonRect, Qt::AlignCenter, "+");
    }
}

void ScreenshotEditorWindow::contextMenuEvent(QContextMenuEvent *event)
{
    if (!m_isPinned) {
        QMainWindow::contextMenuEvent(event);
        return;
    }
    
    QMenu menu(this);
    QAction *unpinAction = menu.addAction("取消钉住");
    QAction *saveAction = menu.addAction("保存截图");
    QAction *copyAction = menu.addAction("复制截图");
    QAction *closeAction = menu.addAction("关闭");
    
    QAction *selected = menu.exec(event->globalPos());
    
    if (selected == unpinAction) {
        setPinMode(false);
    } else if (selected == saveAction) {
        onSaveClicked();
    } else if (selected == copyAction) {
        onCopyClicked();
    } else if (selected == closeAction) {
        close();
    }
}

void ScreenshotEditorWindow::onCloseButtonClicked()
{
    if (m_isPinned) {
        close();
    }
}

void ScreenshotEditorWindow::onZoomInClicked()
{
    if (m_isPinned) {
        m_zoomScale = qBound(0.5, m_zoomScale + 0.25, 3.0);
        update();
    }
}

void ScreenshotEditorWindow::onZoomOutClicked()
{
    if (m_isPinned) {
        m_zoomScale = qBound(0.5, m_zoomScale - 0.25, 3.0);
        update();
    }
}

int ScreenshotEditorWindow::getResizeEdge(const QPoint &pos) const
{
    const int edgeSize = 8;
    QRect rect = QMainWindow::rect();
    
    if (pos.x() <= edgeSize && pos.y() <= edgeSize) return 1;
    if (pos.x() >= rect.width() - edgeSize && pos.y() <= edgeSize) return 2;
    if (pos.x() <= edgeSize && pos.y() >= rect.height() - edgeSize) return 3;
    if (pos.x() >= rect.width() - edgeSize && pos.y() >= rect.height() - edgeSize) return 4;
    if (pos.y() <= edgeSize) return 5;
    if (pos.y() >= rect.height() - edgeSize) return 6;
    if (pos.x() <= edgeSize) return 7;
    if (pos.x() >= rect.width() - edgeSize) return 8;
    
    return 0;
}
