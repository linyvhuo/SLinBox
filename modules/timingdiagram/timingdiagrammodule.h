#ifndef TIMINGDIAGRAMMODULE_H
#define TIMINGDIAGRAMMODULE_H

#include <QWidget>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsItem>
#include <QGraphicsLineItem>
#include <QGraphicsTextItem>
#include "core/configmanager.h"
#include "core/logger.h"

QT_BEGIN_NAMESPACE
namespace Ui { class TimingDiagramModule; }
QT_END_NAMESPACE

class TimingDiagramModule : public QWidget
{
    Q_OBJECT

public:
    explicit TimingDiagramModule(QWidget *parent = nullptr);
    ~TimingDiagramModule();

    void loadConfig();
    void saveConfig();

private slots:
    void onAddChannel();
    void onClearChannels();
    void onExportImage();
    void onExportText();
    void onProtocolChanged(const QString &protocol);

private:
    void initUI();
    void drawTimingDiagram();
    void drawSPI();
    void drawI2C();
    void drawUSART();
    void drawUART();
    void drawI2S();
    void clearDiagram();
    void parseWaveformData();
    void drawClockSignal(int x, int y, const QString &data, int bitWidth, const QPen &pen);
    void drawDataSignal(int x, int y, const QString &data, int bitWidth, const QPen &pen);
    void drawBusSignal(int x, int y, const QString &data, int bitWidth, const QPen &pen);
    QList<QString> m_waveformData;

    Ui::TimingDiagramModule *ui;

    ConfigManager *m_config;
    Logger *m_logger;

    QGraphicsScene *m_scene;
    QGraphicsView *m_graphicsView;
    QString m_currentProtocol;
};

#endif // TIMINGDIAGRAMMODULE_H