#include <QApplication>
#include "mainwindow.h"
#include <QMessageBox>
#include <windows.h>

int main(int argc, char *argv[]) {
    // 检查是否以管理员权限运行
    BOOL isAdmin = FALSE;
    PSID adminGroup = NULL;
    SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;
    
    if (AllocateAndInitializeSid(&ntAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID,
                                DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0,
                                &adminGroup)) {
        CheckTokenMembership(NULL, adminGroup, &isAdmin);
        FreeSid(adminGroup);
    }
    
    // 如果不是管理员权限，重新以管理员权限启动
    if (!isAdmin) {
        SHELLEXECUTEINFO sei = {sizeof(sei)};
        sei.lpVerb = L"runas";
        sei.lpFile = (LPCWSTR)argv[0];
        sei.nShow = SW_NORMAL;
        
        if (!ShellExecuteEx(&sei)) {
            QMessageBox::critical(nullptr, QObject::tr("错误"),
                QObject::tr("安装程序需要管理员权限才能继续。\n"
                           "请右键点击安装程序，选择\"以管理员身份运行\"。"));
            return 1;
        }
        return 0;
    }
    
    QApplication a(argc, argv);
    
    // 设置高DPI支持
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    
    MainWindow w;
    w.show();  // 确保窗口显示
    
    return QApplication::exec();
}