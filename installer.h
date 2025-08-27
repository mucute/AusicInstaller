#ifndef INSTALLER_H
#define INSTALLER_H

#include <QObject>
#include <QThread>
#include <QFile>
#include <QDir>
#include <QTimer>
#include <QProcess>

class Installer : public QObject
{
    Q_OBJECT

public:
    explicit Installer(QObject *parent = nullptr);
    ~Installer();
    
    void startInstallation();
    void setInstallPath(const QString &path);
    QString getInstallDirectory();
    
signals:
    void progressUpdated(int percentage, const QString &message);
    void installationFinished(bool success, const QString &message);
    void errorOccurred(const QString &error);
    
private slots:
    void performInstallation();
    
private:
    // 核心功能函数
    bool extractEmbeddedArchive();
    bool extractArchiveToDirectory(const QString &archivePath, const QString &targetDir);
    bool findArchiveInExecutable(const QString &exePath, qint64 &archiveOffset, qint64 &archiveSize);
    bool copyArchiveFromExecutable(const QString &exePath, qint64 offset, qint64 size, const QString &outputPath);
    
    // 辅助函数
    QString getCurrentExecutablePath();
    QString getTempDirectory();
    bool createDirectory(const QString &path);
    bool isValidZipFile(const QString &filePath);
    void cleanupTempFiles();
    
    // 进度更新
    void updateProgress(int percentage, const QString &message);
    
    // 成员变量
    QTimer *m_progressTimer;
    QString m_tempArchivePath;
    QString m_installPath;
    int m_currentProgress;
    
    // 常量
    static const QByteArray ZIP_SIGNATURE;
    static const QByteArray ZIP_END_SIGNATURE;
    static const int BUFFER_SIZE = 8192;
};

#endif // INSTALLER_H