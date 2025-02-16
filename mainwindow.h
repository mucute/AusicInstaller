#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QGraphicsDropShadowEffect>
#include "pages/installpage.h"
#include "pages/progresspage.h"
#include "pages/completepage.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void initWindowStyle();
    
    QStackedWidget *stackedWidget;
    InstallPage *installPage;
    ProgressPage *progressPage;
    CompletePage *completePage;
    
    QPoint dragPosition;
    bool isDragging = false;
};

#endif // MAINWINDOW_H 