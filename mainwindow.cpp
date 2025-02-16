#include "mainwindow.h"
#include <QVBoxLayout>
#include <QApplication>
#include <QMouseEvent>
#include <QPainter>
#include <QGraphicsDropShadowEffect>
#include <QPushButton>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // 基本窗口设置
    setWindowTitle(tr("软件安装"));
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedSize(800, 600);
    
    // 创建中心部件
    QWidget *centralWidget = new QWidget(this);
    centralWidget->setObjectName("centralWidget");
    setCentralWidget(centralWidget);
    
    // 设置中心部件样式
    centralWidget->setStyleSheet(R"(
        QWidget#centralWidget {
            background-color: white;
            border-radius: 8px;
            border: 1px solid #DCDFE6;
        }
    )");
    centralWidget->setContentsMargins(0, 0, 0, 0);
    
    // 添加阴影效果
    QGraphicsDropShadowEffect *shadowEffect = new QGraphicsDropShadowEffect(this);
    shadowEffect->setBlurRadius(20);
    shadowEffect->setColor(QColor(0, 0, 0, 80));
    shadowEffect->setOffset(0, 0);
    centralWidget->setGraphicsEffect(shadowEffect);
    
    // 创建堆叠部件
    stackedWidget = new QStackedWidget(centralWidget);
    
    // 创建页面
    installPage = new InstallPage(this);
    progressPage = new ProgressPage(this);
    completePage = new CompletePage(this);
    
    stackedWidget->addWidget(installPage);
    stackedWidget->addWidget(progressPage);
    stackedWidget->addWidget(completePage);
    
    // 设置布局
    QVBoxLayout *layout = new QVBoxLayout(centralWidget);
    layout->setContentsMargins(0, 0, 0, 0);  // 移除边距
    layout->addWidget(stackedWidget);
    
    // 添加关闭按钮
    QPushButton *closeButton = new QPushButton(this);
    closeButton->setFixedSize(44, 44);
    closeButton->move(width() - 52, 8);
    closeButton->setStyleSheet(R"(
        QPushButton {
            border: none;
            border-radius: 22px;
            background-color: transparent;
            color: #909399;
            font-size: 22px;
            font-weight: normal;
            font-family: "Microsoft YaHei", "Segoe UI", sans-serif;
            padding: 0;
            margin: 0;
        }
        QPushButton:hover {
            background-color: transparent;
            color: #F56C6C;
        }
        QPushButton:pressed {
            background-color: transparent;
            color: #F56C6C;
            padding: 0;
            margin: 0;
        }
    )");
    closeButton->setText("×");
    closeButton->setFlat(true);  // 设置为平面按钮
    
    connect(installPage, &InstallPage::startInstall, this, [this]() {
        stackedWidget->setCurrentWidget(progressPage);
    });
    
    connect(installPage, &InstallPage::installProgress, progressPage, &ProgressPage::updateProgress);
    
    connect(progressPage, &ProgressPage::finished, this, [this]() {
        completePage->setInstallPath(installPage->locationEdit->text());
        stackedWidget->setCurrentWidget(completePage);
    });
    
    connect(completePage, &CompletePage::finished, this, [this]() {
        close();
    });
    
    connect(closeButton, &QPushButton::clicked, this, &MainWindow::close);
    
    // 显示初始页面
    stackedWidget->setCurrentWidget(installPage);
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    if (findChild<QWidget*>("bgWidget")) {
        findChild<QWidget*>("bgWidget")->setGeometry(0, 0, width(), height());
    }
    // 更新关闭按钮位置
    if (QPushButton *closeButton = findChild<QPushButton*>()) {
        closeButton->move(width() - 52, 8);
    }
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        isDragging = true;
        dragPosition = event->globalPos() - frameGeometry().topLeft();
    }
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton && isDragging) {
        move(event->globalPos() - dragPosition);
    }
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        isDragging = false;
    }
}

MainWindow::~MainWindow()
{
} 