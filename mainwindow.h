#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProgressBar>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextEdit>
#include <QTimer>
#include <QMovie>
#include <QLineEdit>
#include <QFileDialog>
#include <QCheckBox>
#include <QStandardPaths>
#include <QProcess>
#include "installer.h"

QT_BEGIN_NAMESPACE
class QVBoxLayout;
class QHBoxLayout;
class QLabel;
class QPushButton;
class QProgressBar;
class QTextEdit;
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void startInstallation();
    void onInstallationProgress(int percentage, const QString &message);
    void onInstallationFinished(bool success, const QString &message);
    void onInstallationError(const QString &error);
    void browseInstallPath();

private:
    void setupUI();
    void setupWelcomePage();
    void setupInstallPage();
    void setupFinishPage();
    void showWelcomePage();
    void showInstallPage();
    void showFinishPage(bool success);
    bool checkExistingInstallation(const QString &path);
    void deleteOldInstallation(const QString &path);
    void createDesktopShortcut(const QString &installPath);
    void launchApplication(const QString &installPath);
    
    // UI组件
    QWidget *m_centralWidget;
    QVBoxLayout *m_mainLayout;
    
    // 欢迎页面
    QWidget *m_welcomePage;
    QLabel *m_welcomeIcon;
    QLabel *m_welcomeTitle;
    QLabel *m_installPathLabel;
    QLineEdit *m_installPathEdit;
    QPushButton *m_browseButton;
    QPushButton *m_installButton;
    
    // 安装页面
    QWidget *m_installPage;
    QLabel *m_installIcon;
    QLabel *m_installTitle;
    QLabel *m_installStatus;
    QProgressBar *m_progressBar;
    QTextEdit *m_logOutput;
    
    // 完成页面
    QWidget *m_finishPage;
    QLabel *m_finishIcon;
    QLabel *m_finishTitle;
    QLabel *m_finishMessage;
    QCheckBox *m_launchCheckBox;
    QPushButton *m_finishButton;
    
    // 安装器
    Installer *m_installer;
    
    // 动画
    QMovie *m_loadingMovie;
    
    // 升级模式
    bool m_isUpgradeMode;
};

#endif // MAINWINDOW_H