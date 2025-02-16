#ifndef INSTALLPAGE_H
#define INSTALLPAGE_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QStorageInfo>

class InstallPage : public QWidget
{
    Q_OBJECT

public:
    explicit InstallPage(QWidget *parent = nullptr);
    QLineEdit *locationEdit;

signals:
    void startInstall();
    void installProgress(int progress, const QString &status);

private slots:
    void browseLocation();
    void updateInstallButton();
    void performInstallation();

private:
    QPushButton *installButton;
    QLabel *spaceLabel;
    void checkDiskSpace(const QString &path);
    bool extractZip(const QString &zipPath, const QString &targetPath);
    bool createShortcut(const QString &exePath, const QString &shortcutPath);
};

#endif // INSTALLPAGE_H 