#include "mainwindow.h"
#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QProgressBar>
#include <QTextEdit>
#include <QPixmap>
#include <QMovie>
#include <QMessageBox>
#include <QFont>
#include <QSpacerItem>
#include <QTimer>
#include <QDir>
#include <QFileInfo>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_centralWidget(nullptr)
    , m_mainLayout(nullptr)
    , m_installer(nullptr)
    , m_loadingMovie(nullptr)
    , m_isUpgradeMode(false)
{
    setupUI();
    
    // 创建安装器实例
    m_installer = new Installer(this);
    
    // 连接信号
    connect(m_installer, &Installer::progressUpdated, this, &MainWindow::onInstallationProgress);
    connect(m_installer, &Installer::installationFinished, this, &MainWindow::onInstallationFinished);
    connect(m_installer, &Installer::errorOccurred, this, &MainWindow::onInstallationError);
    
    // 显示欢迎页面
    showWelcomePage();
}

MainWindow::~MainWindow()
{
    if (m_loadingMovie) {
        delete m_loadingMovie;
    }
}

void MainWindow::setupUI()
{
    setWindowTitle("音触 安装程序");
    setFixedSize(400, 600);  // 竖屏布局
    setWindowFlags(Qt::Window | Qt::WindowCloseButtonHint);
    
    // 扁平化蓝色风格
    setStyleSheet(
        "QMainWindow {"
        "    background-color: #f8f9fa;"
        "    font-family: 'Microsoft YaHei', 'PingFang SC', sans-serif;"
        "}"
        "QLabel {"
        "    color: #333333;"
        "    font-family: 'Microsoft YaHei', 'PingFang SC', sans-serif;"
        "    font-size: 14px;"
        "}"
        "QProgressBar {"
        "    border: none;"
        "    border-radius: 4px;"
        "    text-align: center;"
        "    background-color: #e3f2fd;"
        "    height: 8px;"
        "    font-size: 12px;"
        "    color: #333333;"
        "}"
        "QProgressBar::chunk {"
        "    background-color: #2196F3;"
        "    border-radius: 4px;"
        "}"
        "QLineEdit {"
        "    background-color: #ffffff;"
        "    color: #333333;"
        "    border: 1px solid #e0e0e0;"
        "    border-radius: 4px;"
        "    font-size: 14px;"
        "    padding: 8px 12px;"
        "    min-height: 20px;"
        "}"
        "QLineEdit:focus {"
        "    border-color: #2196F3;"
        "}"
        "QCheckBox {"
        "    color: #333333;"
        "    font-size: 14px;"
        "    spacing: 8px;"
        "}"
    );
    
    m_centralWidget = new QWidget(this);
    setCentralWidget(m_centralWidget);
    
    m_mainLayout = new QVBoxLayout(m_centralWidget);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    
    // 设置页面
    setupWelcomePage();
    setupInstallPage();
    setupFinishPage();
}

void MainWindow::setupWelcomePage()
{
    m_welcomePage = new QWidget();
    m_welcomePage->setStyleSheet(
        "QWidget {"
        "    background-color: #ffffff;"
        "    border-radius: 8px;"
        "}"
    );
    
    QVBoxLayout *layout = new QVBoxLayout(m_welcomePage);
    layout->setSpacing(25);
    layout->setContentsMargins(30, 40, 30, 30);
    
    // 顶部间距
    layout->addSpacing(100);
    
    // 图标容器布局
    QHBoxLayout *iconLayout = new QHBoxLayout();
    iconLayout->addStretch();
    
    // 图标
    m_welcomeIcon = new QLabel();
    m_welcomeIcon->setAlignment(Qt::AlignCenter);
    m_welcomeIcon->setFixedSize(100, 100);
    QPixmap welcomePixmap(":/images/welcome.png");
    if (!welcomePixmap.isNull()) {
        m_welcomeIcon->setPixmap(welcomePixmap.scaled(100, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        m_welcomeIcon->setText("♪");
        m_welcomeIcon->setStyleSheet(
            "QLabel {"
            "    font-size: 64px;"
            "    color: #165DFF;"
            "    background-color: #e3f2fd;"
            "    border-radius: 50px;"
            "    padding: 20px;"
            "    text-align: center;"
            "}"
        );
    }
    
    iconLayout->addWidget(m_welcomeIcon);
    iconLayout->addStretch();
    
    // 标题
    m_welcomeTitle = new QLabel("音触");
    m_welcomeTitle->setAlignment(Qt::AlignCenter);
    m_welcomeTitle->setWordWrap(true);
    m_welcomeTitle->setStyleSheet(
        "QLabel {"
        "    color: #000000;"
        "    font-size: 20px;"
        "    font-weight: bold;"
        "    margin: 5px 0;"
        "    line-height: 1.4;"
        "    padding: 0;"
        "}"
    );
    
    // 安装路径标签
    m_installPathLabel = new QLabel("安装位置:");
    m_installPathLabel->setAlignment(Qt::AlignLeft);
    m_installPathLabel->setStyleSheet(
        "QLabel {"
        "    color: #1D2129;"
        "    font-size: 13px;"
        "    font-weight: 500;"
        "    margin: 12px 0 6px 0;"
        "    padding: 0;"
        "}"
    );
    
    // 路径选择容器
    QHBoxLayout *pathLayout = new QHBoxLayout();
    pathLayout->setSpacing(8);
    
    m_installPathEdit = new QLineEdit();
    m_installPathEdit->setText("C:\\Program Files\\Ausic");
    m_installPathEdit->setReadOnly(true);
    m_installPathEdit->setFixedHeight(36);
    m_installPathEdit->setStyleSheet(
        "QLineEdit {"
        "    background-color: #F7F8FA;"
        "    color: #1D2129;"
        "    border: 1px solid #E5E6EB;"
        "    border-radius: 6px;"
        "    padding: 0 12px;"
        "    font-size: 14px;"
        "    height: 36px;"
        "}"
        "QLineEdit:focus {"
        "    border-color: #165DFF;"
        "    background-color: #ffffff;"
        "}"
    );
    
    m_browseButton = new QPushButton("浏览...");
    m_browseButton->setFixedHeight(36);
    m_browseButton->setFixedWidth(80);
    m_browseButton->setStyleSheet(
        "QPushButton {"
        "    background-color: transparent;"
        "    color: #4E5969;"
        "    border: 1px solid #E5E6EB;"
        "    border-radius: 6px;"
        "    padding: 0;"
        "    font-size: 14px;"
        "    font-weight: 400;"
        "    height: 36px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #F7F8FA;"
        "    border-color: #C9CDD4;"
        "    color: #272E3B;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #E5E6EB;"
        "    border-color: #C9CDD4;"
        "}"
    );
    
    pathLayout->addWidget(m_installPathEdit);
    pathLayout->addWidget(m_browseButton);
    
    connect(m_browseButton, &QPushButton::clicked, this, &MainWindow::browseInstallPath);
    
    // 按钮容器
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(15);
    
    m_installButton = new QPushButton("安装");
    m_installButton->setDefault(true);
    m_installButton->setStyleSheet(
        "QPushButton {"
        "    background-color: #165DFF;"
        "    color: #ffffff;"
        "    border: none;"
        "    border-radius: 8px;"
        "    font-size: 16px;"
        "    font-weight: 500;"
        "    height: 40px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #1348E6;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #0F3ACC;"
        "}"
    );
     
    buttonLayout->addWidget(m_installButton);
    
    // 检测是否已安装
    QString defaultPath = m_installPathEdit->text();
    m_isUpgradeMode = checkExistingInstallation(defaultPath);
    
    // 连接信号
    connect(m_installButton, &QPushButton::clicked, this, &MainWindow::startInstallation);
    connect(m_installPathEdit, &QLineEdit::textChanged, [this](const QString &text) {
        m_isUpgradeMode = checkExistingInstallation(text);
    });
    
    layout->addLayout(iconLayout);
    layout->addWidget(m_welcomeTitle);
    layout->addWidget(m_installPathLabel);
    layout->addLayout(pathLayout);
    layout->addSpacing(30);
    layout->addLayout(buttonLayout);
    layout->addStretch();
    
    // 设置默认焦点到安装按钮
    m_installButton->setFocus();
}

void MainWindow::setupInstallPage()
{
    m_installPage = new QWidget();
    
    QVBoxLayout *layout = new QVBoxLayout(m_installPage);
    layout->setSpacing(25);
    layout->setContentsMargins(30, 40, 30, 30);
    
    // 顶部间距
    layout->addSpacing(30);
    
    // 图标
    m_installIcon = new QLabel();
    m_installIcon->setAlignment(Qt::AlignCenter);
    m_installIcon->setFixedSize(100, 100);
    QPixmap installPixmap(":/images/installing.png");
    if (!installPixmap.isNull()) {
        m_installIcon->setPixmap(installPixmap.scaled(100, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        m_installIcon->setText("♪");
        m_installIcon->setStyleSheet(
            "QLabel {"
            "    font-size: 64px;"
            "    color: #2196F3;"
            "    background-color: #e3f2fd;"
            "    border-radius: 50px;"
            "    padding: 20px;"
            "    margin: 8px 0;"
            "}"
        );
    }
    
    // 标题
    m_installTitle = new QLabel("正在安装音触...");
    m_installTitle->setAlignment(Qt::AlignCenter);
    m_installTitle->setStyleSheet(
        "QLabel {"
        "    color: #2196F3;"
        "    font-size: 24px;"
        "    font-weight: bold;"
        "    margin: 15px 0 10px 0;"
        "}"
    );
    
    // 状态
    m_installStatus = new QLabel("准备开始安装...");
    m_installStatus->setAlignment(Qt::AlignCenter);
    m_installStatus->setStyleSheet(
        "QLabel {"
        "    color: #666666;"
        "    font-size: 14px;"
        "    margin: 10px 0 20px 0;"
        "}"
    );
    
    // 进度条
    m_progressBar = new QProgressBar();
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);
    m_progressBar->setFixedHeight(12);
    m_progressBar->setStyleSheet(
        "QProgressBar {"
        "    border: none;"
        "    border-radius: 6px;"
        "    text-align: center;"
        "    background-color: #e3f2fd;"
        "    font-size: 12px;"
        "    color: #333333;"
        "}"
        "QProgressBar::chunk {"
        "    background-color: #2196F3;"
        "    border-radius: 6px;"
        "}"
    );
    
    layout->addWidget(m_installIcon, 0, Qt::AlignCenter);
    layout->addWidget(m_installTitle);
    layout->addWidget(m_installStatus);
    layout->addSpacing(20);
    layout->addWidget(m_progressBar);
    layout->addStretch();
    layout->addSpacing(30);
}

void MainWindow::setupFinishPage()
{
    m_finishPage = new QWidget();
    m_finishPage->setStyleSheet(
        "QWidget {"
        "    background-color: #ffffff;"
        "}"
    );
    
    QVBoxLayout *layout = new QVBoxLayout(m_finishPage);
    layout->setSpacing(25);
    layout->setContentsMargins(30, 40, 30, 30);
    
    // 顶部间距
    layout->addSpacing(30);
    
    // 图标
    m_finishIcon = new QLabel();
    m_finishIcon->setAlignment(Qt::AlignCenter);
    m_finishIcon->setFixedSize(100, 100);
    m_finishIcon->setStyleSheet(
        "QLabel {"
        "    background-color: #2196F3;"
        "    border-radius: 50px;"
        "    color: white;"
        "    font-size: 48px;"
        "    font-weight: bold;"
        "}"
    );
    
    // 标题
    m_finishTitle = new QLabel();
    m_finishTitle->setAlignment(Qt::AlignCenter);
    m_finishTitle->setWordWrap(true);
    m_finishTitle->setStyleSheet(
        "QLabel {"
        "    color: #2196F3;"
        "    font-size: 24px;"
        "    font-weight: bold;"
        "    margin: 15px 0 10px 0;"
        "}"
    );

    // 复选框
    m_launchCheckBox = new QCheckBox("立即运行 音触");
    m_launchCheckBox->setChecked(true);

    
    // 按钮
    m_finishButton = new QPushButton("完成");
    m_finishButton->setDefault(true);
    m_finishButton->setFixedSize(140, 40);
    m_finishButton->setStyleSheet(
        "QPushButton {"
        "    background-color: #165DFF;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 4px;"
        "    font-size: 14px;"
        "    font-weight: normal;"
        "}"
        "QPushButton:hover {"
        "    background-color: #1976D2;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #0D47A1;"
        "}"
    );
    
    // 连接信号
    connect(m_finishButton, &QPushButton::clicked, [this]() {
        if (m_launchCheckBox->isChecked()) {
            launchApplication(m_installer->getInstallDirectory());
        }
        this->close();
    });
    
    // 布局组装
    layout->addWidget(m_finishIcon, 0, Qt::AlignCenter);
    layout->addWidget(m_finishTitle);
    layout->addSpacing(20);
    layout->addWidget(m_launchCheckBox, 0, Qt::AlignCenter);
    layout->addSpacing(30);
    layout->addWidget(m_finishButton, 0, Qt::AlignCenter);
    layout->addStretch();
}

void MainWindow::showWelcomePage()
{
    // 清除当前布局
    QLayoutItem *item;
    while ((item = m_mainLayout->takeAt(0)) != nullptr) {
        if (item->widget()) {
            item->widget()->setParent(nullptr);
        }
        delete item;
    }
    
    m_mainLayout->addWidget(m_welcomePage);
    
    // 确保焦点在安装按钮上
    m_installButton->setFocus();
}

void MainWindow::showInstallPage()
{
    // 清除当前布局
    QLayoutItem *item;
    while ((item = m_mainLayout->takeAt(0)) != nullptr) {
        if (item->widget()) {
            item->widget()->setParent(nullptr);
        }
        delete item;
    }
    
    m_mainLayout->addWidget(m_installPage);
}

void MainWindow::showFinishPage(bool success)
{
    // 清除当前布局
    QLayoutItem *item;
    while ((item = m_mainLayout->takeAt(0)) != nullptr) {
        if (item->widget()) {
            item->widget()->setParent(nullptr);
        }
        delete item;
    }
    
    // 设置完成页面内容
    if (success) {
        QPixmap successPixmap(":/images/success.png");
        if (!successPixmap.isNull()) {
            m_finishIcon->setPixmap(successPixmap.scaled(100, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        } else {
            m_finishIcon->setText("✓");
        }
        m_finishTitle->setText("安装完成");
    } else {
        m_finishIcon->setText("✗");
        m_finishIcon->setStyleSheet(
            "QLabel {"
            "    background-color: #f44336;"
            "    border-radius: 50px;"
            "    color: white;"
            "    font-size: 48px;"
            "    font-weight: bold;"
            "}"
        );
        m_finishTitle->setText("安装失败");
        m_finishTitle->setStyleSheet(
             "QLabel {"
             "    color: #f44336;"
             "    font-size: 24px;"
             "    font-weight: bold;"
             "    margin: 15px 0 10px 0;"
             "}"
        );
        m_finishTitle->setText("安装失败");
        m_finishTitle->setStyleSheet(
            "QLabel {"
            "    color: #F53F3F;"
            "    font-size: 28px;"
            "    font-weight: 600;"
            "    margin: 16px 0 8px 0;"
            "    line-height: 1.2;"
            "}"
        );
    }
    
    m_mainLayout->addWidget(m_finishPage);
}

void MainWindow::startInstallation()
{
    showInstallPage();
    
    // 延迟启动安装，让界面有时间更新
    QTimer::singleShot(100, [this]() {
        QString installPath = m_installPathEdit->text();
        
        // 删除目标文件夹中的旧程序文件
        deleteOldInstallation(installPath);
    
    m_installer->setInstallPath(installPath);
        m_installer->startInstallation();
    });
}

void MainWindow::onInstallationProgress(int percentage, const QString &message)
{
    m_progressBar->setValue(percentage);
    m_installStatus->setText(message);
}

void MainWindow::onInstallationFinished(bool success, const QString &message)
{
    if (success) {
        // 安装成功时自动创建桌面快捷方式
        createDesktopShortcut(m_installer->getInstallDirectory());
    }
    
    // 延迟显示完成页面
    QTimer::singleShot(1000, [this, success]() {
        showFinishPage(success);
    });
}

void MainWindow::onInstallationError(const QString &error)
{

    
    m_installStatus->setText("安装失败");
    
    QMessageBox::critical(this, "安装错误", error);
    
    // 显示失败页面
    QTimer::singleShot(1000, [this]() {
        showFinishPage(false);
    });
}

void MainWindow::browseInstallPath()
{
    QString currentPath = m_installPathEdit->text();
    QString selectedPath = QFileDialog::getExistingDirectory(
        this,
        "选择安装目录",
        currentPath,
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
    );
    
    if (!selectedPath.isEmpty()) {
        // 确保路径以 \Ausic 结尾
        if (!selectedPath.endsWith("\\Ausic")) {
            selectedPath += "\\Ausic";
        }
        m_installPathEdit->setText(selectedPath);
    }
}

bool MainWindow::checkExistingInstallation(const QString &path)
{
    QDir installDir(path);
    if (!installDir.exists()) {
        return false;
    }
    
    // 检查是否存在 Ausic.exe 或其他标识文件
    QFileInfo exeFile(installDir.absoluteFilePath("Ausic.exe"));
    return exeFile.exists();
}

void MainWindow::deleteOldInstallation(const QString &path)
{
    QDir installDir(path);
    if (!installDir.exists()) {
        return;
    }
    
    try {
        // 删除目录中的所有文件和子目录
        QStringList entries = installDir.entryList(QDir::NoDotAndDotDot | QDir::AllEntries);
        for (const QString &entry : entries) {
            QString fullPath = installDir.absoluteFilePath(entry);
            QFileInfo fileInfo(fullPath);
            
            if (fileInfo.isDir()) {
                QDir subDir(fullPath);
                if (!subDir.removeRecursively()) {
        
                }
            } else {
                if (!QFile::remove(fullPath)) {
    
                }
            }
        }
    } catch (...) {
        QMessageBox::warning(this, "升级提示", "删除旧文件时遇到问题，但升级将继续进行。");
    }
}

void MainWindow::createDesktopShortcut(const QString &installPath)
{
    QString desktopPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    QString shortcutPath = desktopPath + "/音触.lnk";
    QString exePath = QDir(installPath).absoluteFilePath("Ausic.exe");
    

    
    // 使用Windows API创建快捷方式
#ifdef Q_OS_WIN
    QString command = QString("powershell -Command \"$WshShell = New-Object -comObject WScript.Shell; $Shortcut = $WshShell.CreateShortcut('%1'); $Shortcut.TargetPath = '%2'; $Shortcut.WorkingDirectory = '%3'; $Shortcut.Save()\"")
                     .arg(shortcutPath)
                     .arg(exePath)
                     .arg(installPath);
    
    QProcess::execute(command);
#endif
}

void MainWindow::launchApplication(const QString &installPath)
{
    QString exePath = QDir(installPath).absoluteFilePath("Ausic.exe");
    

    
    if (QFile::exists(exePath)) {
        QProcess::startDetached(exePath, QStringList(), installPath);
    } else {
        QMessageBox::warning(this, "错误", "无法找到应用程序文件：" + exePath);
    }
}