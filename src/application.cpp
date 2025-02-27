#include "application.h"

#include <QDir>
#include <QStandardPaths>
#include <QLocale>
#include <QTranslator>
#include <QLibraryInfo>
#include <QDebug>

Application::Application(int &argc, char **argv)
    : QApplication(argc, argv)
    , settings(nullptr)
    , maxRecentFiles(10)
{
    // 设置应用程序基本信息
    setApplicationName("LabelPrinter");
    setOrganizationName("YourCompany");
    setOrganizationDomain("yourcompany.com");

    // 初始化应用程序
    initialize();
}

Application::~Application()
{
    if (settings) {
        delete settings;
        settings = nullptr;
    }
}

Application *Application::instance()
{
    return qobject_cast<Application*>(QApplication::instance());
}

QVariant Application::getSetting(const QString &key, const QVariant &defaultValue)
{
    return settings->value(key, defaultValue);
}

void Application::setSetting(const QString &key, const QVariant &value)
{
    settings->setValue(key, value);
}

QStringList Application::getRecentFiles()
{
    return settings->value("recentFiles").toStringList();
}

void Application::addRecentFile(const QString &filePath)
{
    QStringList files = getRecentFiles();

    // 如果文件已在列表中，先移除它
    files.removeAll(filePath);

    // 将文件添加到列表头部
    files.prepend(filePath);

    // 只保留指定数量的文件
    while (files.size() > maxRecentFiles) {
        files.removeLast();
    }

    // 保存更新后的列表
    settings->setValue("recentFiles", files);
}

void Application::clearRecentFiles()
{
    settings->setValue("recentFiles", QStringList());
}

QString Application::getDataDirectory()
{
    return dataDir;
}

QString Application::getTemplatesDirectory()
{
    return templatesDir;
}

void Application::initialize()
{
    // 创建设置对象
    settings = new QSettings(QSettings::IniFormat, QSettings::UserScope,
                            organizationName(), applicationName());

    // 设置数据目录
    dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

    // 设置模板目录
    templatesDir = dataDir + "/templates";

    // 创建必要的目录
    createDirectories();

    // 加载翻译
    loadTranslations();

    // 注册元类型
    registerMetaTypes();

    // 输出调试信息
    qDebug() << "应用程序初始化完成";
    qDebug() << "数据目录:" << dataDir;
    qDebug() << "模板目录:" << templatesDir;
}

void Application::createDirectories()
{
    QDir dir;

    // 创建数据目录
    if (!dir.exists(dataDir)) {
        if (!dir.mkpath(dataDir)) {
            qWarning() << "无法创建数据目录:" << dataDir;
        }
    }

    // 创建模板目录
    if (!dir.exists(templatesDir)) {
        if (!dir.mkpath(templatesDir)) {
            qWarning() << "无法创建模板目录:" << templatesDir;
        } else {
            // 可以在这里复制默认模板到模板目录
            // TODO: 复制默认模板
        }
    }

    // 创建其他必要的目录
    QString imagesDir = dataDir + "/images";
    if (!dir.exists(imagesDir)) {
        dir.mkpath(imagesDir);
    }

    QString cachesDir = dataDir + "/caches";
    if (!dir.exists(cachesDir)) {
        dir.mkpath(cachesDir);
    }
}

void Application::loadTranslations()
{
    // 获取系统语言环境
    QLocale locale = QLocale::system();

    // 加载Qt翻译
    QTranslator qtTranslator;
    if (qtTranslator.load(locale, "qt", "_", QLibraryInfo::location(QLibraryInfo::TranslationsPath))) {
        installTranslator(&qtTranslator);
    }

    // 加载应用程序翻译
    QString translationsPath = dataDir + "/translations";
    if (appTranslator.load(locale, "labelprinter", "_", translationsPath)) {
        installTranslator(&appTranslator);
    } else {
        // 尝试从应用程序目录加载翻译
        QString appDirPath = applicationDirPath() + "/translations";
        if (appTranslator.load(locale, "labelprinter", "_", appDirPath)) {
            installTranslator(&appTranslator);
        }
    }
}

void Application::registerMetaTypes()
{
    // 注册自定义类型，以便在Qt的信号/槽和属性系统中使用
    // 例如：
    // qRegisterMetaType<CustomType>("CustomType");

    // 注：如果需要，可以为序列化添加类型转换函数
    // QMetaType::registerConverter<CustomType, QString>([](const CustomType &obj) -> QString {
    //     return obj.toString();
    // });

    // 注：如果需要，可以为反序列化添加类型转换函数
    // QMetaType::registerConverter<QString, CustomType>([](const QString &str) -> CustomType {
    //     return CustomType::fromString(str);
    // });
}