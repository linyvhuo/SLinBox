#include "timingdiagrammodule.h"
#include "ui_timingdiagrammodule.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include <QPen>
#include <QBrush>
#include <QFont>


TimingDiagramModule::TimingDiagramModule(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TimingDiagramModule)
    , m_config(ConfigManager::instance())
    , m_logger(Logger::instance())
    , m_scene(nullptr)
    , m_graphicsView(nullptr)
    , m_currentProtocol("SPI")
{
    ui->setupUi(this);
    initUI();
    loadConfig();
}

TimingDiagramModule::~TimingDiagramModule()
{
    delete ui;
}

void TimingDiagramModule::initUI()
{
    m_scene = new QGraphicsScene(this);
    m_graphicsView = ui->graphicsView;
    m_graphicsView->setScene(m_scene);
    m_graphicsView->setRenderHint(QPainter::Antialiasing);
    
    connect(ui->btnAddChannel, &QPushButton::clicked, this, &TimingDiagramModule::onAddChannel);
    connect(ui->btnClearChannels, &QPushButton::clicked, this, &TimingDiagramModule::onClearChannels);
    connect(ui->btnExportImage, &QPushButton::clicked, this, &TimingDiagramModule::onExportImage);
    connect(ui->btnExportText, &QPushButton::clicked, this, &TimingDiagramModule::onExportText);
    connect(ui->comboProtocol, QOverload<const QString&>::of(&QComboBox::currentTextChanged),
            this, &TimingDiagramModule::onProtocolChanged);
    connect(ui->editWaveform, &QPlainTextEdit::textChanged, this, &TimingDiagramModule::parseWaveformData);
    
    drawTimingDiagram();
}

void TimingDiagramModule::loadConfig()
{
    ui->spinChannelCount->setValue(m_config->getValue("TimingDiagram", "ChannelCount", 4).toInt());
    ui->spinTimeScale->setValue(m_config->getValue("TimingDiagram", "TimeScale", 1.0).toDouble());
    QString protocol = m_config->getValue("TimingDiagram", "Protocol", "SPI").toString();
    int index = ui->comboProtocol->findText(protocol);
    if (index >= 0) {
        ui->comboProtocol->setCurrentIndex(index);
    }
}

void TimingDiagramModule::saveConfig()
{
    m_config->setValue("TimingDiagram", "ChannelCount", ui->spinChannelCount->value());
    m_config->setValue("TimingDiagram", "TimeScale", ui->spinTimeScale->value());
    m_config->setValue("TimingDiagram", "Protocol", ui->comboProtocol->currentText());
    m_config->sync();
}

void TimingDiagramModule::onAddChannel()
{
    m_logger->info("TimingDiagram", "添加时序图通道");
    drawTimingDiagram();
}

void TimingDiagramModule::onClearChannels()
{
    ui->editWaveform->clear();
    clearDiagram();
    m_logger->info("TimingDiagram", "清空时序图通道");
}

void TimingDiagramModule::onExportImage()
{
    QString filePath = QFileDialog::getSaveFileName(this, "导出时序图", "", 
        "PNG Files (*.png);;SVG Files (*.svg);;All Files (*)");
    
    if (!filePath.isEmpty()) {
        QPixmap pixmap = m_graphicsView->grab();
        pixmap.save(filePath);
        QMessageBox::information(this, "成功", "时序图导出成功！");
        m_logger->info("TimingDiagram", QString("时序图导出成功: %1").arg(filePath));
    }
}

void TimingDiagramModule::onExportText()
{
    QString filePath = QFileDialog::getSaveFileName(this, "导出时序图文本", "", 
        "Text Files (*.txt);;All Files (*)");
    
    if (!filePath.isEmpty()) {
        QFile file(filePath);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            out << "时序图数据\n";
            out << "协议类型: " << m_currentProtocol << "\n";
            out << "通道数量: " << ui->spinChannelCount->value() << "\n";
            out << "时间缩放: " << ui->spinTimeScale->value() << "\n";
            out << "波形数据:\n";
            out << ui->editWaveform->toPlainText();
            file.close();
            QMessageBox::information(this, "成功", "时序图文本导出成功！");
            m_logger->info("TimingDiagram", QString("时序图文本导出成功: %1").arg(filePath));
        }
    }
}

void TimingDiagramModule::onProtocolChanged(const QString &protocol)
{
    m_currentProtocol = protocol;
    m_logger->info("TimingDiagram", QString("协议切换为: %1").arg(protocol));
    drawTimingDiagram();
}

void TimingDiagramModule::drawTimingDiagram()
{
    clearDiagram();
    
    if (m_currentProtocol == "SPI") {
        drawSPI();
    } else if (m_currentProtocol == "I2C") {
        drawI2C();
    } else if (m_currentProtocol == "USART") {
        drawUSART();
    } else if (m_currentProtocol == "UART") {
        drawUART();
    } else if (m_currentProtocol == "I2S") {
        drawI2S();
    }
}

void TimingDiagramModule::clearDiagram()
{
    m_scene->clear();
}

void TimingDiagramModule::drawSPI()
{
    clearDiagram();
    
    int channelCount = ui->spinChannelCount->value();
    double timeScale = ui->spinTimeScale->value();
    
    QPen clockPen(Qt::blue, 2);
    QPen mosiPen(Qt::red, 2);
    QPen misoPen(Qt::green, 2);
    QPen csPen(QColor(255, 165, 0), 2);
    QPen textPen(Qt::black, 1);
    
    int yOffset = 40;
    int xStart = 80;
    int bitWidth = static_cast<int>(30 * timeScale);
    
    for (int ch = 0; ch < channelCount; ++ch) {
        int y = yOffset + ch * 80;
        
        m_scene->addText(QString("CH%1").arg(ch), QFont("Arial", 9))->setPos(10, y - 15);
        
        if (m_waveformData.size() > ch) {
            QString clockData = m_waveformData[ch];
            QString mosiData = m_waveformData.size() > ch + channelCount ? m_waveformData[ch + channelCount] : "00000000";
            QString misoData = m_waveformData.size() > ch + channelCount * 2 ? m_waveformData[ch + channelCount * 2] : "00000000";
            QString csData = m_waveformData.size() > ch + channelCount * 3 ? m_waveformData[ch + channelCount * 3] : "11111111";
            
            m_scene->addText("CLK", QFont("Arial", 8))->setPos(xStart - 20, y);
            drawClockSignal(xStart, y, clockData, bitWidth, clockPen);
            
            m_scene->addText("MOSI", QFont("Arial", 8))->setPos(xStart - 20, y + 25);
            drawDataSignal(xStart, y + 25, mosiData, bitWidth, mosiPen);
            
            m_scene->addText("MISO", QFont("Arial", 8))->setPos(xStart - 20, y + 50);
            drawDataSignal(xStart, y + 50, misoData, bitWidth, misoPen);
            
            m_scene->addText("CS", QFont("Arial", 8))->setPos(xStart - 20, y + 75);
            drawBusSignal(xStart, y + 75, csData, bitWidth, csPen);
        } else {
            m_scene->addText("CLK", QFont("Arial", 8))->setPos(xStart - 20, y);
            m_scene->addLine(xStart, y, xStart + 200, y, clockPen);
            
            m_scene->addText("MOSI", QFont("Arial", 8))->setPos(xStart - 20, y + 25);
            m_scene->addLine(xStart, y + 25, xStart + 200, y + 25, mosiPen);
            
            m_scene->addText("MISO", QFont("Arial", 8))->setPos(xStart - 20, y + 50);
            m_scene->addLine(xStart, y + 50, xStart + 200, y + 50, misoPen);
            
            m_scene->addText("CS", QFont("Arial", 8))->setPos(xStart - 20, y + 75);
            m_scene->addLine(xStart, y + 75, xStart + 200, y + 75, csPen);
        }
        
        y += 80;
    }
    
    int totalWidth = xStart + 200 + (m_waveformData.size() > 0 ? m_waveformData[0].length() * bitWidth : 0);
    int totalHeight = yOffset + channelCount * 80;
    m_scene->setSceneRect(0, 0, totalWidth, totalHeight);
}

void TimingDiagramModule::parseWaveformData()
{
    QString text = ui->editWaveform->toPlainText().trimmed();
    m_waveformData.clear();
    
    if (!text.isEmpty()) {
        QStringList lines = text.split('\n', Qt::SkipEmptyParts);
        for (const QString &line : lines) {
            m_waveformData.append(line.trimmed());
        }
    }
}

void TimingDiagramModule::drawClockSignal(int x, int y, const QString &data, int bitWidth, const QPen &pen)
{
    int currentLevel = 0;
    int halfHeight = 10;
    
    for (int i = 0; i < data.length(); ++i) {
        QChar bit = data[i];
        int startX = x + i * bitWidth;
        int endX = startX + bitWidth;
        
        if (bit == '0') {
            if (currentLevel == 1) {
                m_scene->addLine(startX, y, startX, y + halfHeight, pen);
            }
            currentLevel = 0;
            m_scene->addLine(startX, y + halfHeight, endX, y + halfHeight, pen);
        } else if (bit == '1') {
            if (currentLevel == 0) {
                m_scene->addLine(startX, y, startX, y + halfHeight, pen);
            }
            currentLevel = 1;
            m_scene->addLine(startX, y, endX, y, pen);
        }
    }
    
    if (currentLevel == 1) {
        m_scene->addLine(x + data.length() * bitWidth, y, x + data.length() * bitWidth, y + halfHeight, pen);
    }
}

void TimingDiagramModule::drawDataSignal(int x, int y, const QString &data, int bitWidth, const QPen &pen)
{
    int currentLevel = 0;
    int halfHeight = 10;
    
    for (int i = 0; i < data.length(); ++i) {
        QChar bit = data[i];
        int startX = x + i * bitWidth;
        int endX = startX + bitWidth;
        
        if (bit == '0') {
            if (currentLevel == 1) {
                m_scene->addLine(startX, y, startX, y + halfHeight, pen);
            }
            currentLevel = 0;
            m_scene->addLine(startX, y + halfHeight, endX, y + halfHeight, pen);
        } else if (bit == '1') {
            if (currentLevel == 0) {
                m_scene->addLine(startX, y + halfHeight, startX, y, pen);
            }
            currentLevel = 1;
            m_scene->addLine(startX, y, endX, y, pen);
        }
    }
}

void TimingDiagramModule::drawBusSignal(int x, int y, const QString &data, int bitWidth, const QPen &pen)
{
    int currentLevel = 0;
    int halfHeight = 10;
    
    for (int i = 0; i < data.length(); ++i) {
        QChar bit = data[i];
        int startX = x + i * bitWidth;
        int endX = startX + bitWidth;
        
        if (bit == '0') {
            if (currentLevel == 1) {
                m_scene->addLine(startX, y, startX, y + halfHeight, pen);
            }
            currentLevel = 0;
            m_scene->addLine(startX, y + halfHeight, endX, y + halfHeight, pen);
        } else if (bit == '1') {
            if (currentLevel == 0) {
                m_scene->addLine(startX, y + halfHeight, startX, y, pen);
            }
            currentLevel = 1;
            m_scene->addLine(startX, y, endX, y, pen);
        }
    }
}

void TimingDiagramModule::drawI2C()
{
    clearDiagram();
    
    int channelCount = ui->spinChannelCount->value();
    double timeScale = ui->spinTimeScale->value();
    
    QPen sclPen(Qt::blue, 2);
    QPen sdaPen(Qt::red, 2);
    QPen textPen(Qt::black, 1);
    
    int yOffset = 40;
    int xStart = 80;
    int bitWidth = static_cast<int>(30 * timeScale);
    
    for (int ch = 0; ch < channelCount; ++ch) {
        int y = yOffset + ch * 80;
        
        m_scene->addText(QString("CH%1").arg(ch), QFont("Arial", 9))->setPos(10, y - 15);
        
        if (m_waveformData.size() > ch) {
            QString sclData = m_waveformData[ch];
            QString sdaData = m_waveformData.size() > ch + channelCount ? m_waveformData[ch + channelCount] : "00000000";
            
            m_scene->addText("SCL", QFont("Arial", 8))->setPos(xStart - 20, y);
            drawClockSignal(xStart, y, sclData, bitWidth, sclPen);
            
            m_scene->addText("SDA", QFont("Arial", 8))->setPos(xStart - 20, y + 40);
            drawDataSignal(xStart, y + 40, sdaData, bitWidth, sdaPen);
        } else {
            m_scene->addText("SCL", QFont("Arial", 8))->setPos(xStart - 20, y);
            m_scene->addLine(xStart, y, xStart + 200, y, sclPen);
            
            m_scene->addText("SDA", QFont("Arial", 8))->setPos(xStart - 20, y + 40);
            m_scene->addLine(xStart, y + 40, xStart + 200, y + 40, sdaPen);
        }
        
        y += 80;
    }
    
    int totalWidth = xStart + 200 + (m_waveformData.size() > 0 ? m_waveformData[0].length() * bitWidth : 0);
    int totalHeight = yOffset + channelCount * 80;
    m_scene->setSceneRect(0, 0, totalWidth, totalHeight);
}

void TimingDiagramModule::drawUSART()
{
    clearDiagram();
    
    int channelCount = ui->spinChannelCount->value();
    double timeScale = ui->spinTimeScale->value();
    
    QPen txPen(Qt::blue, 2);
    QPen rxPen(Qt::red, 2);
    QPen textPen(Qt::black, 1);
    
    int yOffset = 40;
    int xStart = 80;
    int bitWidth = static_cast<int>(30 * timeScale);
    
    for (int ch = 0; ch < channelCount; ++ch) {
        int y = yOffset + ch * 80;
        
        m_scene->addText(QString("CH%1").arg(ch), QFont("Arial", 9))->setPos(10, y - 15);
        
        if (m_waveformData.size() > ch) {
            QString txData = m_waveformData[ch];
            QString rxData = m_waveformData.size() > ch + channelCount ? m_waveformData[ch + channelCount] : "00000000";
            
            m_scene->addText("TX", QFont("Arial", 8))->setPos(xStart - 20, y);
            drawDataSignal(xStart, y + 25, txData, bitWidth, txPen);
            
            m_scene->addText("RX", QFont("Arial", 8))->setPos(xStart - 20, y + 50);
            drawDataSignal(xStart, y + 50, rxData, bitWidth, rxPen);
        } else {
            m_scene->addText("TX", QFont("Arial", 8))->setPos(xStart - 20, y);
            m_scene->addLine(xStart, y + 25, xStart + 200, y + 25, txPen);
            
            m_scene->addText("RX", QFont("Arial", 8))->setPos(xStart - 20, y + 50);
            m_scene->addLine(xStart, y + 50, xStart + 200, y + 50, rxPen);
        }
        
        y += 80;
    }
    
    int totalWidth = xStart + 200 + (m_waveformData.size() > 0 ? m_waveformData[0].length() * bitWidth : 0);
    int totalHeight = yOffset + channelCount * 80;
    m_scene->setSceneRect(0, 0, totalWidth, totalHeight);
}

void TimingDiagramModule::drawUART()
{
    clearDiagram();
    
    int channelCount = ui->spinChannelCount->value();
    double timeScale = ui->spinTimeScale->value();
    
    QPen txPen(Qt::blue, 2);
    QPen rxPen(Qt::red, 2);
    QPen textPen(Qt::black, 1);
    
    int yOffset = 40;
    int xStart = 80;
    int bitWidth = static_cast<int>(30 * timeScale);
    
    for (int ch = 0; ch < channelCount; ++ch) {
        int y = yOffset + ch * 80;
        
        m_scene->addText(QString("CH%1").arg(ch), QFont("Arial", 9))->setPos(10, y - 15);
        
        if (m_waveformData.size() > ch) {
            QString txData = m_waveformData[ch];
            QString rxData = m_waveformData.size() > ch + channelCount ? m_waveformData[ch + channelCount] : "00000000";
            
            m_scene->addText("TX", QFont("Arial", 8))->setPos(xStart - 20, y);
            drawDataSignal(xStart, y + 25, txData, bitWidth, txPen);
            
            m_scene->addText("RX", QFont("Arial", 8))->setPos(xStart - 20, y + 50);
            drawDataSignal(xStart, y + 50, rxData, bitWidth, rxPen);
        } else {
            m_scene->addText("TX", QFont("Arial", 8))->setPos(xStart - 20, y);
            m_scene->addLine(xStart, y + 25, xStart + 200, y + 25, txPen);
            
            m_scene->addText("RX", QFont("Arial", 8))->setPos(xStart - 20, y + 50);
            m_scene->addLine(xStart, y + 50, xStart + 200, y + 50, rxPen);
        }
        
        y += 80;
    }
    
    int totalWidth = xStart + 200 + (m_waveformData.size() > 0 ? m_waveformData[0].length() * bitWidth : 0);
    int totalHeight = yOffset + channelCount * 80;
    m_scene->setSceneRect(0, 0, totalWidth, totalHeight);
}

void TimingDiagramModule::drawI2S()
{
    clearDiagram();
    
    int channelCount = ui->spinChannelCount->value();
    double timeScale = ui->spinTimeScale->value();
    
    QPen sclkPen(Qt::blue, 2);
    QPen lrclkPen(Qt::red, 2);
    QPen sdataPen(Qt::green, 2);
    QPen textPen(Qt::black, 1);
    
    int yOffset = 40;
    int xStart = 80;
    int bitWidth = static_cast<int>(30 * timeScale);
    
    for (int ch = 0; ch < channelCount; ++ch) {
        int y = yOffset + ch * 80;
        
        m_scene->addText(QString("CH%1").arg(ch), QFont("Arial", 9))->setPos(10, y - 15);
        
        if (m_waveformData.size() > ch) {
            QString sclkData = m_waveformData[ch];
            QString lrclkData = m_waveformData.size() > ch + channelCount ? m_waveformData[ch + channelCount] : "00000000";
            QString sdataData = m_waveformData.size() > ch + channelCount * 2 ? m_waveformData[ch + channelCount * 2] : "00000000";
            
            m_scene->addText("SCLK", QFont("Arial", 8))->setPos(xStart - 20, y);
            drawClockSignal(xStart, y, sclkData, bitWidth, sclkPen);
            
            m_scene->addText("LRCLK", QFont("Arial", 8))->setPos(xStart - 20, y + 30);
            drawClockSignal(xStart, y + 30, lrclkData, bitWidth, lrclkPen);
            
            m_scene->addText("SDATA", QFont("Arial", 8))->setPos(xStart - 20, y + 60);
            drawDataSignal(xStart, y + 60, sdataData, bitWidth, sdataPen);
        } else {
            m_scene->addText("SCLK", QFont("Arial", 8))->setPos(xStart - 20, y);
            m_scene->addLine(xStart, y, xStart + 200, y, sclkPen);
            
            m_scene->addText("LRCLK", QFont("Arial", 8))->setPos(xStart - 20, y + 30);
            m_scene->addLine(xStart, y + 30, xStart + 200, y + 30, lrclkPen);
            
            m_scene->addText("SDATA", QFont("Arial", 8))->setPos(xStart - 20, y + 60);
            m_scene->addLine(xStart, y + 60, xStart + 200, y + 60, sdataPen);
        }
        
        y += 80;
    }
    
    int totalWidth = xStart + 200 + (m_waveformData.size() > 0 ? m_waveformData[0].length() * bitWidth : 0);
    int totalHeight = yOffset + channelCount * 80;
    m_scene->setSceneRect(0, 0, totalWidth, totalHeight);
}
