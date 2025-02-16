#ifndef COMPLETEPAGE_H
#define COMPLETEPAGE_H

#include <QWidget>
#include <QCheckBox>

class CompletePage : public QWidget
{
    Q_OBJECT

public:
    explicit CompletePage(QWidget *parent = nullptr);
    void setInstallPath(const QString &path);

signals:
    void finished();

private slots:
    void onFinishClicked();

private:
    QCheckBox *launchCheckBox;
    QString installPath;
};

#endif // COMPLETEPAGE_H 