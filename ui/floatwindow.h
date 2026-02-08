#ifndef FLOATWINDOW_H
#define FLOATWINDOW_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QProgressBar>
#include <QMouseEvent>
#include <QTimer>
#include <QPainter>
#include <QPainterPath>

class FloatWindow : public QWidget
{
    Q_OBJECT

public:
    explicit FloatWindow(QWidget *parent = nullptr);
    ~FloatWindow();

    void setProgress(int value);
    void setStatus(const QString &status);
    void setRunning(bool running);
    void closeWindow();

signals:
    void stopClicked();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private slots:
    void onStopClicked();

private:
    void initUI();
    void updateStyle();

    QPushButton *m_stopBtn;

    QPoint m_dragPosition;
    bool m_isRunning;
    int m_progress;
};

#endif // FLOATWINDOW_H
