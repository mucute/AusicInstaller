#include "installer.h"
#include <QApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>
#include <QProcess>

#include <QThread>
#include <QTimer>
#include <QDataStream>
#include <QIODevice>
#include <QtCore/private/qzipreader_p.h>

// ZIP文件签名
const QByteArray Installer::ZIP_SIGNATURE = QByteArray("PK\x03\x04");
const QByteArray Installer::ZIP_END_SIGNATURE = QByteArray("PK\x05\x06");

Installer::Installer(QObject *parent)
    : QObject(parent)
    , m_progressTimer(new QTimer(this))
    , m_currentProgress(0)
{

    m_progressTimer->setSingleShot(true);
}

Installer::~Installer()
{
    cleanupTempFiles();
}

void Installer::startInstallation()
{

    
    // 在单独的线程中执行安装
    QTimer::singleShot(100, this, &Installer::performInstallation);
}

void Installer::setInstallPath(const QString &path)
{
    m_installPath = path;

}

void Installer::performInstallation()
{
    try {
        updateProgress(0, "开始安装过程...");
        
        // 步骤1: 从exe中提取压缩包
        updateProgress(10, "正在从安装程序中提取文件...");
        if (!extractEmbeddedArchive()) {
            emit errorOccurred("无法从安装程序中提取压缩包");
            return;
        }
        
        updateProgress(30, "压缩包提取完成");
        
        // 步骤2: 创建安装目录
        updateProgress(40, "正在创建安装目录...");
        QString targetPath = getInstallDirectory();
        if (!createDirectory(targetPath)) {
            emit errorOccurred(QString("无法创建安装目录: %1").arg(targetPath));
            return;
        }
        
        updateProgress(50, "安装目录创建完成");
        
        // 步骤3: 解压文件到安装目录
        updateProgress(60, "正在解压文件...");
        if (!extractArchiveToDirectory(m_tempArchivePath, targetPath)) {
            emit errorOccurred("解压文件失败");
            return;
        }
        
        updateProgress(90, "文件解压完成");
        
        // 步骤4: 清理临时文件
        updateProgress(95, "正在清理临时文件...");
        cleanupTempFiles();
        
        updateProgress(100, "安装完成");
        
        emit installationFinished(true, "Ausic 安装成功完成!");
        
    } catch (const std::exception &e) {
        emit errorOccurred(QString("安装过程中发生异常: %1").arg(e.what()));
    } catch (...) {
        emit errorOccurred("安装过程中发生未知错误");
    }
}

bool Installer::extractEmbeddedArchive()
{

    
    QString exePath = getCurrentExecutablePath();
    if (exePath.isEmpty()) {
        return false;
    }
    

    
    qint64 archiveOffset = 0;
    qint64 archiveSize = 0;
    
    if (!findArchiveInExecutable(exePath, archiveOffset, archiveSize)) {
        return false;
    }
    

    
    // 创建临时目录
    QString tempDir = getTempDirectory();
    if (!createDirectory(tempDir)) {
        return false;
    }

    
    m_tempArchivePath = tempDir + "/ausic_temp.zip";

    
    // 删除已存在的临时文件
    if (QFile::exists(m_tempArchivePath)) {
        QFile::remove(m_tempArchivePath);
    }
    
    if (!copyArchiveFromExecutable(exePath, archiveOffset, archiveSize, m_tempArchivePath)) {
        return false;
    }
    
    return true;
}

bool Installer::findArchiveInExecutable(const QString &exePath, qint64 &archiveOffset, qint64 &archiveSize)
{
    QFile file(exePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }
    
    qint64 fileSize = file.size();

    
    // 首先从文件末尾查找ZIP结束记录，这是最可靠的方法
    // ZIP结束记录的最小大小是22字节，最大是65557字节（包含注释）
    const qint64 maxEndRecordSize = 65557;
    qint64 searchStart = qMax(0LL, fileSize - maxEndRecordSize);
    
    file.seek(searchStart);
    QByteArray endData = file.read(fileSize - searchStart);
    
    // 从后往前搜索ZIP结束记录
    for (int i = endData.size() - 22; i >= 0; i--) {
        if (endData.mid(i, 4) == ZIP_END_SIGNATURE) {

            
            // 读取ZIP结束记录中的信息
            QDataStream stream(endData.mid(i));
            stream.setByteOrder(QDataStream::LittleEndian);
            
            quint32 signature;
            quint16 diskNumber, startDisk, entriesOnDisk, totalEntries;
            quint32 centralDirSize, centralDirOffset;
            quint16 commentLength;
            
            stream >> signature >> diskNumber >> startDisk >> entriesOnDisk 
                   >> totalEntries >> centralDirSize >> centralDirOffset >> commentLength;
            

            
            // ZIP结束记录的绝对位置
            qint64 zipEndRecordPos = searchStart + i;
            
            // 根据中央目录偏移计算ZIP文件开始位置
            // 中央目录偏移是相对于ZIP文件开始的
            qint64 zipStartPos = zipEndRecordPos - centralDirOffset - centralDirSize;
            
            // ZIP文件结束位置（包含结束记录和注释）
            qint64 zipEndPos = zipEndRecordPos + 22 + commentLength;
            
            // 验证ZIP文件头
            file.seek(zipStartPos);
            QByteArray header = file.read(4);
            
            if (header == ZIP_SIGNATURE && zipStartPos >= 0 && zipStartPos < fileSize) {
                archiveOffset = zipStartPos;
                archiveSize = zipEndPos - zipStartPos;

                file.close();
                return true;
            } else {

            }
        }
    }
    

    
    // 备用方法：从文件末尾向前搜索ZIP文件头
    const qint64 maxSearchSize = qMin(fileSize, 200 * 1024 * 1024LL); // 最多搜索200MB
    
    for (qint64 pos = fileSize - 4; pos >= fileSize - maxSearchSize; pos--) {
        file.seek(pos);
        QByteArray header = file.read(4);
        
        if (header == ZIP_SIGNATURE) {
            archiveOffset = pos;
            archiveSize = fileSize - pos;
            file.close();
            return true;
        }
    }
    
    file.close();
    return false;
}

bool Installer::copyArchiveFromExecutable(const QString &exePath, qint64 offset, qint64 size, const QString &outputPath)
{

    
    QFile sourceFile(exePath);
    if (!sourceFile.open(QIODevice::ReadOnly)) {
        return false;
    }
    
    if (!sourceFile.seek(offset)) {
        sourceFile.close();
        return false;
    }
    
    QFile outputFile(outputPath);
    if (!outputFile.open(QIODevice::WriteOnly)) {
        sourceFile.close();
        return false;
    }
    
    // 直接读取所有数据
    QByteArray archiveData = sourceFile.read(size);
    sourceFile.close();
    
    if (archiveData.size() != size) {
        outputFile.close();
        return false;
    }
    
    qint64 bytesWritten = outputFile.write(archiveData);
    outputFile.close();
    
    if (bytesWritten != size) {
        return false;
    }
    

    
    // 验证提取的文件
    return isValidZipFile(outputPath);
}

bool Installer::extractArchiveToDirectory(const QString &archivePath, const QString &targetDir)
{

    
    // 检查源文件是否存在
    if (!QFile::exists(archivePath)) {
        return false;
    }
    
    // 确保目标目录存在
    if (!createDirectory(targetDir)) {
        return false;
    }
    
    // 使用Qt内置的ZIP解压功能
    QZipReader zipReader(archivePath);
    
    if (!zipReader.isReadable()) {
        return false;
    }
    

    
    // 获取ZIP文件中的所有条目
    QList<QZipReader::FileInfo> fileInfos = zipReader.fileInfoList();

    
    int extractedCount = 0;
    
    // 逐个提取文件
    for (const QZipReader::FileInfo &fileInfo : fileInfos) {
        QString fileName = fileInfo.filePath;
        QString fullPath = QDir(targetDir).absoluteFilePath(fileName);
        

        
        if (fileInfo.isDir) {
            // 创建目录
            QDir().mkpath(fullPath);

        } else {
            // 创建文件的父目录
            QFileInfo fileInfoObj(fullPath);
            QDir().mkpath(fileInfoObj.absolutePath());
            
            // 提取文件内容
            QByteArray fileData = zipReader.fileData(fileName);
            
            QFile outputFile(fullPath);
            if (outputFile.open(QIODevice::WriteOnly)) {
                qint64 bytesWritten = outputFile.write(fileData);
                outputFile.close();
                
                if (bytesWritten == fileData.size()) {
                    extractedCount++;
                } else {
                    return false;
                }
            } else {
                return false;
            }
        }
    }
    
    zipReader.close();
    
    // 验证解压结果
    QDir targetDirectory(targetDir);
    QStringList entries = targetDirectory.entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
    
    if (entries.isEmpty()) {
        return false;
    }
    
    return true;
}

QString Installer::getCurrentExecutablePath()
{
    return QApplication::applicationFilePath();
}

QString Installer::getTempDirectory()
{
    QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    return tempDir + "/AusicInstaller";
}

QString Installer::getInstallDirectory()
{
    // 如果用户设置了安装路径，使用用户设置的路径，否则使用默认路径
    if (!m_installPath.isEmpty()) {
        return m_installPath;
    }
    // 默认安装到当前目录
    return QDir::currentPath() + "/Ausic";
}

bool Installer::createDirectory(const QString &path)
{
    QDir dir;
    return dir.mkpath(path);
}

bool Installer::isValidZipFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }
    
    qint64 fileSize = file.size();

    
    if (fileSize < 4) {
        file.close();
        return false;
    }
    
    // 只检查文件头是否为ZIP签名
    QByteArray header = file.read(4);
    
    file.close();
    
    return (header == ZIP_SIGNATURE);
}

void Installer::cleanupTempFiles()
{
    if (!m_tempArchivePath.isEmpty()) {
        QFile::remove(m_tempArchivePath);
        m_tempArchivePath.clear();
    }
    
    // 清理临时目录
    QString tempDir = getTempDirectory();
    QDir dir(tempDir);
    if (dir.exists()) {
        dir.removeRecursively();
    }
}

void Installer::updateProgress(int percentage, const QString &message)
{
    m_currentProgress = percentage;
    emit progressUpdated(percentage, message);
    
    // 给UI时间更新
    QApplication::processEvents();
    QThread::msleep(100);
}