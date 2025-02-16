#ifndef PROGRESSPAGE_H
#define PROGRESSPAGE_H

#include <QWidget>
#include <QProgressBar>
#include <QTimer>
#include <QLabel>

class ProgressPage : public QWidget
{
    Q_OBJECT

public:
    explicit ProgressPage(QWidget *parent = nullptr);
    void startInstallation();

signals:
    void finished();

public slots:
    void updateProgress(int progress, const QString &status);

private:
    QProgressBar *progressBar;
    QLabel *statusLabel;
    QLabel *percentLabel;
    QTimer *timer;
    QLabel *iconLabel;
    
    void updateStatus(int progress);
};

#endif // PROGRESSPAGE_H 