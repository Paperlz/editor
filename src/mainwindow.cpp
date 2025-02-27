#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "models/labelmodels.h"
#include "ui/labeleditview.h"
#include "ui/propertiespanel.h"
#include "application.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QPrinterInfo>
#include <QPrintDialog>
#include <QPrintPreviewDialog>
#include <QPageSetupDialog>
#include <QCloseEvent>
#include <QClipboard>
#include <QMimeData>
#include <QUndoView>
#include <QSettings>
#include <QStandardPaths>
#include <QImageReader>
#include <QLabel>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_undoStack(new QUndoStack(this))
    , m_currentDocument(nullptr)
{
    // 设置UI
    ui->setupUi(this);

    // 初始化属性面板
    m_propertiesPanel = new PropertiesPanel(this);
    addDockWidget(Qt::RightDockWidgetArea, m_propertiesPanel);

    // 设置状态栏
    setupStatusBar();

    // 连接信号和槽
    connectSignals();

    // 设置撤销栈
    m_undoStack->setUndoLimit(50);

    // 设置撤销/重做动作
    ui->actionUndo->setEnabled(false);
    ui->actionRedo->setEnabled(false);

    connect(m_undoStack, &QUndoStack::canUndoChanged, ui->actionUndo, &QAction::setEnabled);
    connect(m_undoStack, &QUndoStack::canRedoChanged, ui->actionRedo, &QAction::setEnabled);

    // 更新撤销/重做文本
    connect(m_undoStack, &QUndoStack::undoTextChanged, [this](const QString &text) {
        ui->actionUndo->setText(tr("撤销 %1").arg(text));
    });

    connect(m_undoStack, &QUndoStack::redoTextChanged, [this](const QString &text) {
        ui->actionRedo->setText(tr("重做 %1").arg(text));
    });

    // 创建新文档
    newDocument();

    // 读取窗口设置
    readSettings();
}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::openFile(const QString &filePath)
{
    if (maybeSave()) {
        return loadFile(filePath);
    }
    return false;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (maybeSave()) {
        writeSettings();
        event->accept();
    } else {
        event->ignore();
    }
}

void MainWindow::newDocument()
{
    if (maybeSave()) {
        // 创建新文档
        if (m_currentDocument) {
            delete m_currentDocument;
        }

        m_currentDocument = new LabelDocument(this);

        // 设置默认纸张大小（可以从设置中读取）
        m_currentDocument->setPageSize(QPrinter::A4);

        // 更新界面
        ui->labelEditView->setDocument(m_currentDocument);
        m_propertiesPanel->setDocument(m_currentDocument);

        // 设置撤销栈
        m_currentDocument->setUndoStack(m_undoStack);

        // 清除撤销栈
        m_undoStack->clear();

        // 重置当前文件路径
        setCurrentFile("");

        // 更新窗口标题
        updateWindowTitle();

        // 更新动作状态
        updateActions();

        // 显示就绪消息
        statusBar()->showMessage(tr("新文档已创建"), 2000);
    }
}

void MainWindow::openDocument()
{
    if (maybeSave()) {
        QString fileName = QFileDialog::getOpenFileName(this,
            tr("打开标签文档"), QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
            tr("标签文档 (*.lbl);;所有文件 (*)"));

        if (!fileName.isEmpty()) {
            loadFile(fileName);
        }
    }
}

void MainWindow::saveDocument()
{
    if (m_currentFilePath.isEmpty()) {
        saveDocumentAs();
    } else {
        saveFile(m_currentFilePath);
    }
}

void MainWindow::saveDocumentAs()
{
    QString fileName = QFileDialog::getSaveFileName(this,
        tr("保存标签文档"), QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
        tr("标签文档 (*.lbl);;所有文件 (*)"));

    if (!fileName.isEmpty()) {
        saveFile(fileName);
    }
}

void MainWindow::exportAsPDF()
{
    QString fileName = QFileDialog::getSaveFileName(this,
        tr("导出为PDF"), QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
        tr("PDF文件 (*.pdf)"));

    if (!fileName.isEmpty()) {
        QPrinter printer(QPrinter::HighResolution);
        printer.setOutputFormat(QPrinter::PdfFormat);
        printer.setOutputFileName(fileName);

        // 设置页面大小和方向
        printer.setPageSize(m_currentDocument->pageSize());
        printer.setPageOrientation(m_currentDocument->orientation());

        // 执行打印
        QPainter painter;
        if (painter.begin(&printer)) {
            m_currentDocument->render(&painter, printer.pageRect());
            painter.end();

            QMessageBox::information(this, tr("导出成功"),
                tr("文档已成功导出为PDF。"));
        } else {
            QMessageBox::warning(this, tr("导出失败"),
                tr("无法导出文档为PDF。"));
        }
    }
}

void MainWindow::printDocument()
{
    // 配置打印机
    QPrintDialog dialog(&m_printer, this);
    if (dialog.exec() == QDialog::Accepted) {
        // 执行打印
        QPainter painter;
        if (painter.begin(&m_printer)) {
            m_currentDocument->render(&painter, m_printer.pageRect());
            painter.end();

            statusBar()->showMessage(tr("打印完成"), 2000);
        } else {
            QMessageBox::warning(this, tr("打印失败"),
                tr("无法打印文档。"));
        }
    }
}

void MainWindow::printPreview()
{
    // 创建打印预览对话框
    QPrintPreviewDialog preview(&m_printer, this);

    // 连接打印信号
    connect(&preview, &QPrintPreviewDialog::paintRequested,
            this, [this](QPrinter *printer) {
                QPainter painter;
                if (painter.begin(printer)) {
                    m_currentDocument->render(&painter, printer->pageRect());
                    painter.end();
                }
            });

    // 显示预览对话框
    preview.exec();
}

void MainWindow::cutSelectedItems()
{
    ui->labelEditView->cutSelectedItems();
}

void MainWindow::copySelectedItems()
{
    ui->labelEditView->copySelectedItems();
}

void MainWindow::pasteItems()
{
    ui->labelEditView->pasteItems();
}

void MainWindow::deleteSelectedItems()
{
    ui->labelEditView->deleteSelectedItems();
}

void MainWindow::selectAllItems()
{
    ui->labelEditView->selectAll();
}

void MainWindow::deselectAllItems()
{
    ui->labelEditView->deselectAll();
}

void MainWindow::addTextElement()
{
    ui->labelEditView->addTextElement();
}

void MainWindow::addImageElement()
{
    ui->labelEditView->addImageElement();
}

void MainWindow::addBarcodeElement()
{
    ui->labelEditView->addBarcodeElement();
}

void MainWindow::addQRCodeElement()
{
    ui->labelEditView->addQRCodeElement();
}

void MainWindow::zoomIn()
{
    ui->labelEditView->zoomIn();
}

void MainWindow::zoomOut()
{
    ui->labelEditView->zoomOut();
}

void MainWindow::zoomReset()
{
    ui->labelEditView->zoomReset();
}

void MainWindow::zoomFit()
{
    ui->labelEditView->zoomToFit();
}

void MainWindow::showGridToggled(bool checked)
{
    // 在LabelEditView中实现
}

void MainWindow::showRulersToggled(bool checked)
{
    // 在LabelEditView中实现
}

void MainWindow::snapToGridToggled(bool checked)
{
    // 在LabelEditView中实现
}

void MainWindow::showPageSetupDialog()
{
    // 创建页面设置对话框
    QPageSetupDialog dialog(&m_printer, this);
    if (dialog.exec() == QDialog::Accepted) {
        // 更新文档的页面设置
        m_currentDocument->setPageSize(m_printer.pageLayout().pageSize().id());
        m_currentDocument->setOrientation(m_printer.pageLayout().orientation());

        // 更新视图
        ui->labelEditView->updateView();
    }
}

void MainWindow::showPrinterSettingsDialog()
{
    // 创建打印机设置对话框
    QPrinterInfo printerInfo(m_printer);
    QPrintDialog dialog(&m_printer, this);
    dialog.setWindowTitle(tr("打印机设置"));

    if (dialog.exec() == QDialog::Accepted) {
        // 更新视图
        ui->labelEditView->updateView();
    }
}

void MainWindow::showAboutDialog()
{
    QMessageBox::about(this, tr("关于标签打印编辑器"),
        tr("<h3>标签打印编辑器 v1.0</h3>"
           "<p>一个用于设计和打印标签的应用程序。</p>"
           "<p>支持文本、图像、条形码和二维码元素。</p>"
           "<p>© 2023 YourCompany</p>"));
}

void MainWindow::documentModified()
{
    // 设置文档已修改标志
    setWindowModified(true);

    // 更新窗口标题
    updateWindowTitle();

    // 更新状态栏
    m_modifiedLabel->setText(tr("已修改"));
}

void MainWindow::selectionChanged()
{
    // 更新动作状态
    updateActions();

    // 更新属性面板
    m_propertiesPanel->updateSelection(ui->labelEditView->selectedItems());
}

void MainWindow::mousePositionChanged(const QPointF &pos)
{
    // 更新状态栏中的位置显示
    m_positionLabel->setText(tr("位置: X=%1, Y=%2").arg(pos.x(), 0, 'f', 1).arg(pos.y(), 0, 'f', 1));
}

void MainWindow::zoomChanged(qreal zoom)
{
    // 更新状态栏中的缩放显示
    m_zoomLabel->setText(tr("缩放: %1%").arg(zoom * 100, 0, 'f', 0));
}

void MainWindow::setupStatusBar()
{
    // 创建状态栏标签
    m_positionLabel = new QLabel(tr("位置: X=0, Y=0"), this);
    m_modifiedLabel = new QLabel(tr("未修改"), this);
    m_zoomLabel = new QLabel(tr("缩放: 100%"), this);

    // 添加到状态栏
    statusBar()->addWidget(m_positionLabel);
    statusBar()->addPermanentWidget(m_modifiedLabel);
    statusBar()->addPermanentWidget(m_zoomLabel);

    // 显示就绪消息
    statusBar()->showMessage(tr("就绪"), 3000);
}

void MainWindow::connectSignals()
{
    // 文件菜单
    connect(ui->actionNew, &QAction::triggered, this, &MainWindow::newDocument);
    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::openDocument);
    connect(ui->actionSave, &QAction::triggered, this, &MainWindow::saveDocument);
    connect(ui->actionSaveAs, &QAction::triggered, this, &MainWindow::saveDocumentAs);
    connect(ui->actionExportPDF, &QAction::triggered, this, &MainWindow::exportAsPDF);
    connect(ui->actionPrint, &QAction::triggered, this, &MainWindow::printDocument);
    connect(ui->actionPrintPreview, &QAction::triggered, this, &MainWindow::printPreview);
    connect(ui->actionExit, &QAction::triggered, this, &QWidget::close);

    // 编辑菜单
    connect(ui->actionUndo, &QAction::triggered, m_undoStack, &QUndoStack::undo);
    connect(ui->actionRedo, &QAction::triggered, m_undoStack, &QUndoStack::redo);
    connect(ui->actionCut, &QAction::triggered, this, &MainWindow::cutSelectedItems);
    connect(ui->actionCopy, &QAction::triggered, this, &MainWindow::copySelectedItems);
    connect(ui->actionPaste, &QAction::triggered, this, &MainWindow::pasteItems);
    connect(ui->actionDelete, &QAction::triggered, this, &MainWindow::deleteSelectedItems);
    connect(ui->actionSelectAll, &QAction::triggered, this, &MainWindow::selectAllItems);
    connect(ui->actionDeselectAll, &QAction::triggered, this, &MainWindow::deselectAllItems);

    // 插入菜单
    connect(ui->actionAddText, &QAction::triggered, this, &MainWindow::addTextElement);
    connect(ui->actionAddImage, &QAction::triggered, this, &MainWindow::addImageElement);
    connect(ui->actionAddBarcode, &QAction::triggered, this, &MainWindow::addBarcodeElement);
    connect(ui->actionAddQRCode, &QAction::triggered, this, &MainWindow::addQRCodeElement);

    // 视图菜单
    connect(ui->actionZoomIn, &QAction::triggered, this, &MainWindow::zoomIn);
    connect(ui->actionZoomOut, &QAction::triggered, this, &MainWindow::zoomOut);
    connect(ui->actionZoomReset, &QAction::triggered, this, &MainWindow::zoomReset);
    connect(ui->actionZoomFit, &QAction::triggered, this, &MainWindow::zoomFit);
    connect(ui->actionShowGrid, &QAction::toggled, this, &MainWindow::showGridToggled);
    connect(ui->actionShowRulers, &QAction::toggled, this, &MainWindow::showRulersToggled);
    connect(ui->actionSnapToGrid, &QAction::toggled, this, &MainWindow::snapToGridToggled);

    // 设置菜单
    connect(ui->actionPageSetup, &QAction::triggered, this, &MainWindow::showPageSetupDialog);
    connect(ui->actionPrinterSettings, &QAction::triggered, this, &MainWindow::showPrinterSettingsDialog);

    // 帮助菜单
    connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::showAboutDialog);

    // 编辑视图信号
    connect(ui->labelEditView, &LabelEditView::selectionChanged, this, &MainWindow::selectionChanged);
    connect(ui->labelEditView, &LabelEditView::mousePositionChanged, this, &MainWindow::mousePositionChanged);
    connect(ui->labelEditView, &LabelEditView::zoomChanged, this, &MainWindow::zoomChanged);
}

void MainWindow::updateActions()
{
    QList<LabelItem*> selectedItems = ui->labelEditView->selectedItems();
    bool hasSelection = !selectedItems.isEmpty();

    // 更新需要选中项的动作状态
    ui->actionCut->setEnabled(hasSelection);
    ui->actionCopy->setEnabled(hasSelection);
    ui->actionDelete->setEnabled(hasSelection);

    // 更新粘贴动作状态
    const QClipboard *clipboard = QApplication::clipboard();
    const QMimeData *mimeData = clipboard->mimeData();
    ui->actionPaste->setEnabled(mimeData->hasFormat("application/x-labelitem") ||
                               mimeData->hasImage() ||
                               mimeData->hasText());
}

void MainWindow::updateWindowTitle()
{
    QString title = tr("标签打印编辑器");

    if (!m_currentFilePath.isEmpty()) {
        QFileInfo fileInfo(m_currentFilePath);
        title = fileInfo.fileName() + "[*] - " + title;
    } else {
        title = tr("未命名[*] - ") + title;
    }

    setWindowTitle(title);
}

bool MainWindow::maybeSave()
{
    if (!isWindowModified()) {
        return true;
    }

    QMessageBox::StandardButton ret = QMessageBox::warning(this, tr("未保存的更改"),
        tr("文档已被修改。\n是否保存更改?"),
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

    if (ret == QMessageBox::Save) {
        return saveDocument();
    } else if (ret == QMessageBox::Cancel) {
        return false;
    }

    return true;
}

bool MainWindow::saveFile(const QString &fileName)
{
    if (!m_currentDocument) {
        return false;
    }

    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("保存失败"),
            tr("无法写入文件 %1:\n%2.")
            .arg(QDir::toNativeSeparators(fileName), file.errorString()));
        return false;
    }

    // 保存文档内容
    bool success = m_currentDocument->saveToXml(&file);

    if (success) {
        // 设置当前文件路径
        setCurrentFile(fileName);
        statusBar()->showMessage(tr("文件已保存"), 2000);
        return true;
    } else {
        QMessageBox::warning(this, tr("保存失败"),
            tr("保存文件 %1 时出错")
            .arg(QDir::toNativeSeparators(fileName)));
        return false;
    }
}

bool MainWindow::loadFile(const QString &fileName)
{
    if (!m_currentDocument) {
        m_currentDocument = new LabelDocument(this);
    }

    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("打开失败"),
            tr("无法读取文件 %1:\n%2.")
            .arg(QDir::toNativeSeparators(fileName), file.errorString()));
        return false;
    }

    // 加载文档内容
    bool success = m_currentDocument->loadFromXml(&file);

    if (success) {
        // 更新界面
        ui->labelEditView->setDocument(m_currentDocument);
        m_propertiesPanel->setDocument(m_currentDocument);

        // 设置撤销栈
        m_currentDocument->setUndoStack(m_undoStack);

        // 清除撤销栈
        m_undoStack->clear();

        // 设置当前文件路径
        setCurrentFile(fileName);

        // 连接文档修改信号
        connect(m_currentDocument, &LabelDocument::documentModified,
                this, &MainWindow::documentModified, Qt::UniqueConnection);

        statusBar()->showMessage(tr("文件已加载"), 2000);
        return true;
    } else {
        QMessageBox::warning(this, tr("打开失败"),
            tr("文件格式错误或不支持: %1")
            .arg(QDir::toNativeSeparators(fileName)));

        // 创建一个新的空文档
        delete m_currentDocument;
        m_currentDocument = new LabelDocument(this);
        ui->labelEditView->setDocument(m_currentDocument);
        m_propertiesPanel->setDocument(m_currentDocument);

        // 设置撤销栈
        m_currentDocument->setUndoStack(m_undoStack);

        // 连接文档修改信号
        connect(m_currentDocument, &LabelDocument::documentModified,
                this, &MainWindow::documentModified, Qt::UniqueConnection);

        return false;
    }
}

void MainWindow::setCurrentFile(const QString &fileName)
{
    m_currentFilePath = fileName;

    // 文档未修改
    setWindowModified(false);
    m_modifiedLabel->setText(tr("未修改"));

    // 更新窗口标题
    updateWindowTitle();

    // 更新最近文件列表
    QSettings settings;
    QStringList files = settings.value("recentFileList").toStringList();

    files.removeAll(fileName);
    if (!fileName.isEmpty()) {
        files.prepend(fileName);
    }

    while (files.size() > 10) {
        files.removeLast();
    }

    settings.setValue("recentFileList", files);

    // 更新最近文件菜单（如果有）
    // 这部分可以在需要时实现
}

void MainWindow::readSettings()
{
    QSettings settings;

    // 读取窗口几何信息
    const QByteArray geometry = settings.value("geometry", QByteArray()).toByteArray();
    if (!geometry.isEmpty()) {
        restoreGeometry(geometry);
    }

    // 读取窗口状态
    const QByteArray state = settings.value("windowState", QByteArray()).toByteArray();
    if (!state.isEmpty()) {
        restoreState(state);
    }

    // 读取网格和标尺设置
    bool showGrid = settings.value("showGrid", true).toBool();
    bool showRulers = settings.value("showRulers", true).toBool();
    bool snapToGrid = settings.value("snapToGrid", true).toBool();

    ui->actionShowGrid->setChecked(showGrid);
    ui->actionShowRulers->setChecked(showRulers);
    ui->actionSnapToGrid->setChecked(snapToGrid);
}

void MainWindow::writeSettings()
{
    QSettings settings;

    // 保存窗口几何信息
    settings.setValue("geometry", saveGeometry());

    // 保存窗口状态
    settings.setValue("windowState", saveState());

    // 保存网格和标尺设置
    settings.setValue("showGrid", ui->actionShowGrid->isChecked());
    settings.setValue("showRulers", ui->actionShowRulers->isChecked());
    settings.setValue("snapToGrid", ui->actionSnapToGrid->isChecked());
}