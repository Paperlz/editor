#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QPrinter>
#include <QLabel>
#include <QUndoStack>

// 前向声明
class LabelEditView;
class PropertiesPanel;
class LabelDocument;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    // 打开文件
    bool openFile(const QString &filePath);

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    // 文件操作
    void newDocument();
    void openDocument();
    void saveDocument();
    void saveDocumentAs();
    void exportAsPDF();
    void printDocument();
    void printPreview();

    // 编辑操作
    void cutSelected();
    void copySelected();
    void pasteFromClipboard();
    void deleteSelected();

    // 元素添加
    void addTextElement();
    void addImageElement();
    void addBarcodeElement();
    void addQRCodeElement();

    // 打印设置
    void configurePrinter();

    // 视图操作
    void zoomIn();
    void zoomOut();
    void zoomReset();

    // 文档已修改
    void documentModified();

    // 元素选择变化
    void selectionChanged();

    // 撤销/重做操作
    void updateUndoRedoActions();

private:
    // 初始化UI组件
    void setupUi();
    void createActions();
    void createMenus();
    void createToolbars();
    void createStatusBar();
    void setupConnections();

    // 更新UI状态
    void updateActions();
    void updateWindowTitle();

    // 文件操作辅助函数
    bool maybeSave();
    bool saveFile(const QString &fileName);
    bool loadFile(const QString &fileName);

    // 设置当前文件
    void setCurrentFile(const QString &fileName);

    // UI组件
    Ui::MainWindow *ui;
    LabelEditView *labelEditView;
    PropertiesPanel *propertiesPanel;

    // 模型
    LabelDocument *currentDocument;

    // 打印
    QPrinter printer;

    // 撤销/重做堆栈
    QUndoStack *undoStack;

    // 状态栏组件
    QLabel *positionLabel;
    QLabel *modifiedLabel;
    QLabel *zoomLabel;

    // 动作
    QAction *newAction;
    QAction *openAction;
    QAction *saveAction;
    QAction *saveAsAction;
    QAction *exportAction;
    QAction *printAction;
    QAction *printPreviewAction;
    QAction *exitAction;

    QAction *undoAction;
    QAction *redoAction;
    QAction *cutAction;
    QAction *copyAction;
    QAction *pasteAction;
    QAction *deleteAction;

    QAction *addTextAction;
    QAction *addImageAction;
    QAction *addBarcodeAction;
    QAction *addQRCodeAction;

    QAction *zoomInAction;
    QAction *zoomOutAction;
    QAction *zoomResetAction;

    // 当前文件路径
    QString currentFilePath;
};

#endif // MAINWINDOW_H