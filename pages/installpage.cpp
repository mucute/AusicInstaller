#include "installpage.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFileDialog>
#include <QStyle>
#include <QStandardPaths>
#include <QStorageInfo>
#include <QFile>
#include <QDir>
#include <QMessageBox>
#include <QtCore/private/qzipreader_p.h>
#include <QCoreApplication>
#include <QDebug>
#include <windows.h>
#include <shlobj.h>
#include <objbase.h>

InstallPage::InstallPage(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(30, 30, 30, 50);  // 增加底部边距到50
    
    // 标题区域
    QWidget *titleWidget = new QWidget(this);
    QVBoxLayout *titleLayout = new QVBoxLayout(titleWidget);
    titleLayout->setSpacing(0);
    titleLayout->setContentsMargins(0, 0, 0, 20);  // 添加底部边距
    titleLayout->setAlignment(Qt::AlignTop);
    
    QLabel *titleLabel = new QLabel(tr("安装 Ausic"), this);
    titleLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #303133; padding: 0;");
    
    titleLayout->addWidget(titleLabel, 0, Qt::AlignBottom);
    
    mainLayout->addWidget(titleWidget);
    
    // 图标和描述区域
    QWidget *logoWidget = new QWidget(this);
    QVBoxLayout *logoLayout = new QVBoxLayout(logoWidget);
    logoLayout->setSpacing(8);  // 设置图标和文字之间的间距
    logoLayout->setContentsMargins(0, 0, 0, 0);
    
    QLabel *iconLabel = new QLabel(this);
    iconLabel->setPixmap(QPixmap(":/images/welcome.png").scaled(140, 140, Qt::KeepAspectRatio, Qt::SmoothTransformation));  // 缩小到 140x140
    iconLabel->setAlignment(Qt::AlignCenter);
    
    QLabel *descLabel = new QLabel(tr("自动化弹琴助手"), this);
    descLabel->setStyleSheet("color: #606266; font-size: 16px;");  // 设置描述文字样式
    descLabel->setAlignment(Qt::AlignCenter);
    
    logoLayout->addWidget(iconLabel);
    logoLayout->addWidget(descLabel);
    
    mainLayout->addWidget(logoWidget);
    
    // 安装信息区域
    QWidget *infoWidget = new QWidget(this);
    QVBoxLayout *infoLayout = new QVBoxLayout(infoWidget);
    infoLayout->setSpacing(12);
    
    // 安装位置
    QLabel *locationLabel = new QLabel(tr("安装位置"), this);
    locationLabel->setStyleSheet("color: #606266; font-weight: bold;");
    
    QHBoxLayout *locationLayout = new QHBoxLayout;
    QString defaultPath = "C:/Program Files/Ausic";  // 修改默认安装路径
    locationEdit = new QLineEdit(defaultPath, this);
    locationEdit->setStyleSheet(R"(
        QLineEdit {
            padding: 8px 12px;
            border: 1px solid #DCDFE6;
            border-radius: 4px;
            background-color: #F5F7FA;
            color: #606266;
            selection-background-color: #E4E7ED;
        }
        QLineEdit:hover {
            border-color: #C0C4CC;
        }
    )");
    locationEdit->setFocusPolicy(Qt::ClickFocus);  // 只在点击时获取焦点
    
    QPushButton *browseButton = new QPushButton(tr("浏览"), this);
    browseButton->setStyleSheet(R"(
        QPushButton {
            padding: 8px 16px;
            border: 1px solid #DCDFE6;
            border-radius: 4px;
            background-color: white;
            color: #606266;
        }
        QPushButton:hover {
            background-color: #F5F7FA;
            color: #409EFF;
            border-color: #409EFF;
        }
    )");
    
    locationLayout->addWidget(locationEdit);
    locationLayout->addWidget(browseButton);
    
    // 空间信息
    spaceLabel = new QLabel(this);
    spaceLabel->setStyleSheet("color: #909399;");
    checkDiskSpace(locationEdit->text());
    
    infoLayout->addWidget(locationLabel);
    infoLayout->addLayout(locationLayout);
    infoLayout->addWidget(spaceLabel);
    
    mainLayout->addWidget(infoWidget);
    
    // 安装按钮
    installButton = new QPushButton(tr("立即安装"), this);
    installButton->setFixedHeight(40);
    installButton->setStyleSheet(R"(
        QPushButton {
            background-color: #409EFF;
            color: white;
            border: none;
            border-radius: 4px;
            font-size: 14px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #66B1FF;
        }
        QPushButton:pressed {
            background-color: #3A8EE6;
        }
        QPushButton:disabled {
            background-color: #A0CFFF;
        }
    )");
    
    mainLayout->addWidget(installButton);
    
    // 连接信号槽
    connect(browseButton, &QPushButton::clicked, this, &InstallPage::browseLocation);
    connect(locationEdit, &QLineEdit::textChanged, this, &InstallPage::updateInstallButton);
    connect(installButton, &QPushButton::clicked, this, &InstallPage::performInstallation);
    
    setStyleSheet("background-color: white; border-radius: 8px;");
}

void InstallPage::browseLocation()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("选择安装目录"),
                                                  locationEdit->text(),
                                                  QFileDialog::ShowDirsOnly
                                                  | QFileDialog::DontResolveSymlinks);
    if (!dir.isEmpty()) {
        locationEdit->setText(dir);
    }
}

void InstallPage::checkDiskSpace(const QString &path)
{
    QStorageInfo storage(path);
    if (storage.isValid()) {
        qint64 freeSpace = storage.bytesAvailable() / (1024 * 1024 * 1024); // 转换为 GB
        spaceLabel->setText(tr("可用空间: %1 GB").arg(freeSpace));
    }
}

void InstallPage::updateInstallButton()
{
    QDir dir(locationEdit->text());
    bool isValid = !locationEdit->text().isEmpty() && dir.exists();
    installButton->setEnabled(isValid);
    
    checkDiskSpace(locationEdit->text());
}

void InstallPage::performInstallation()
{
    QString installPath = locationEdit->text();
    QDir dir(installPath);
    
    // 创建安装目录
    if (!dir.exists()) {
        if (!dir.mkpath(".")) {
            QMessageBox::critical(this, tr("错误"), tr("无法创建安装目录，请检查权限"));
            return;
        }
    }
    
    // 获取应用资源路径
    QString zipPath = ":/assets/app.zip";
    QFile zipFile(zipPath);
    
    qDebug() << "Checking resource file:" << zipPath;
    qDebug() << "Resource exists:" << zipFile.exists();
    
    if (!zipFile.exists()) {
        QMessageBox::critical(this, tr("错误"), 
            tr("安装文件不存在于资源系统中"));
        return;
    }
    
    // 使用应用程序目录作为临时文件位置
    QString tempDir = QCoreApplication::applicationDirPath() + "/temp";
    QDir().mkpath(tempDir);  // 创建临时目录
    
    QString tempPath = tempDir + "/app.zip";
    qDebug() << "Temp file path:" << tempPath;
    
    // 如果临时文件已存在，先删除
    if (QFile::exists(tempPath)) {
        if (!QFile::remove(tempPath)) {
            QMessageBox::critical(this, tr("错误"), 
                tr("无法删除已存在的临时文件\n路径: %1").arg(tempPath));
            return;
        }
    }
    
    // 尝试打开源文件
    if (!zipFile.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, tr("错误"), 
            tr("无法打开资源文件\n错误: %1").arg(zipFile.errorString()));
        return;
    }
    
    // 尝试复制到临时文件
    QFile tempFile(tempPath);
    if (!tempFile.open(QIODevice::WriteOnly)) {
        zipFile.close();
        QMessageBox::critical(this, tr("错误"), 
            tr("无法创建临时文件\n路径: %1\n错误: %2").arg(tempPath, tempFile.errorString()));
        return;
    }
    
    // 复制文件内容
    QByteArray data = zipFile.readAll();
    if (tempFile.write(data) != data.size()) {
        zipFile.close();
        tempFile.close();
        QFile::remove(tempPath);
        QMessageBox::critical(this, tr("错误"), 
            tr("复制文件内容失败\n错误: %1").arg(tempFile.errorString()));
        return;
    }
    
    zipFile.close();
    tempFile.close();
    
    // 开始安装
    emit startInstall();
    
    // 使用临时文件路径进行解压
    if (!extractZip(tempPath, installPath)) {
        QFile::remove(tempPath);
        QDir(tempDir).removeRecursively();  // 清理临时目录
        QMessageBox::critical(this, tr("错误"), 
            tr("安装失败，无法解压文件\n请确保您有足够的权限访问安装目录: %1").arg(installPath));
        return;
    }
    
    // 解压完成后创建快捷方式
    QString exePath = installPath + "/org.example.project.exe";
    QString desktopPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    QString shortcutPath = desktopPath + "/Ausic.lnk";
    
    if (!createShortcut(exePath, shortcutPath)) {
        qDebug() << "Failed to create desktop shortcut";
    } else {
        qDebug() << "Desktop shortcut created successfully";
    }
    
    // 清理临时文件和目录
    QFile::remove(tempPath);
    QDir(tempDir).removeRecursively();
}

bool InstallPage::extractZip(const QString &zipPath, const QString &targetPath)
{
    qDebug() << "Opening zip file:" << zipPath;
    QZipReader zip(zipPath);
    if (!zip.exists()) {
        qDebug() << "Zip file does not exist or is invalid";
        return false;
    }
    
    const QVector<QZipReader::FileInfo> allFiles = zip.fileInfoList();
    qDebug() << "Total files in zip:" << allFiles.size();
    
    if (allFiles.isEmpty()) {
        qDebug() << "Zip file is empty";
        return false;
    }
    
    int total = allFiles.size();
    int current = 0;
    
    for (const QZipReader::FileInfo &info : allFiles) {
        const QString filePath = targetPath + "/" + info.filePath;
        QFileInfo fileInfo(filePath);
        
        qDebug() << "Processing:" << info.filePath;
        qDebug() << "Target path:" << filePath;
        
        // 创建目录结构
        if (!fileInfo.dir().exists()) {
            qDebug() << "Creating directory:" << fileInfo.dir().path();
            if (!fileInfo.dir().mkpath(".")) {
                qDebug() << "Failed to create directory";
                return false;
            }
        }
        
        if (!info.isDir) {
            QFile file(filePath);
            if (file.open(QIODevice::WriteOnly)) {
                QByteArray data = zip.fileData(info.filePath);
                qDebug() << "File size:" << data.size() << "bytes";
                
                if (file.write(data) != data.size()) {
                    qDebug() << "Failed to write file:" << file.errorString();
                    file.close();
                    return false;
                }
                file.close();
            } else {
                qDebug() << "Failed to open file for writing:" << file.errorString();
                return false;
            }
        }
        
        current++;
        int progress = (current * 100) / total;
        emit installProgress(progress, tr("正在复制: %1").arg(info.filePath));
    }
    
    zip.close();
    return true;
}

bool InstallPage::createShortcut(const QString &exePath, const QString &shortcutPath)
{
    HRESULT hres;
    IShellLink* psl;
    
    // 初始化 COM
    hres = CoInitialize(NULL);
    if (!SUCCEEDED(hres)) {
        return false;
    }
    
    // 创建 IShellLink 对象
    hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
                          IID_IShellLink, (void**)&psl);
    
    if (SUCCEEDED(hres)) {
        IPersistFile* ppf;
        
        // 设置目标路径
        psl->SetPath(exePath.toStdWString().c_str());
        psl->SetWorkingDirectory(QFileInfo(exePath).path().toStdWString().c_str());
        
        // 获取 IPersistFile 接口
        hres = psl->QueryInterface(IID_IPersistFile, (void**)&ppf);
        if (SUCCEEDED(hres)) {
            // 保存快捷方式
            hres = ppf->Save(shortcutPath.toStdWString().c_str(), TRUE);
            ppf->Release();
        }
        psl->Release();
    }
    
    CoUninitialize();
    return SUCCEEDED(hres);
} 