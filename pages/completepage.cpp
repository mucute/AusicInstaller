#include "completepage.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QCheckBox>
#include <QProcess>
#include <QDebug>

CompletePage::CompletePage(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(30, 30, 30, 30);
    
    // 图标
    QLabel *iconLabel = new QLabel(this);
    iconLabel->setPixmap(QPixmap(":/images/success.png").scaled(120, 120, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    iconLabel->setAlignment(Qt::AlignCenter);
    
    // 标题
    QLabel *titleLabel = new QLabel(tr("安装完成"), this);
    titleLabel->setStyleSheet("font-size: 24px; color: #303133; font-weight: bold;");
    titleLabel->setAlignment(Qt::AlignCenter);
    
    // 描述
    QLabel *descLabel = new QLabel(tr("音触 已成功安装到您的计算机"), this);
    descLabel->setStyleSheet("color: #606266;");
    descLabel->setAlignment(Qt::AlignCenter);
    
    // 复选框
    launchCheckBox = new QCheckBox(tr("立即启动 音触"), this);
    launchCheckBox->setChecked(true);
    launchCheckBox->setStyleSheet(R"(
        QCheckBox {
            color: #606266;
        }
        QCheckBox::indicator {
            width: 16px;
            height: 16px;
            border: 1px solid #DCDFE6;
            border-radius: 2px;
        }
        QCheckBox::indicator:checked {
            background-color: #409EFF;
            border-color: #409EFF;
            image: url(:/images/check.png);
        }
        QCheckBox::indicator:hover {
            border-color: #409EFF;
        }
    )");
    
    // 完成按钮
    QPushButton *finishButton = new QPushButton(tr("完成"), this);
    finishButton->setFixedSize(120, 40);
    finishButton->setStyleSheet(R"(
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
    )");
    
    mainLayout->addStretch();
    mainLayout->addWidget(iconLabel);
    mainLayout->addWidget(titleLabel);
    mainLayout->addWidget(descLabel);
    mainLayout->addWidget(launchCheckBox, 0, Qt::AlignCenter);
    mainLayout->addWidget(finishButton, 0, Qt::AlignCenter);
    mainLayout->addStretch();
    
    connect(finishButton, &QPushButton::clicked, this, &CompletePage::onFinishClicked);
    
    setStyleSheet("background-color: white;");
}

void CompletePage::setInstallPath(const QString &path)
{
    installPath = path;
    qDebug() << "Set install path:" << path;
}

void CompletePage::onFinishClicked()
{
    if (launchCheckBox->isChecked()) {
        QString exePath = installPath + "/org.example.project.exe";
        qDebug() << "Attempting to launch:" << exePath;
        
        QProcess *process = new QProcess(this);
        process->setWorkingDirectory(installPath);
        
        // 使用绝对路径启动程序
        QStringList arguments;
        bool success = process->startDetached(exePath, arguments, installPath);
        
        if (!success) {
            qDebug() << "Launch failed with error:" << process->errorString();
            qDebug() << "Working directory:" << process->workingDirectory();
            qDebug() << "Program:" << exePath;
        } else {
            qDebug() << "Launch successful";
        }
        
        process->deleteLater();
    }
    emit finished();
} 