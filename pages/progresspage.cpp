#include "progresspage.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QTimer>
#include <QApplication>

ProgressPage::ProgressPage(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(30, 30, 30, 30);
    
    // 图标
    iconLabel = new QLabel(this);
    iconLabel->setPixmap(QPixmap(":/images/installing.png").scaled(120, 120, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    iconLabel->setAlignment(Qt::AlignCenter);
    
    // 状态标签
    statusLabel = new QLabel(tr("正在准备安装..."), this);
    statusLabel->setStyleSheet("font-size: 18px; color: #303133; font-weight: bold;");
    statusLabel->setAlignment(Qt::AlignCenter);
    
    // 进度条
    progressBar = new QProgressBar(this);
    progressBar->setStyleSheet(R"(
        QProgressBar {
            border: none;
            background-color: #F5F7FA;
            border-radius: 2px;
            height: 6px;
            text-align: center;
        }
        QProgressBar::chunk {
            background-color: #409EFF;
            border-radius: 2px;
        }
    )");
    progressBar->setTextVisible(false);
    
    // 百分比标签
    percentLabel = new QLabel("0%", this);
    percentLabel->setStyleSheet("color: #909399;");
    percentLabel->setAlignment(Qt::AlignCenter);
    
    mainLayout->addStretch();
    mainLayout->addWidget(iconLabel);
    mainLayout->addWidget(statusLabel);
    mainLayout->addWidget(progressBar);
    mainLayout->addWidget(percentLabel);
    mainLayout->addStretch();
    
    setStyleSheet("background-color: white;");
}

void ProgressPage::startInstallation()
{
    progressBar->setValue(0);
    statusLabel->setText(tr("正在准备安装..."));
    percentLabel->setText("0%");
}

void ProgressPage::updateProgress(int progress, const QString &status)
{
    // 更新进度条
    progressBar->setValue(progress);
    
    // 更新状态文本
    statusLabel->setText(status);
    
    // 更新百分比
    percentLabel->setText(QString("%1%").arg(progress));
    
    // 如果安装完成，发送完成信号
    if (progress >= 100) {
        QTimer::singleShot(500, this, [this]() {
            emit finished();
        });
    }
    
    // 强制更新界面
    QApplication::processEvents();
} 