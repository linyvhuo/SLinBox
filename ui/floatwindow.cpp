#include "floatwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QScreen>
#include <QGuiApplication>
#include <QPainter>
#include <QPainterPath>
#include <QConicalGradient>
#include <QFontMetrics>
#include "core/configmanager.h"

FloatWindow::FloatWindow(QWidget *parent)
    : QWidget(parent)
    , m_isRunning(false)
    , m_progress(0)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);
    
    initUI();
    updateStyle();
    
    QScreen *screen = QGuiApplication::primaryScreen();
    if (screen) {
        QRect screenGeometry = screen->availableGeometry();
        move(screenGeometry.left() + 20, screenGeometry.bottom() - height() - 20);
    }
}

FloatWindow::~FloatWindow()
{
}

void FloatWindow::initUI()
{
    setFixedSize(75, 75);
    
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    
    m_stopBtn = new QPushButton("", this);
    m_stopBtn->setFixedSize(75, 75);
    m_stopBtn->setEnabled(false);
    m_stopBtn->setStyleSheet("QPushButton { background: transparent; border: none; }");
    connect(m_stopBtn, &QPushButton::clicked, this, &FloatWindow::onStopClicked);
    
    mainLayout->addWidget(m_stopBtn);
}

void FloatWindow::updateStyle()
{
    Q_UNUSED(this);
}

void FloatWindow::setProgress(int value)
{
    m_progress = value;
    update();
}

void FloatWindow::setStatus(const QString &status)
{
    Q_UNUSED(status);
}

void FloatWindow::setRunning(bool running)
{
    m_isRunning = running;
    m_stopBtn->setEnabled(running);
}

void FloatWindow::closeWindow()
{
    hide();
}

void FloatWindow::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    int centerX = width() / 2;
    int centerY = height() / 2;
    int radius = 35;
    
    ConfigManager *config = ConfigManager::instance();
    QString theme = config->getValue("General", "Theme", "light").toString();
    
    QColor bgColor = (theme == "dark") ? QColor(44, 62, 80, 230) : QColor(255,255,255, 230);
    QColor borderColor = (theme == "dark") ? QColor(255,255,255, 100) : QColor(0, 0, 0, 100);
    QColor progressColor = (theme == "dark") ? QColor(46, 204, 113, 230) : QColor(52, 152, 219, 230);
    QColor textColor = (theme == "dark") ? QColor(255,255,255) : QColor(0, 0, 0);
    
    QPainterPath path;
    path.addEllipse(centerX - radius, centerY - radius, radius * 2, radius * 2);
    
    painter.setPen(QPen(borderColor, 2));
    painter.setBrush(bgColor);
    painter.drawPath(path);
    
    if (m_progress > 0) {
        int startAngle = 90 * 16;
        int spanAngle = -(m_progress * 360 / 100) * 16;
        
        QPen progressPen(progressColor, 5);
        progressPen.setCapStyle(Qt::RoundCap);
        painter.setPen(progressPen);
        
        painter.drawArc(centerX - radius + 5, centerY - radius + 5, (radius - 5) * 2, (radius - 5) * 2, startAngle, spanAngle);
    }
    
    painter.setPen(textColor);
    painter.setFont(QFont("Arial", 10, QFont::Bold));
    painter.drawText(QRect(0, centerY - 8, width(), 16), Qt::AlignCenter, QString("%1%").arg(m_progress));
}

void FloatWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_dragPosition = event->globalPos() - frameGeometry().topLeft();
        event->accept();
    }
}

void FloatWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        QPoint newPos = event->globalPos() - m_dragPosition;
        
        QScreen *screen = QGuiApplication::screenAt(newPos);
        if (!screen) {
            screen = QGuiApplication::primaryScreen();
        }
        
        if (screen) {
            QRect screenGeometry = screen->availableGeometry();
            QRect windowGeometry = frameGeometry();
            
            int newX = newPos.x();
            int newY = newPos.y();
            
            if (newX < screenGeometry.left()) {
                newX = screenGeometry.left();
            } else if (newX + windowGeometry.width() > screenGeometry.right()) {
                newX = screenGeometry.right() - windowGeometry.width();
            }
            
            if (newY < screenGeometry.top()) {
                newY = screenGeometry.top();
            } else if (newY + windowGeometry.height() > screenGeometry.bottom()) {
                newY = screenGeometry.bottom() - windowGeometry.height();
            }
            
            move(newX, newY);
        } else {
            move(newPos);
        }
        
        event->accept();
    }
}

void FloatWindow::onStopClicked()
{
    emit stopClicked();
}
