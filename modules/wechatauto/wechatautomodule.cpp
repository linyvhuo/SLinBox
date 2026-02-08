
#include "wechatautomodule.h"
#include "ui_wechatautomodule.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QStandardPaths>
#include <QTextStream>
#include <QFile>
#include <QDir>
#include <QScreen>
#include <QGuiApplication>
#include <QElapsedTimer>
#include <QDebug>
#include <QListWidget>
#include <QTextEdit>
#include <QDialog>
#include <QVBoxLayout>
#include <QPushButton>
#include <QThread>
#include <QMutex>
#include "core/globalhotkeymanager.h"

WeChatAutoWorker::WeChatAutoWorker(QObject *parent)
    : QObject(parent)
    , m_running(false)
    , m_configManager(ConfigManager::instance())
{
}

WeChatAutoWorker::~WeChatAutoWorker()
{
}

void WeChatAutoWorker::setConfig(const QMap<QString, QVariant> &config)
{
    QMutexLocker locker(&m_mutex);
    m_config = config;
}

void WeChatAutoWorker::setQuestions(const QStringList &questions)
{
    QMutexLocker locker(&m_mutex);
    m_questions = questions;
}

void WeChatAutoWorker::stop()
{
    QMutexLocker locker(&m_mutex);
    m_running = false;
}

bool WeChatAutoWorker::shouldStop()
{
    QMutexLocker locker(&m_mutex);
    return !m_running;
}

void WeChatAutoWorker::startAutomation()
{
    {
        QMutexLocker locker(&m_mutex);
        m_running = true;
    }
        
    if (!startWeChat()) {
        emit errorOccurred("启动企业微信失败");
        emit finished();
        return;
    }
    
    QThread::msleep(2000);
    
    if (!activateWeChatWindow()) {
        emit errorOccurred("激活企业微信窗口失败");
        emit finished();
        return;
    }
    
    
    QString iconPriority[] = {"input_box", "history_chat", "mindspark_tab", "mindspark_app", "workbench"};
    int iconCount = sizeof(iconPriority) / sizeof(iconPriority[0]);
    
    emit progressChanged(0, iconCount);
    
    bool iconRecognized = false;
    QPoint iconPosition;
    int recognizedIndex = -1;
    
    for (int i = 0; i < iconCount; ++i) {
        if (!m_running) {
            emit finished();
            return;
        }
                
        int maxTry = m_config.value("MaxRecognizeTry", 3).toInt();
        
        for (int tryCount = 0; tryCount < maxTry; ++tryCount) {
            if (!m_running) {
                emit finished();
                return;
            }
            
            if (recognizeIcon(iconPriority[i], iconPosition)) {
                iconRecognized = true;
                recognizedIndex = i;
                
                if (i < 4) {
                    if (!clickIcon(iconPosition)) {
                        emit errorOccurred("点击图标失败");
                        emit finished();
                        return;
                    }
                    QThread::msleep(100);
                } else {
                    emit statusChanged("进入工作台，正在打开MindSpark...");
                    if (!clickIcon(iconPosition)) {
                        emit errorOccurred("点击工作台失败");
                        emit finished();
                        return;
                    }
                    QThread::msleep(100);
                }
                break;
            }
            
            QThread::msleep(100);
        }
        
        if (iconRecognized) {
            break;
        }
    }
    
    if (!iconRecognized) {
        emit errorOccurred("未能识别到任何图标");
        emit finished();
        return;
    }
    
    if (recognizedIndex == 4) {
        emit statusChanged("正在识别并打开MindSpark应用...");
        if (!recognizeAndOpenIcon("mindspark_app")) {
            emit errorOccurred("识别并打开MindSpark应用失败");
            emit finished();
            return;
        }
        
        emit statusChanged("正在识别并打开历史对话...");
        if (!recognizeAndOpenIcon("history_chat")) {
            emit errorOccurred("识别并打开历史对话失败");
            emit finished();
            return;
        }
    }
    
    if (!executeQuestionLoop()) {
        emit finished();
        return;
    }
    
    emit statusChanged("自动化完成");
    emit finished();
}

bool WeChatAutoWorker::startWeChat()
{
    QString weChatPath = m_config.value("WeChatPath", "").toString();
    
    if (weChatPath.isEmpty()) {
        emit statusChanged("企业微信路径为空，正在尝试查找运行中的进程...");
        WindowHelper *windowHelper = WindowHelper::instance();
        if (windowHelper->isProcessRunning("WeCom.exe") || 
            windowHelper->isProcessRunning("WXWork.exe")) {
            return true;
        }
        return false;
    }
    
    WindowHelper *windowHelper = WindowHelper::instance();
    
    if (windowHelper->isProcessRunning("WeCom.exe") || 
        windowHelper->isProcessRunning("WXWork.exe")) {
        return true;
    }
    
    emit statusChanged("正在启动企业微信...");
    
    if (!windowHelper->startProcess(weChatPath)) {
        return false;
    }
    
    QThread::msleep(2000);
    
    return true;
}

bool WeChatAutoWorker::activateWeChatWindow()
{
    WindowHelper *windowHelper = WindowHelper::instance();
    
    bool windowTopMost = m_config.value("WindowTopMost", false).toBool();
    
    if (!windowHelper->activateWindowByProcess("WeCom.exe") && 
        !windowHelper->activateWindowByProcess("WXWork.exe")) {
        return false;
    }
    
    QThread::msleep(100);
    
    if (windowTopMost) {
        windowHelper->setWindowTopMost("WeChat", true);
    }
    
    return true;
}

bool WeChatAutoWorker::recognizeIcon(const QString &iconName, QPoint &position)
{
#ifdef USE_OPENCV
    QString templatePath = getTemplatePath(iconName);
    if (templatePath.isEmpty()) {
        emit statusChanged(QString("图标路径为空: %1").arg(iconName));
        return false;
    }
    
    cv::Mat templ = loadTemplate(templatePath);
    if (templ.empty()) {
        emit statusChanged(QString("图标模板加载失败: %1").arg(templatePath));
        return false;
    }
    
    ScreenHelper *screenHelper = ScreenHelper::instance();
    int mainDisplayIndex = m_config.value("MainDisplayIndex", 0).toInt();
    QPixmap screenPixmap = screenHelper->captureScreen(mainDisplayIndex);
    
    if (screenPixmap.isNull()) {
        emit statusChanged("屏幕截图失败");
        return false;
    }
    
    QImage screenImage = screenPixmap.toImage();
    cv::Mat screenMat(screenImage.height(), screenImage.width(), CV_8UC4,
                     const_cast<uchar*>(screenImage.bits()), screenImage.bytesPerLine());
    
    cv::Mat screenGray;
    cv::cvtColor(screenMat, screenGray, cv::COLOR_BGRA2GRAY);
    
    cv::Mat templGray;
    if (templ.channels() == 3) {
        cv::cvtColor(templ, templGray, cv::COLOR_BGR2GRAY);
    } else if (templ.channels() == 4) {
        cv::cvtColor(templ, templGray, cv::COLOR_BGRA2GRAY);
    } else {
        templGray = templ.clone();
    }
    
    cv::Point matchLoc;
    double threshold = m_config.value("RecognizeThreshold", 0.8).toDouble();
    
    if (!matchTemplate(screenGray, templGray, matchLoc, threshold)) {
        return false;
    }
    
    position = QPoint(matchLoc.x + templGray.cols / 2, matchLoc.y + templGray.rows / 2);
    return true;
#else
    Q_UNUSED(iconName);
    Q_UNUSED(position);
    emit errorOccurred("OpenCV不可用，图标识别已禁用");
    return false;
#endif
}

bool WeChatAutoWorker::clickIcon(const QPoint &position)
{
    WindowHelper *windowHelper = WindowHelper::instance();
    
    ScreenHelper *screenHelper = ScreenHelper::instance();
    int mainDisplayIndex = m_config.value("MainDisplayIndex", 0).toInt();
    QRect screenRect = screenHelper->getScreenGeometry(mainDisplayIndex);
    
    QPoint globalPos = screenRect.topLeft() + position;
    
    return windowHelper->clickAt(globalPos.x(), globalPos.y());
}

bool WeChatAutoWorker::recognizeAndOpenIcon(const QString &iconName)
{
    int maxTry = m_config.value("MaxRecognizeTry", 3).toInt();
    
    for (int tryCount = 0; tryCount < maxTry; ++tryCount) {
        if (!m_running) {
            return false;
        }
        
        QPoint iconPosition;
        if (recognizeIcon(iconName, iconPosition)) {
            if (!clickIcon(iconPosition)) {
                emit statusChanged(QString("点击图标失败: %1").arg(iconName));
                return false;
            }
            QThread::msleep(100);
            return true;
        }
        
        QThread::msleep(100);
    }
    
    return false;
}

bool WeChatAutoWorker::inputQuestion(const QString &question)
{
    WindowHelper *windowHelper = WindowHelper::instance();
        
    bool activated = windowHelper->activateWindowByProcess("WeCom.exe") || 
                   windowHelper->activateWindowByProcess("WXWork.exe");
    
    if (!activated) {
        emit statusChanged("错误: 无法激活企业微信窗口");
        return false;
    }
    
    
    QPoint inputBoxPos;
    bool foundInputBox = false;
    QStringList inputBoxTemplates = {"input_box", "input_box_small", "input_box_large"};
    
    
    for (const QString& templateName : inputBoxTemplates) {
        if (recognizeIcon(templateName, inputBoxPos)) {
            foundInputBox = true;
            break;
        }
    }
    
    if (!foundInputBox) {
        
        ScreenHelper *screenHelper = ScreenHelper::instance();
        int mainDisplayIndex = m_config.value("MainDisplayIndex", 0).toInt();
        QRect screenRect = screenHelper->getScreenGeometry(mainDisplayIndex);
        
        inputBoxPos.setX(screenRect.width() / 2);
        inputBoxPos.setY(screenRect.height() - 70);
    }
    
    
    ScreenHelper *screenHelper = ScreenHelper::instance();
    int mainDisplayIndex = m_config.value("MainDisplayIndex", 0).toInt();
    QRect screenRect = screenHelper->getScreenGeometry(mainDisplayIndex);
    QPoint globalPos = screenRect.topLeft() + inputBoxPos;
        
    
    windowHelper->clickAt(globalPos.x(), globalPos.y());
    
    
    windowHelper->clickAt(globalPos.x(), globalPos.y());
    
    QThread::msleep(100);
        
    QString fullQuestion = question;
    QString answerLimitTip = m_config.value("AnswerLimitTip", "").toString();
    if (!answerLimitTip.isEmpty()) {
        fullQuestion = question + " " + answerLimitTip;
    }
    
    
    QString inputMethod = m_config.value("InputMethod", "simulate").toString();
    
    auto stopCheck = [this]() { return shouldStop(); };
    
    bool result = false;
    if (inputMethod == "paste") {
        result = windowHelper->pasteText("WeChat", fullQuestion, stopCheck);
    } else {
        result = windowHelper->sendKeys("WeChat", fullQuestion, stopCheck);
    }
        
    return result;
}

bool WeChatAutoWorker::clickSendButton()
{
    QPoint sendButtonPos;
    if (!recognizeIcon("send_button", sendButtonPos)) {
        return false;
    }
    
    return clickIcon(sendButtonPos);
}

bool WeChatAutoWorker::waitForAnswer(int timeout)
{
    QElapsedTimer timer;
    timer.start();
    
    while (timer.elapsed() < timeout && m_running) {
        QThread::msleep(500);
    }
    
    return m_running;
}

bool WeChatAutoWorker::executeQuestionLoop()
{
    int loopCount = m_config.value("LoopCount", 10).toInt();
    int answerTimeout = m_config.value("AnswerTimeout", 10).toInt();
    int roundInterval = m_config.value("RoundInterval", 10).toInt();
    bool continueOnError = m_config.value("ContinueOnError", true).toBool();
    bool continueOnTimeout = m_config.value("ContinueOnTimeout", true).toBool();
    
    for (int i = 0; i < loopCount && m_running; ++i) {
        emit progressChanged(i + 1, loopCount);
        
        
        QString question;
        int questionCount = 0;
        {
            QMutexLocker locker(&m_mutex);
            questionCount = m_questions.size();
            
            if (m_questions.isEmpty()) {
                emit statusChanged("错误: 题库为空");
                emit errorOccurred("题库为空");
                return false;
            }
            
            question = m_questions[i % m_questions.size()];
        }
        
        if (!inputQuestion(question)) {
            emit statusChanged("错误: inputQuestion() 返回失败");
            emit errorOccurred("输入问题失败");
            if (!continueOnError) {
                return false;
            }
            continue;
        }
        
        if (!clickSendButton()) {
            emit statusChanged("错误: clickSendButton() 返回失败");
            emit errorOccurred("点击发送按钮失败");
            if (!continueOnError) {
                return false;
            }
            continue;
        }
        
        emit statusChanged("发送按钮点击成功，等待回答...");
        emit statusChanged("正在等待回答...");
        
        if (!waitForAnswer(answerTimeout)) {
            emit statusChanged("错误: waitForAnswer() 返回失败");
            emit errorOccurred("等待回答超时");
            if (!continueOnTimeout) {
                return false;
            }
        }
        
        QThread::msleep(roundInterval);
    }
    
    return true;
}

QString WeChatAutoWorker::getTemplatePath(const QString &iconName)
{
    QString configKey;
    
    if (iconName == "input_box") {
        configKey = "InputBoxIcon";
    } else if (iconName == "history_chat") {
        configKey = "HistoryChatIcon";
    } else if (iconName == "mindspark_tab") {
        configKey = "MindsparkTabIcon";
    } else if (iconName == "mindspark_app") {
        configKey = "MindsparkAppIcon";
    } else if (iconName == "workbench") {
        configKey = "WorkbenchIcon";
    } else if (iconName == "send_button") {
        configKey = "SendButtonIcon";
    } else {
        return QString();
    }
    
    QString iconPath = m_configManager->getValue("WeChatAuto", configKey, "").toString();
    
    if (iconPath.isEmpty()) {
        return QString();
    }
    
    return iconPath;
}

#ifdef USE_OPENCV
cv::Mat WeChatAutoWorker::loadTemplate(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        emit statusChanged(QString("无法打开模板图像: %1").arg(path));
        return cv::Mat();
    }
    
    QByteArray fileData = file.readAll();
    file.close();
    
    if (fileData.isEmpty()) {
        emit statusChanged(QString("模板图像为空: %1").arg(path));
        return cv::Mat();
    }
    
    std::vector<uchar> buffer(fileData.begin(), fileData.end());
    cv::Mat image = cv::imdecode(buffer, cv::IMREAD_COLOR);
    
    if (image.empty()) {
        emit statusChanged(QString("无法加载模板图像: %1").arg(path));
        return cv::Mat();
    }
    
    int originalWidth = image.cols;
    int originalHeight = image.rows;
        
    return image;
}

bool WeChatAutoWorker::matchTemplate(const cv::Mat &screen, const cv::Mat &templ, cv::Point &matchLoc, double threshold)
{
    if (screen.empty() || templ.empty()) {
        emit statusChanged("匹配失败：屏幕或模板为空");
        return false;
    }
    
    int screenWidth = screen.cols;
    int screenHeight = screen.rows;
    int templateWidth = templ.cols;
    int templateHeight = templ.rows;
    
    if (templateWidth > screenWidth || templateHeight > screenHeight) {
        emit statusChanged(QString("匹配失败：模板(%1x%2)大于屏幕(%3x%4)").arg(templateWidth).arg(templateHeight).arg(screenWidth).arg(screenHeight));
        return false;
    }
    
    cv::Mat result;
    cv::matchTemplate(screen, templ, result, cv::TM_CCOEFF_NORMED);
    
    double minVal, maxVal;
    cv::Point minLoc, maxLoc;
    cv::minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc);
        
    if (maxVal >= threshold) {
        matchLoc = maxLoc;
        return true;
    }
    
    return false;
}
#endif

WeChatAutoModule::WeChatAutoModule(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::WeChatAutoModule)
    , m_workerThread(nullptr)
    , m_worker(nullptr)
    , m_config(ConfigManager::instance())
    , m_logger(Logger::instance())
    , m_floatWindow(nullptr)
    , m_stopHotkeyRegistered(false)
{
    ui->setupUi(this);
    initUI();
    loadConfig();
}

WeChatAutoModule::~WeChatAutoModule()
{
    if (m_worker) {
        m_worker->stop();
        m_workerThread->quit();
        m_workerThread->wait();
        delete m_worker;
    }
    if (m_floatWindow) {
        delete m_floatWindow;
        m_floatWindow = nullptr;
    }
    delete ui;
}

void WeChatAutoModule::initUI()
{
    connect(ui->btnBrowseWeChatPath, &QPushButton::clicked, this, &WeChatAutoModule::onBrowseWeChatPath);
    connect(ui->btnBrowseQuestionLibrary, &QPushButton::clicked, this, &WeChatAutoModule::onBrowseQuestionLibrary);
    connect(ui->btnStart, &QPushButton::clicked, this, &WeChatAutoModule::onStartAutomation);
    connect(ui->btnStop, &QPushButton::clicked, this, &WeChatAutoModule::onStopAutomation);
    connect(ui->comboQuestionMode, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &WeChatAutoModule::onQuestionModeChanged);
    connect(ui->btnBrowseInputBoxIcon, &QPushButton::clicked, this, &WeChatAutoModule::onBrowseInputBoxIcon);
    connect(ui->btnBrowseHistoryChatIcon, &QPushButton::clicked, this, &WeChatAutoModule::onBrowseHistoryChatIcon);
    connect(ui->btnBrowseMindsparkTabIcon, &QPushButton::clicked, this, &WeChatAutoModule::onBrowseMindsparkTabIcon);
    connect(ui->btnBrowseMindsparkAppIcon, &QPushButton::clicked, this, &WeChatAutoModule::onBrowseMindsparkAppIcon);
    connect(ui->btnBrowseWorkbenchIcon, &QPushButton::clicked, this, &WeChatAutoModule::onBrowseWorkbenchIcon);
    connect(ui->btnBrowseSendButtonIcon, &QPushButton::clicked, this, &WeChatAutoModule::onBrowseSendButtonIcon);
    connect(ui->checkFloatWindowEnable, &QCheckBox::stateChanged, this, &WeChatAutoModule::onFloatWindowEnableChanged);
    
    ui->btnStop->setEnabled(false);
    ui->progressBar->setRange(0, 100);
    ui->progressBar->setValue(0);
}

void WeChatAutoModule::loadConfig()
{
    ui->editWeChatPath->setText(m_config->getValue("WeChatAuto", "WeChatPath", "").toString());
    ui->checkWindowTopMost->setChecked(m_config->getValue("WeChatAuto", "WindowTopMost", false).toBool());
    ui->spinRecognizeThreshold->setValue(m_config->getValue("WeChatAuto", "RecognizeThreshold", 0.8).toDouble());
    ui->spinMaxRecognizeTry->setValue(m_config->getValue("WeChatAuto", "MaxRecognizeTry", 3).toInt());
    ui->spinPageLoadTimeout->setValue(m_config->getValue("WeChatAuto", "PageLoadTimeout", 5000).toInt());
    ui->spinLoopCount->setValue(m_config->getValue("WeChatAuto", "LoopCount", 10).toInt());
    ui->spinAnswerTimeout->setValue(m_config->getValue("WeChatAuto", "AnswerTimeout", 30000).toInt());
    ui->spinRoundInterval->setValue(m_config->getValue("WeChatAuto", "RoundInterval", 2000).toInt());
    ui->checkContinueOnError->setChecked(m_config->getValue("WeChatAuto", "ContinueOnError", true).toBool());
    ui->checkContinueOnTimeout->setChecked(m_config->getValue("WeChatAuto", "ContinueOnTimeout", true).toBool());
    ui->editAnswerLimitTip->setText(m_config->getValue("WeChatAuto", "AnswerLimitTip", "").toString());
    ui->spinMainDisplayIndex->setValue(m_config->getValue("WeChatAuto", "MainDisplayIndex", 0).toInt());
    ui->editInputBoxIcon->setText(m_config->getValue("WeChatAuto", "InputBoxIcon", "").toString());
    ui->editHistoryChatIcon->setText(m_config->getValue("WeChatAuto", "HistoryChatIcon", "").toString());
    ui->editMindsparkTabIcon->setText(m_config->getValue("WeChatAuto", "MindsparkTabIcon", "").toString());
    ui->editMindsparkAppIcon->setText(m_config->getValue("WeChatAuto", "MindsparkAppIcon", "").toString());
    ui->editWorkbenchIcon->setText(m_config->getValue("WeChatAuto", "WorkbenchIcon", "").toString());
    ui->editSendButtonIcon->setText(m_config->getValue("WeChatAuto", "SendButtonIcon", "").toString());
    
    QString questionMode = m_config->getValue("WeChatAuto", "QuestionMode", "Loop").toString();
    int modeIndex = ui->comboQuestionMode->findText(questionMode);
    if (modeIndex >= 0) {
        ui->comboQuestionMode->setCurrentIndex(modeIndex);
    }
    
    QString questionLibraryPath = m_config->getValue("WeChatAuto", "QuestionLibraryPath", "").toString();
    if (!questionLibraryPath.isEmpty()) {
        ui->editQuestionLibrary->setText(questionLibraryPath);
        loadQuestionsFromFile(questionLibraryPath);
    }
    
    bool floatWindowEnable = m_config->getValue("WeChatAuto", "FloatWindowEnable", true).toBool();
    ui->checkFloatWindowEnable->setChecked(floatWindowEnable);
    
    QString inputMethod = m_config->getValue("WeChatAuto", "InputMethod", "simulate").toString();
    if (inputMethod == "paste") {
        ui->radioPasteInput->setChecked(true);
    } else {
        ui->radioSimulateInput->setChecked(true);
    }
    
    if (floatWindowEnable && !m_floatWindow) {
        m_floatWindow = new FloatWindow(this);
        connect(m_floatWindow, &FloatWindow::stopClicked, this, &WeChatAutoModule::onFloatWindowStopClicked);
    }
}

void WeChatAutoModule::saveConfig()
{
    m_config->setValue("WeChatAuto", "WeChatPath", ui->editWeChatPath->text());
    m_config->setValue("WeChatAuto", "WindowTopMost", ui->checkWindowTopMost->isChecked());
    m_config->setValue("WeChatAuto", "RecognizeThreshold", ui->spinRecognizeThreshold->value());
    m_config->setValue("WeChatAuto", "MaxRecognizeTry", ui->spinMaxRecognizeTry->value());
    m_config->setValue("WeChatAuto", "PageLoadTimeout", ui->spinPageLoadTimeout->value());
    m_config->setValue("WeChatAuto", "LoopCount", ui->spinLoopCount->value());
    m_config->setValue("WeChatAuto", "AnswerTimeout", ui->spinAnswerTimeout->value());
    m_config->setValue("WeChatAuto", "RoundInterval", ui->spinRoundInterval->value());
    m_config->setValue("WeChatAuto", "ContinueOnError", ui->checkContinueOnError->isChecked());
    m_config->setValue("WeChatAuto", "ContinueOnTimeout", ui->checkContinueOnTimeout->isChecked());
    m_config->setValue("WeChatAuto", "AnswerLimitTip", ui->editAnswerLimitTip->text());
    m_config->setValue("WeChatAuto", "MainDisplayIndex", ui->spinMainDisplayIndex->value());
    m_config->setValue("WeChatAuto", "QuestionMode", getQuestionModeString());
    m_config->setValue("WeChatAuto", "QuestionLibraryPath", ui->editQuestionLibrary->text());
    m_config->setValue("WeChatAuto", "InputBoxIcon", ui->editInputBoxIcon->text());
    m_config->setValue("WeChatAuto", "HistoryChatIcon", ui->editHistoryChatIcon->text());
    m_config->setValue("WeChatAuto", "MindsparkTabIcon", ui->editMindsparkTabIcon->text());
    m_config->setValue("WeChatAuto", "MindsparkAppIcon", ui->editMindsparkAppIcon->text());
    m_config->setValue("WeChatAuto", "WorkbenchIcon", ui->editWorkbenchIcon->text());
    m_config->setValue("WeChatAuto", "SendButtonIcon", ui->editSendButtonIcon->text());
    m_config->setValue("WeChatAuto", "FloatWindowEnable", ui->checkFloatWindowEnable->isChecked());
    m_config->setValue("WeChatAuto", "InputMethod", ui->radioSimulateInput->isChecked() ? "simulate" : "paste");
    m_config->sync();
}

void WeChatAutoModule::onBrowseWeChatPath()
{
    QString filePath = QFileDialog::getOpenFileName(this, "Select WeChat executable", "", "Executable (*.exe)");
    if (!filePath.isEmpty()) {
        ui->editWeChatPath->setText(filePath);
        saveConfig();
    }
}

void WeChatAutoModule::onBrowseQuestionLibrary()
{
    QString filePath = QFileDialog::getOpenFileName(this, "选择题库文件", "", "文本文件 (*.txt *.csv)");
    if (!filePath.isEmpty()) {
        ui->editQuestionLibrary->setText(filePath);
        loadQuestionsFromFile(filePath);
        saveConfig();
        
        if (!m_questions.isEmpty()) {
            showQuestionsPreview();
        }
    }
}

void WeChatAutoModule::onStartAutomation()
{
    saveConfig();
    
    if (m_questions.isEmpty()) {
        QMessageBox::warning(this, "警告", "题库为空，请先加载题库！");
        return;
    }
    
    ui->btnStart->setEnabled(false);
    ui->btnStop->setEnabled(true);
    ui->progressBar->setValue(0);
    
    if (m_floatWindow) {
        m_floatWindow->show();
        m_floatWindow->setRunning(true);
        m_floatWindow->setStatus("就绪");
        m_floatWindow->setProgress(0);
    }
    
    QMap<QString, QVariant> config;
    config["WeChatPath"] = ui->editWeChatPath->text();
    config["WindowTopMost"] = ui->checkWindowTopMost->isChecked();
    config["RecognizeThreshold"] = ui->spinRecognizeThreshold->value();
    config["MaxRecognizeTry"] = ui->spinMaxRecognizeTry->value();
    config["PageLoadTimeout"] = ui->spinPageLoadTimeout->value();
    config["LoopCount"] = ui->spinLoopCount->value();
    config["AnswerTimeout"] = ui->spinAnswerTimeout->value();
    config["RoundInterval"] = ui->spinRoundInterval->value();
    config["ContinueOnError"] = ui->checkContinueOnError->isChecked();
    config["ContinueOnTimeout"] = ui->checkContinueOnTimeout->isChecked();
    config["AnswerLimitTip"] = ui->editAnswerLimitTip->text();
    config["MainDisplayIndex"] = ui->spinMainDisplayIndex->value();
    config["InputBoxIcon"] = ui->editInputBoxIcon->text();
    config["HistoryChatIcon"] = ui->editHistoryChatIcon->text();
    config["MindsparkTabIcon"] = ui->editMindsparkTabIcon->text();
    config["MindsparkAppIcon"] = ui->editMindsparkAppIcon->text();
    config["WorkbenchIcon"] = ui->editWorkbenchIcon->text();
    config["SendButtonIcon"] = ui->editSendButtonIcon->text();
    config["InputMethod"] = ui->radioSimulateInput->isChecked() ? "simulate" : "paste";
    
    m_workerThread = new QThread(this);
    m_worker = new WeChatAutoWorker();
    m_worker->moveToThread(m_workerThread);
    
    connect(m_workerThread, &QThread::started, m_worker, &WeChatAutoWorker::startAutomation);
    connect(m_worker, &WeChatAutoWorker::progressChanged, this, &WeChatAutoModule::onProgressChanged);
    connect(m_worker, &WeChatAutoWorker::statusChanged, this, &WeChatAutoModule::onStatusChanged);
    connect(m_worker, &WeChatAutoWorker::errorOccurred, this, &WeChatAutoModule::onErrorOccurred);
    connect(m_worker, &WeChatAutoWorker::finished, this, &WeChatAutoModule::onWorkerFinished);
    
    m_worker->setConfig(config);
    m_worker->setQuestions(m_questions);
    
    m_workerThread->start();
    
    m_logger->info("WeChatAuto", "启动自动化");
}

void WeChatAutoModule::onStopAutomation()
{
    if (m_worker) {
        m_worker->stop();
        m_logger->info("WeChatAuto", "停止企业微信自动化");
    }
}

void WeChatAutoModule::onProgressChanged(int current, int total)
{
    int progress = (current * 100) / total;
    ui->progressBar->setValue(progress);
    ui->labelProgress->setText(QString("进度: %1/%2").arg(current).arg(total));
}

void WeChatAutoModule::onStatusChanged(const QString &status)
{
    ui->labelStatus->setText("状态: " + status);
    m_logger->info("WeChatAuto", status);
}

void WeChatAutoModule::onErrorOccurred(const QString &error)
{
    QMessageBox::critical(this, "错误", error);
    m_logger->error("WeChatAuto", error);
}

void WeChatAutoModule::onWorkerFinished()
{
    ui->btnStart->setEnabled(true);
    ui->btnStop->setEnabled(false);
    
    if (m_floatWindow) {
        m_floatWindow->closeWindow();
    }
    
    if (m_workerThread) {
        m_workerThread->quit();
        m_workerThread->wait();
        delete m_workerThread;
        m_workerThread = nullptr;
    }
    
    if (m_worker) {
        delete m_worker;
        m_worker = nullptr;
    }
    
    QMessageBox::information(this, "自动化完成", "自动化任务已成功完成！");
}

void WeChatAutoModule::onQuestionModeChanged(int index)
{
    QString mode = ui->comboQuestionMode->itemText(index);
    
    if (mode == "Auto Generate") {
        generateQuestionsFromKeywords();
    }
    
    saveConfig();
}

void WeChatAutoModule::loadQuestionsFromFile(const QString &filePath)
{
    m_questions.clear();
    
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "警告", "无法打开题库文件！");
        return;
    }
    
    QTextStream in(&file);
    in.setCodec("UTF-8");
    
    QString suffix = QFileInfo(filePath).suffix().toLower();
    
    if (suffix == "csv") {
        while (!in.atEnd()) {
            QString line = in.readLine();
            QStringList parts = line.split(',');
            if (parts.size() >= 1) {
                QString question = parts[0].trimmed();
                if (!question.isEmpty() && question != "Question") {
                    m_questions.append(question);
                }
            }
        }
    } else {
        while (!in.atEnd()) {
            QString line = in.readLine().trimmed();
            if (!line.isEmpty()) {
                m_questions.append(line);
            }
        }
    }
    
    file.close();
    
    m_logger->info("WeChatAuto", QString("题库加载成功，共%1个问题").arg(m_questions.size()));
}

void WeChatAutoModule::generateQuestionsFromKeywords()
{
    QString documentsPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    QString keywordsPath = documentsPath + "/SLinBox/Keywords.txt";
    
    QFile file(keywordsPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "警告", "无法打开关键词文件！");
        return;
    }
    
    QStringList keywords;
    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (!line.isEmpty()) {
            keywords.append(line);
        }
    }
    file.close();
    
    m_questions.clear();
    
    for (const QString &keyword : keywords) {
        m_questions.append(keyword + " what is it?");
        m_questions.append("Please explain " + keyword);
        m_questions.append(keyword + " what are the characteristics?");
    }
    
    m_logger->info("WeChatAuto", QString("Auto-generated questions successfully, total %1 questions").arg(m_questions.size()));
}

QString WeChatAutoModule::getQuestionModeString() const
{
    return ui->comboQuestionMode->currentText();
}

void WeChatAutoModule::onBrowseInputBoxIcon()
{
    QString defaultPath = ui->editInputBoxIcon->text();
    if (defaultPath.isEmpty()) {
        defaultPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    }
    
    QString path = QFileDialog::getOpenFileName(this, "选择输入框图标", defaultPath, "图片 (*.png *.jpg *.jpeg *.bmp)");
    if (!path.isEmpty()) {
        ui->editInputBoxIcon->setText(path);
        saveConfig();
        m_logger->info("WeChatAuto", QString("输入框图标已设置: %1").arg(path));
        
        showIconPreview(path);
    }
}

void WeChatAutoModule::onBrowseHistoryChatIcon()
{
    QString defaultPath = ui->editHistoryChatIcon->text();
    if (defaultPath.isEmpty()) {
        defaultPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    }
    
    QString path = QFileDialog::getOpenFileName(this, "Select History Chat Icon", defaultPath, "Images (*.png *.jpg *.jpeg *.bmp)");
    if (!path.isEmpty()) {
        ui->editHistoryChatIcon->setText(path);
        saveConfig();
        m_logger->info("WeChatAuto", QString("History chat icon set: %1").arg(path));
    }
}

void WeChatAutoModule::onBrowseMindsparkTabIcon()
{
    QString defaultPath = ui->editMindsparkTabIcon->text();
    if (defaultPath.isEmpty()) {
        defaultPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    }
    
    QString path = QFileDialog::getOpenFileName(this, "Select Mindspark Tab Icon", defaultPath, "Images (*.png *.jpg *.jpeg *.bmp)");
    if (!path.isEmpty()) {
        ui->editMindsparkTabIcon->setText(path);
        saveConfig();
        m_logger->info("WeChatAuto", QString("Mindspark tab icon set: %1").arg(path));
    }
}

void WeChatAutoModule::onBrowseMindsparkAppIcon()
{
    QString defaultPath = ui->editMindsparkAppIcon->text();
    if (defaultPath.isEmpty()) {
        defaultPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    }
    
    QString path = QFileDialog::getOpenFileName(this, "Select Mindspark App Icon", defaultPath, "Images (*.png *.jpg *.jpeg *.bmp)");
    if (!path.isEmpty()) {
        ui->editMindsparkAppIcon->setText(path);
        saveConfig();
        m_logger->info("WeChatAuto", QString("Mindspark app icon set: %1").arg(path));
    }
}

void WeChatAutoModule::onBrowseWorkbenchIcon()
{
    QString defaultPath = ui->editWorkbenchIcon->text();
    if (defaultPath.isEmpty()) {
        defaultPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    }
    
    QString path = QFileDialog::getOpenFileName(this, "Select Workbench Icon", defaultPath, "Images (*.png *.jpg *.jpeg *.bmp)");
    if (!path.isEmpty()) {
        ui->editWorkbenchIcon->setText(path);
        saveConfig();
        m_logger->info("WeChatAuto", QString("Workbench icon set: %1").arg(path));
    }
}

void WeChatAutoModule::onBrowseSendButtonIcon()
{
    QString defaultPath = ui->editSendButtonIcon->text();
    if (defaultPath.isEmpty()) {
        defaultPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    }
    
    QString path = QFileDialog::getOpenFileName(this, "Select Send Button Icon", defaultPath, "Images (*.png *.jpg *.jpeg *.bmp)");
    if (!path.isEmpty()) {
        ui->editSendButtonIcon->setText(path);
        saveConfig();
        m_logger->info("WeChatAuto", QString("Send button icon set: %1").arg(path));
    }
}

void WeChatAutoModule::showQuestionsPreview()
{
    if (m_questions.isEmpty()) {
        return;
    }
    
    QDialog previewDialog(this);
    previewDialog.setWindowTitle("题库预览");
    previewDialog.resize(600, 400);
    
    QVBoxLayout *layout = new QVBoxLayout(&previewDialog);
    
    QTextEdit *questionText = new QTextEdit(&previewDialog);
    questionText->setReadOnly(true);
    
    QString allQuestions;
    for (int i = 0; i < m_questions.size(); ++i) {
        allQuestions += QString("%1. %2\n").arg(i + 1).arg(m_questions[i]);
    }
    
    questionText->setPlainText(allQuestions);
    layout->addWidget(questionText);
    
    QPushButton *closeBtn = new QPushButton("关闭", &previewDialog);
    connect(closeBtn, &QPushButton::clicked, &previewDialog, &QDialog::accept);
    layout->addWidget(closeBtn);
    
    previewDialog.exec();
    
    m_logger->info("WeChatAuto", QString("题库预览完成，共%1个问题").arg(m_questions.size()));
}

void WeChatAutoModule::showIconPreview(const QString &iconPath)
{
    if (iconPath.isEmpty()) {
        return;
    }
    
    QFileInfo fileInfo(iconPath);
    if (!fileInfo.exists() || !fileInfo.isFile()) {
        return;
    }
    
    QDialog previewDialog(this);
    previewDialog.setWindowTitle("图标预览");
    previewDialog.resize(400, 400);
    
    QVBoxLayout *layout = new QVBoxLayout(&previewDialog);
    
    QLabel *iconLabel = new QLabel(&previewDialog);
    iconLabel->setAlignment(Qt::AlignCenter);
    QPixmap pixmap(iconPath);
    if (!pixmap.isNull()) {
        iconLabel->setPixmap(pixmap.scaled(200, 200, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        iconLabel->setText("无法加载图片");
    }
    layout->addWidget(iconLabel);
    
    QLabel *pathLabel = new QLabel(fileInfo.fileName(), &previewDialog);
    pathLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(pathLabel);
    
    QPushButton *closeBtn = new QPushButton("关闭", &previewDialog);
    connect(closeBtn, &QPushButton::clicked, &previewDialog, &QDialog::accept);
    layout->addWidget(closeBtn);
    
    previewDialog.exec();
    
    m_logger->info("WeChatAuto", QString("图标预览完成: %1").arg(fileInfo.fileName()));
}


void WeChatAutoModule::onPreviewQuestions()
{
    showQuestionsPreview();
}

void WeChatAutoModule::onFloatWindowStopClicked()
{
    if (m_worker) {
        m_worker->stop();
        m_logger->info("WeChatAuto", "通过悬浮窗停止自动化");
    }
}

void WeChatAutoModule::onFloatWindowEnableChanged(int state)
{
    bool enable = (state == Qt::Checked);
    
    if (enable && !m_floatWindow) {
        m_floatWindow = new FloatWindow(this);
        connect(m_floatWindow, &FloatWindow::stopClicked, this, &WeChatAutoModule::onFloatWindowStopClicked);
    } else if (!enable && m_floatWindow) {
        delete m_floatWindow;
        m_floatWindow = nullptr;
    }
}

void WeChatAutoModule::setupStopHotkey(const QString &hotkeyStr)
{
    GlobalHotkeyManager *hotkeyManager = GlobalHotkeyManager::instance();
    
    if (m_stopHotkeyRegistered) {
        QKeySequence oldHotkey = QKeySequence(m_config->getValue("WeChatAuto", "StopHotkey", "Ctrl+Shift+X").toString());
        if (!oldHotkey.isEmpty()) {
            hotkeyManager->unregisterHotkey(oldHotkey);
            m_logger->info("WeChatAuto", QString("已注销旧停止快捷键: %1").arg(oldHotkey.toString()));
        }
    }
    
    QKeySequence hotkey = QKeySequence(hotkeyStr);
    if (!hotkey.isEmpty()) {
        if (hotkeyManager->registerHotkey(hotkey, this, SLOT(onStopHotkeyTriggered()))) {
            m_stopHotkeyRegistered = true;
            m_logger->info("WeChatAuto", QString("停止快捷键已设置: %1").arg(hotkey.toString()));
        } else {
            m_stopHotkeyRegistered = false;
            m_logger->error("WeChatAuto", QString("停止快捷键注册失败: %1").arg(hotkey.toString()));
        }
    }
}

void WeChatAutoModule::onStopHotkeyTriggered()
{
    m_logger->info("WeChatAuto", "停止快捷键已触发");
    onStopAutomation();
}
