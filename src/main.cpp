#include "application.h"
#include "mainwindow.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QLocale>
#include <QTranslator>
#include <QLibraryInfo>
#include <QDir>
#include <QDebug>
#include <QSplashScreen>
#include <QPixmap>
#include <QThread>

int main(int argc, char *argv[])
{
    // 设置高DPI显示支持
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    // 创建应用程序实例
    Application app(argc, argv);

    // 设置应用程序信息
    app.setApplicationName("Label Printer Editor");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("YourCompany");
    app.setOrganizationDomain("yourcompany.com");

    // 显示启动画面
    QPixmap splashPixmap(":/icons/app.png");
    QSplashScreen splash(splashPixmap);
    splash.show();

    // 确保启动画面在一定时间内显示
    app.processEvents();

    // 加载翻译
    QTranslator qtTranslator;
    if (qtTranslator.load(QLocale::system(), "qt", "_",
                        QLibraryInfo::location(QLibraryInfo::TranslationsPath))) {
        app.installTranslator(&qtTranslator);
    }

    QTranslator appTranslator;
    if (appTranslator.load(QLocale::system(), "printer", "_",
                        app.applicationDirPath() + "/translations")) {
        app.installTranslator(&appTranslator);
    }

    // 命令行参数解析
    QCommandLineParser parser;
    parser.setApplicationDescription("Label Printer Editor");
    parser.addHelpOption();
    parser.addVersionOption();

    // 添加文件参数
    parser.addPositionalArgument("file", "The file to open.");

    // 处理命令行参数
    parser.process(app);

    // 获取要打开的文件（如果有）
    QString filePath;
    const QStringList args = parser.positionalArguments();
    if (!args.isEmpty()) {
        filePath = args.first();
    }

    // 创建主窗口
    MainWindow mainWindow;

    // 短暂延迟以显示启动画面
    QThread::msleep(1000);

    // 隐藏启动画面
    splash.finish(&mainWindow);

    // 如果有指定文件，则打开
    if (!filePath.isEmpty()) {
        mainWindow.openFile(filePath);
    }

    // 显示主窗口
    mainWindow.show();

    // 输出调试信息
    qDebug() << "Application started successfully";
    qDebug() << "Application path:" << app.applicationDirPath();
    qDebug() << "Data directory:" << app.getDataDirectory();
    qDebug() << "Templates directory:" << app.getTemplatesDirectory();

    // 进入应用程序事件循环
    return app.exec();
}