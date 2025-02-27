#ifndef APPLICATION_H
#define APPLICATION_H

#include <QApplication>
#include <QStringList>
#include <QTranslator>
#include <QSettings>

/**
 * @brief 应用程序类
 *
 * 扩展QApplication类，添加应用程序特定的功能
 */
class Application : public QApplication
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param argc 命令行参数数量
     * @param argv 命令行参数数组
     */
    Application(int &argc, char **argv);

    /**
     * @brief 析构函数
     */
    ~Application();

    /**
     * @brief 获取应用程序实例
     * @return 应用程序实例的指针
     */
    static Application *instance();

    /**
     * @brief 读取应用程序设置
     * @param key 设置键名
     * @param defaultValue 默认值（如果设置不存在）
     * @return 设置值
     */
    QVariant getSetting(const QString &key, const QVariant &defaultValue = QVariant());

    /**
     * @brief 写入应用程序设置
     * @param key 设置键名
     * @param value 设置值
     */
    void setSetting(const QString &key, const QVariant &value);

    /**
     * @brief 获取最近使用的文件列表
     * @return 最近文件路径列表
     */
    QStringList getRecentFiles();

    /**
     * @brief 添加文件到最近使用的文件列表
     * @param filePath 文件路径
     */
    void addRecentFile(const QString &filePath);

    /**
     * @brief 清空最近使用的文件列表
     */
    void clearRecentFiles();

    /**
     * @brief 获取应用程序数据目录
     * @return 数据目录路径
     */
    QString getDataDirectory();

    /**
     * @brief 获取模板目录
     * @return 模板目录路径
     */
    QString getTemplatesDirectory();

private:
    /**
     * @brief 初始化应用程序
     */
    void initialize();

    /**
     * @brief 创建必要的目录
     */
    void createDirectories();

    /**
     * @brief 加载翻译文件
     */
    void loadTranslations();

    /**
     * @brief 注册自定义元数据类型
     */
    void registerMetaTypes();

    QSettings *settings;       ///< 应用程序设置
    QTranslator appTranslator; ///< 应用程序翻译器
    QString dataDir;           ///< 数据目录
    QString templatesDir;      ///< 模板目录
    int maxRecentFiles;        ///< 最大记住的最近文件数量
};

#endif // APPLICATION_H