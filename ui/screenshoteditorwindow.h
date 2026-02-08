#ifndef SCREENSHOTEDITORWINDOW_H
#define SCREENSHOTEDITORWINDOW_H

#include <QMainWindow>
#include <QPixmap>
#include <QRect>
#include <QPoint>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QContextMenuEvent>
#include <QMenu>
#include <QAction>

QT_BEGIN_NAMESPACE
namespace Ui { class ScreenshotEditorWindow; }
QT_END_NAMESPACE

class ScreenshotEditorWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ScreenshotEditorWindow(const QPixmap &screenshot, QWidget *parent = nullptr);
    ~ScreenshotEditorWindow();
    
    QPixmap getEditedScreenshot() const;
    
protected:
    void closeEvent(QCloseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;

private slots:
    void onOcrClicked();
    void onSaveClicked();
    void onCopyClicked();
    void onClearAnnotationClicked();
    void onExportResultClicked();
    void onPinToDesktopClicked();
    void onCloseButtonClicked();
    void onZoomInClicked();
    void onZoomOutClicked();

private:
    void initUI();
    void setupConnections();
    void drawAnnotations();
    void drawPreview();
    void drawArrow(QPainter &painter, const QPoint &start, const QPoint &end, int lineWidth, const QColor &color);
    QPoint mapToScreenshot(const QPoint &windowPos) const;
    void togglePinMode();
    void setPinMode(bool pinned);
    int getResizeEdge(const QPoint &pos) const;
    
    Ui::ScreenshotEditorWindow *ui;
    QPixmap m_originalScreenshot;
    QPixmap m_editedScreenshot;
    QList<QPair<QPoint, QPoint>> m_lineAnnotations;
    QList<QRect> m_rectAnnotations;
    QList<QPair<QPoint, QPoint>> m_arrowAnnotations;
    QList<QPair<QPoint, QString>> m_textAnnotations;
    bool m_isDrawing;
    QPoint m_drawStart;
    QPoint m_drawEnd;
    int m_annotationType;
    
    bool m_isPinned;
    bool m_isResizing;
    int m_resizeEdge;
    QPoint m_dragPosition;
    QPoint m_resizeStartPos;
    QSize m_resizeStartSize;
    QRect m_closeButtonRect;
    QRect m_zoomInButtonRect;
    QRect m_zoomOutButtonRect;
    double m_zoomScale;
};

#endif // SCREENSHOTEDITORWINDOW_H
