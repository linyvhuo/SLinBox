#include "userguidedialog.h"
#include "ui_userguidedialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QFont>

UserGuideDialog::UserGuideDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::UserGuideDialog)
{
    ui->setupUi(this);
    initUI();
    loadGuideContent();
}

UserGuideDialog::~UserGuideDialog()
{
    delete ui;
}

void UserGuideDialog::initUI()
{
    setWindowTitle("使用说明");
    resize(800, 600);
    
    connect(ui->btnOk, &QPushButton::clicked, this, &UserGuideDialog::onOkClicked);
}

void UserGuideDialog::loadGuideContent()
{
    QString guideContent = R"(
<h1>欢迎使用 SLinBox 智能工具箱！</h1>

<h2>首次使用配置指南</h2>

<h3>1. 配置截图快捷键</h3>
<p>• 打开 <b>设置</b> 对话框，切换到 <b>模块</b> 标签页</p>
<p>• 在 <b>截图+OCR</b> 区域中，点击快捷键输入框</p>
<p>• 按下您想要设置的快捷键组合（例如：Ctrl+Shift+S）</p>
<p>• 点击 <b>确定</b> 保存设置</p>
<p>• 现在您可以使用快捷键快速截图了！</p>

<h3>2. 配置企业微信路径</h3>
<p>• 切换到 <b>微信自动化</b> 标签页</p>
<p>• 点击 <b>浏览</b> 按钮，选择企业微信的可执行文件（WeCom.exe 或 WXWork.exe）</p>
<p>• 配置图标识别模板（可选）</p>
<p>• 设置自动回复题库（可选）</p>
<p>• 点击 <b>开始</b> 按钮启动自动化</p>

<h3>3. 配置OCR引擎</h3>
<p>• 在 <b>设置</b> 对话框的 <b>模块</b> 标签页中</p>
<p>• 选择 <b>OCR引擎</b>（推荐使用 Umi-OCR）</p>
<p>• 设置 <b>OCR语言</b>（简体中文/English）</p>
<p>• 调整 <b>识别置信度</b>（0.0-1.0，默认0.7）</p>
<p>• 启用 <b>自动识别</b> 选项，截图后自动进行OCR识别</p>

<h3>4. 配置串口通信</h3>
<p>• 切换到 <b>串口通信</b> 标签页</p>
<p>• 选择 <b>串口</b>（COM1、COM2等）</p>
<p>• 设置 <b>波特率</b>（9600、115200等）</p>
<p>• 配置 <b>数据位</b>、<b>停止位</b>、<b>校验位</b></p>
<p>• 点击 <b>打开串口</b> 开始通信</p>

<h3>5. 配置停止按钮快捷键</h3>
<p>• 在 <b>设置</b> 对话框的 <b>模块</b> 标签页中</p>
<p>• 在 <b>企业微信自动化</b> 区域下方，找到 <b>停止按钮快捷键</b></p>
<p>• 点击快捷键输入框，按下您想要设置的快捷键（例如：Ctrl+Shift+X）</p>
<p>• 点击 <b>确定</b> 保存设置</p>
<p>• 现在您可以使用快捷键快速停止自动化任务了！</p>

<h3>6. 启用悬浮窗</h3>
<p>• 在 <b>微信自动化</b> 标签页中</p>
<p>• 勾选 <b>启用悬浮窗</b> 选项</p>
<p>• 悬浮窗将显示在屏幕上，显示当前状态和进度</p>
<p>• 点击悬浮窗上的 <b>停止</b> 按钮可以快速停止任务</p>

<h2>其他功能</h2>

<h3>进制转换</h3>
<p>• 支持二进制、八进制、十进制、十六进制之间的相互转换</p>
<p>• 输入任意一种进制的数值，自动转换为其他进制</p>

<h3>CRC校验</h3>
<p>• 支持多种CRC算法（CRC-8、CRC-16、CRC-32等）</p>
<p>• 输入数据，自动计算CRC校验值</p>

<h3>时序图</h3>
<p>• 支持SPI、I2C、USART、UART、I2S等通信协议</p>
<p>• 可以绘制和导出时序图</p>

<h3>延时计算</h3>
<p>• 计算代码执行延时</p>
<p>• 输入时钟频率和指令周期数，自动计算延时时间</p>

<h3>ASCII查询</h3>
<p>• 查询ASCII码表</p>
<p>• 支持字符和十进制、十六进制之间的转换</p>

<h3>校验和计算</h3>
<p>• 计算数据的校验和</p>
<p>• 支持多种校验和算法</p>

<h3>数据手册</h3>
<p>• 查看常用芯片的数据手册</p>
<p>• 支持PDF格式的数据手册</p>

<h2>常见问题</h2>

<h3>Q: 快捷键不生效怎么办？</h3>
<p>A: 1. 检查快捷键是否被其他程序占用<br>2. 尝试使用其他快捷键组合<br>3. 确保程序窗口已显示</p>

<h3>Q: OCR识别不准确怎么办？</h3>
<p>A: 1. 调整识别置信度（降低置信度可以提高识别率）<br>2. 确保图片清晰度足够<br>3. 尝试使用不同的OCR引擎</p>

<h3>Q: 企业微信自动化不工作怎么办？</h3>
<p>A: 1. 检查企业微信路径是否正确<br>2. 确保企业微信正在运行<br>3. 检查图标识别模板是否正确<br>4. 查看日志了解详细错误信息</p>

<h3>Q: 如何查看日志？</h3>
<p>A: 日志文件存储在：文档/SLinBox/Logs/ 目录下<br>您可以使用任何文本编辑器查看日志文件</p>

<h2>技术支持</h2>
<p>• 邮箱：support@slinbox.com</p>
<p>• 主页：https://github.com/slinbox/slinbox</p>
<p>• 感谢您使用 SLinBox 智能工具箱！</p>
)";
    
    ui->textGuide->setHtml(guideContent);
}

void UserGuideDialog::onOkClicked()
{
    accept();
}
