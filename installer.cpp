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
    
    // 新方法：查找魔术签名和元数据
    const QByteArray MAGIC_SIGNATURE = QByteArray("AUSIC_ZIP_INFO");
    const qint64 METADATA_SIZE = 14 + 8 + 8 + 4; // magic + offset + size + metadata_size
    
    if (fileSize < METADATA_SIZE) {
        file.close();
        return false;
    }
    
    // 读取文件末尾的元数据
    file.seek(fileSize - METADATA_SIZE);
    QByteArray metadataBlock = file.read(METADATA_SIZE);
    
    if (metadataBlock.size() != METADATA_SIZE) {
        file.close();
        return false;
    }
    
    // 检查魔术签名
    QByteArray magic = metadataBlock.left(14);
    if (magic == MAGIC_SIGNATURE) {
        // 解析元数据
        QDataStream stream(metadataBlock.mid(14));
        stream.setByteOrder(QDataStream::LittleEndian);
        
        quint64 zipOffset, zipSize;
        quint32 metadataSize;
        
        stream >> zipOffset >> zipSize >> metadataSize;
        
        // 验证数据的合理性
        if (zipOffset < fileSize && zipSize > 0 && 
            zipOffset + zipSize <= fileSize - METADATA_SIZE) {
            
            // 验证ZIP文件头
            file.seek(zipOffset);
            QByteArray header = file.read(4);
            
            if (header == ZIP_SIGNATURE) {
                // 进一步验证ZIP文件的完整性
                // 检查ZIP结束记录是否在预期位置
                qint64 expectedEndPos = zipOffset + zipSize - 22;
                if (expectedEndPos >= 0 && expectedEndPos < fileSize - 22) {
                    file.seek(expectedEndPos);
                    QByteArray endSig = file.read(4);
                    if (endSig == ZIP_END_SIGNATURE) {
                        archiveOffset = zipOffset;
                        archiveSize = zipSize;
                        file.close();
                        return true;
                    }
                }
            }
        }
    }
    
    // 备用方法：使用旧的ZIP结束记录查找方法
    // 首先从文件末尾查找ZIP结束记录
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
            qint64 zipStartPos = zipEndRecordPos - centralDirOffset - centralDirSize;
            
            // ZIP文件结束位置（包含结束记录和注释）
            qint64 zipEndPos = zipEndRecordPos + 22 + commentLength;
            
            // 验证ZIP文件头
            file.seek(zipStartPos);
            QByteArray header = file.read(4);
            
            if (header == ZIP_SIGNATURE && zipStartPos >= 0 && zipStartPos < fileSize) {
                // 验证计算出的ZIP大小是否合理
                qint64 calculatedSize = zipEndPos - zipStartPos;
                if (calculatedSize > 0 && calculatedSize < fileSize && 
                    zipEndPos <= fileSize) {
                    archiveOffset = zipStartPos;
                    archiveSize = calculatedSize;
                    file.close();
                    return true;
                }
            }
        }
    }
    
    // 最后的备用方法：从文件末尾向前搜索ZIP文件头
    const qint64 maxSearchSize = qMin(fileSize, 200 * 1024 * 1024LL);
    
    for (qint64 pos = fileSize - 4; pos >= fileSize - maxSearchSize; pos--) {
        file.seek(pos);
        QByteArray header = file.read(4);
        
        if (header == ZIP_SIGNATURE) {
            qint64 potentialSize = fileSize - pos;
            // 验证这个位置的ZIP文件是否有有效的结束记录
            if (potentialSize >= 22) {
                file.seek(fileSize - 22);
                QByteArray endSig = file.read(4);
                if (endSig == ZIP_END_SIGNATURE) {
                    archiveOffset = pos;
                    archiveSize = potentialSize;
                    file.close();
                    return true;
                }
            }
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
    
    // 验证文件大小和偏移量的合理性
    qint64 fileSize = sourceFile.size();
    if (offset < 0 || size <= 0 || offset + size > fileSize) {
        sourceFile.close();
        return false;
    }
    
    if (!sourceFile.seek(offset)) {
        sourceFile.close();
        return false;
    }
    
    // 分块读取大文件，避免内存问题
    const qint64 CHUNK_SIZE = 1024 * 1024; // 1MB chunks
    QFile outputFile(outputPath);
    if (!outputFile.open(QIODevice::WriteOnly)) {
        sourceFile.close();
        return false;
    }
    
    qint64 totalBytesRead = 0;
    qint64 totalBytesWritten = 0;
    
    while (totalBytesRead < size) {
        qint64 bytesToRead = qMin(CHUNK_SIZE, size - totalBytesRead);
        QByteArray chunk = sourceFile.read(bytesToRead);
        
        if (chunk.isEmpty() && bytesToRead > 0) {
            // 读取失败
            sourceFile.close();
            outputFile.close();
            QFile::remove(outputPath);
            return false;
        }
        
        qint64 bytesWritten = outputFile.write(chunk);
        if (bytesWritten != chunk.size()) {
            // 写入失败
            sourceFile.close();
            outputFile.close();
            QFile::remove(outputPath);
            return false;
        }
        
        totalBytesRead += chunk.size();
        totalBytesWritten += bytesWritten;
    }
    
    sourceFile.close();
    
    // 强制刷新到磁盘
    outputFile.flush();
    outputFile.close();
    
    // 验证总字节数
    if (totalBytesWritten != size) {
        QFile::remove(outputPath);
        return false;
    }
    
    // 验证提取的文件
    if (!isValidZipFile(outputPath)) {
        QFile::remove(outputPath);
        return false;
    }
    
    // 额外验证：检查文件大小
    QFileInfo outputInfo(outputPath);
    if (outputInfo.size() != size) {
        QFile::remove(outputPath);
        return false;
    }
    
    return true;
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
            
            // 详细验证文件数据
            if (fileInfo.size > 0 && fileData.isEmpty()) {
                // 文件应该有内容但读取为空，这是一个严重错误
                return false;
            }
            
            // 验证读取的数据大小是否与预期一致
            if (fileData.size() != fileInfo.size) {
                // 数据大小不匹配
                return false;
            }
            
            QFile outputFile(fullPath);
            if (!outputFile.open(QIODevice::WriteOnly)) {
                return false;
            }
            
            qint64 bytesWritten = 0;
            if (fileInfo.size > 0) {
                // 对于非空文件，写入数据
                bytesWritten = outputFile.write(fileData);
                if (bytesWritten != fileData.size()) {
                    outputFile.close();
                    QFile::remove(fullPath);
                    return false;
                }
            }
            
            // 强制刷新缓冲区到磁盘
            if (!outputFile.flush()) {
                outputFile.close();
                QFile::remove(fullPath);
                return false;
            }
            
            outputFile.close();
            
            // 验证文件是否正确创建
            QFileInfo createdFileInfo(fullPath);
            if (!createdFileInfo.exists()) {
                return false;
            }
            
            // 验证文件大小
            if (createdFileInfo.size() != fileInfo.size) {
                QFile::remove(fullPath);
                return false;
            }
            
            extractedCount++;
            
            // 设置文件权限为可读写
            outputFile.setPermissions(QFile::ReadOwner | QFile::WriteOwner | 
                                    QFile::ReadGroup | QFile::ReadOther);
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
    if (fileSize < 22) { // ZIP文件最小大小（包含中央目录结束记录）
        file.close();
        return false;
    }
    
    // 检查文件头是否为ZIP签名
    QByteArray header = file.read(4);
    
    if (header != ZIP_SIGNATURE) {
        file.close();
        return false;
    }
    
    // 验证ZIP结束记录
    file.seek(fileSize - 22);
    QByteArray endRecord = file.read(22);
    file.close();
    
    if (endRecord.size() != 22 || endRecord.left(4) != ZIP_END_SIGNATURE) {
        return false;
    }
    
    // 解析ZIP结束记录
    QDataStream stream(endRecord);
    stream.setByteOrder(QDataStream::LittleEndian);
    
    quint32 signature;
    quint16 diskNumber, startDisk, entriesOnDisk, totalEntries;
    quint32 centralDirSize, centralDirOffset;
    quint16 commentLength;
    
    stream >> signature >> diskNumber >> startDisk >> entriesOnDisk 
           >> totalEntries >> centralDirSize >> centralDirOffset >> commentLength;
    
    // 验证中央目录的位置和大小是否合理
    if (centralDirOffset >= fileSize || 
        centralDirSize == 0 || 
        centralDirOffset + centralDirSize > fileSize - 22 - commentLength) {
        return false;
    }
    
    // 使用QZipReader进一步验证文件完整性
    QZipReader zipReader(filePath);
    if (!zipReader.isReadable()) {
        return false;
    }
    
    // 尝试获取文件列表并验证条目数量
    QList<QZipReader::FileInfo> fileInfos = zipReader.fileInfoList();
    
    // 验证文件条目数量是否与ZIP结束记录一致
    if (fileInfos.size() != totalEntries) {
        zipReader.close();
        return false;
    }
    
    // 验证每个文件的完整性（检查前几个文件）
    int checkCount = qMin(5, fileInfos.size()); // 只检查前5个文件避免性能问题
    for (int i = 0; i < checkCount; i++) {
        const QZipReader::FileInfo &info = fileInfos[i];
        if (!info.isDir && info.size > 0) {
            QByteArray data = zipReader.fileData(info.filePath);
            // 如果文件应该有内容但读取为空，或大小不匹配，说明ZIP损坏
            if (data.isEmpty() || data.size() != info.size) {
                zipReader.close();
                return false;
            }
        }
    }
    
    zipReader.close();
    return true;
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