#include "propertiespanel.h"
#include "../models/labelmodels.h"
#include "../items/labelitem.h"
#include "../items/textitem.h"
#include "../items/imageitem.h"
#include "../items/barcodeitem.h"
#include "../items/qrcodeitem.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QComboBox>
#include <QFontComboBox>
#include <QColorDialog>
#include <QPushButton>
#include <QGroupBox>
#include <QTextEdit>
#include <QFileDialog>
#include <QStackedWidget>
#include <QScrollArea>
#include <QMessageBox>

PropertiesPanel::PropertiesPanel(QWidget *parent)
    : QDockWidget(tr("属性"), parent)
    , m_document(nullptr)
    , m_editorStack(nullptr)
    , m_emptyEditor(nullptr)
    , m_commonEditor(nullptr)
    , m_textEditor(nullptr)
    , m_imageEditor(nullptr)
    , m_barcodeEditor(nullptr)
    , m_qrcodeEditor(nullptr)
    , m_updatingUI(false)
{
    // 创建界面
    createUI();

    // 初始化连接
    initConnections();

    // 设置可停靠位置
    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
}

PropertiesPanel::~PropertiesPanel()
{
    // Qt会自动清理子控件
}

void PropertiesPanel::setDocument(LabelDocument *document)
{
    if (m_document == document) {
        return;
    }

    m_document = document;

    // 清空选择
    m_selectedItems.clear();
    updateEditors();
}

void PropertiesPanel::updateSelection(const QList<LabelItem*> &items)
{
    m_selectedItems = items;
    updateEditors();
}

void PropertiesPanel::updateCommonProperties()
{
    if (m_updatingUI || m_selectedItems.isEmpty()) {
        return;
    }

    // 阻止信号
    blockSignals(true);

    LabelItem *item = m_selectedItems.first();

    // 设置通用属性控件的值
    m_nameEdit->setText(item->name());
    m_xPosSpinBox->setValue(item->position().x());
    m_yPosSpinBox->setValue(item->position().y());
    m_widthSpinBox->setValue(item->size().width());
    m_heightSpinBox->setValue(item->size().height());
    m_rotationSpinBox->setValue(item->rotation());
    m_lockedCheckBox->setChecked(item->isLocked());
    m_visibleCheckBox->setChecked(item->isVisible());

    // 恢复信号
    blockSignals(false);
}

void PropertiesPanel::updateTextProperties()
{
    if (m_updatingUI || m_selectedItems.isEmpty()) {
        return;
    }

    TextItem *textItem = dynamic_cast<TextItem*>(m_selectedItems.first());
    if (!textItem) {
        return;
    }

    // 阻止信号
    blockSignals(true);

    // 设置文本属性控件的值
    m_textEdit->setText(textItem->text());

    QFont font = textItem->font();
    m_fontComboBox->setCurrentFont(font);
    m_fontSizeSpinBox->setValue(font.pointSize());
    m_boldCheckBox->setChecked(font.bold());
    m_italicCheckBox->setChecked(font.italic());
    m_underlineCheckBox->setChecked(font.underline());

    // 设置颜色按钮样式
    QString textColorStyle = QString("background-color: %1;").arg(textItem->textColor().name());
    m_textColorButton->setStyleSheet(textColorStyle);

    QString bgColorStyle = QString("background-color: %1;").arg(textItem->backgroundColor().name());
    m_backgroundColorButton->setStyleSheet(bgColorStyle);

    // 设置对齐方式
    int alignIndex = 0;
    Qt::Alignment alignment = textItem->alignment();
    if (alignment & Qt::AlignLeft) alignIndex = 0;
    else if (alignment & Qt::AlignHCenter) alignIndex = 1;
    else if (alignment & Qt::AlignRight) alignIndex = 2;
    else if (alignment & Qt::AlignJustify) alignIndex = 3;
    m_alignmentComboBox->setCurrentIndex(alignIndex);

    // 设置其他属性
    m_wordWrapCheckBox->setChecked(textItem->wordWrap());
    m_borderWidthSpinBox->setValue(textItem->borderWidth());

    QString borderColorStyle = QString("background-color: %1;").arg(textItem->borderColor().name());
    m_borderColorButton->setStyleSheet(borderColorStyle);

    // 恢复信号
    blockSignals(false);
}

void PropertiesPanel::updateImageProperties()
{
    if (m_updatingUI || m_selectedItems.isEmpty()) {
        return;
    }

    ImageItem *imageItem = dynamic_cast<ImageItem*>(m_selectedItems.first());
    if (!imageItem) {
        return;
    }

    // 阻止信号
    blockSignals(true);

    // 设置图像属性控件的值
    m_imagePathLabel->setText(imageItem->imagePath());
    m_keepAspectRatioCheckBox->setChecked(imageItem->keepAspectRatio());
    m_imageBorderWidthSpinBox->setValue(imageItem->borderWidth());

    QString borderColorStyle = QString("background-color: %1;").arg(imageItem->borderColor().name());
    m_imageBorderColorButton->setStyleSheet(borderColorStyle);

    m_opacitySpinBox->setValue(imageItem->opacity() * 100);
    m_grayScaleCheckBox->setChecked(imageItem->grayScale());

    // 恢复信号
    blockSignals(false);
}

void PropertiesPanel::updateBarcodeProperties()
{
    if (m_updatingUI || m_selectedItems.isEmpty()) {
        return;
    }

    BarcodeItem *barcodeItem = dynamic_cast<BarcodeItem*>(m_selectedItems.first());
    if (!barcodeItem) {
        return;
    }

    // 阻止信号
    blockSignals(true);

    // 设置条形码属性控件的值
    m_barcodeDataEdit->setText(barcodeItem->data());

    // 设置条形码类型
    QString typeName = BarcodeItem::getTypeName(barcodeItem->type());
    int typeIndex = m_barcodeTypeComboBox->findText(typeName);
    if (typeIndex >= 0) {
        m_barcodeTypeComboBox->setCurrentIndex(typeIndex);
    }

    // 设置颜色按钮样式
    QString fgColorStyle = QString("background-color: %1;").arg(barcodeItem->foregroundColor().name());
    m_barcodeForegroundButton->setStyleSheet(fgColorStyle);

    QString bgColorStyle = QString("background-color: %1;").arg(barcodeItem->backgroundColor().name());
    m_barcodeBackgroundButton->setStyleSheet(bgColorStyle);

    // 设置其他属性
    m_showTextCheckBox->setChecked(barcodeItem->showText());

    QFont font = barcodeItem->textFont();
    m_barcodeFontComboBox->setCurrentFont(font);
    m_barcodeFontSizeSpinBox->setValue(font.pointSize());

    m_marginSpinBox->setValue(barcodeItem->margin());
    m_includeChecksumCheckBox->setChecked(barcodeItem->includeChecksum());

    // 恢复信号
    blockSignals(false);
}

void PropertiesPanel::updateQRCodeProperties()
{
    if (m_updatingUI || m_selectedItems.isEmpty()) {
        return;
    }

    QRCodeItem *qrcodeItem = dynamic_cast<QRCodeItem*>(m_selectedItems.first());
    if (!qrcodeItem) {
        return;
    }

    // 阻止信号
    blockSignals(true);

    // 设置二维码属性控件的值
    m_qrcodeDataEdit->setText(qrcodeItem->data());

    // 设置错误校正级别
    QString levelName = QRCodeItem::getErrorCorrectionLevelName(qrcodeItem->errorCorrectionLevel());
    int levelIndex = m_errorLevelComboBox->findText(levelName);
    if (levelIndex >= 0) {
        m_errorLevelComboBox->setCurrentIndex(levelIndex);
    }

    // 设置颜色按钮样式
    QString fgColorStyle = QString("background-color: %1;").arg(qrcodeItem->foregroundColor().name());
    m_qrcodeForegroundButton->setStyleSheet(fgColorStyle);

    QString bgColorStyle = QString("background-color: %1;").arg(qrcodeItem->backgroundColor().name());
    m_qrcodeBackgroundButton->setStyleSheet(bgColorStyle);

    // 设置其他属性
    m_qrcodeMarginSpinBox->setValue(qrcodeItem->margin());
    m_qrcodeSizeSpinBox->setValue(qrcodeItem->size());
    m_quietZoneCheckBox->setChecked(qrcodeItem->quietZone());

    // 恢复信号
    blockSignals(false);
}

void PropertiesPanel::applyCommonProperties()
{
    if (m_updatingUI || m_selectedItems.isEmpty()) {
        return;
    }

    // 设置正在更新UI标志
    m_updatingUI = true;

    // 应用通用属性到所有选中的元素
    for (LabelItem *item : m_selectedItems) {
        // 名称
        item->setName(m_nameEdit->text());

        // 位置和大小
        item->setPosition(QPointF(m_xPosSpinBox->value(), m_yPosSpinBox->value()));
        item->setSize(QSizeF(m_widthSpinBox->value(), m_heightSpinBox->value()));

        // 旋转
        item->setRotation(m_rotationSpinBox->value());

        // 锁定状态
        item->setLocked(m_lockedCheckBox->isChecked());

        // 可见性
        item->setVisible(m_visibleCheckBox->isChecked());
    }

    // 重置正在更新UI标志
    m_updatingUI = false;
}

void PropertiesPanel::applyTextProperties()
{
    if (m_updatingUI || m_selectedItems.isEmpty()) {
        return;
    }

    TextItem *textItem = dynamic_cast<TextItem*>(m_selectedItems.first());
    if (!textItem) {
        return;
    }

    // 设置正在更新UI标志
    m_updatingUI = true;

    // 应用文本属性
    textItem->setText(m_textEdit->toPlainText());

    // 设置字体
    QFont font = m_fontComboBox->currentFont();
    font.setPointSize(m_fontSizeSpinBox->value());
    font.setBold(m_boldCheckBox->isChecked());
    font.setItalic(m_italicCheckBox->isChecked());
    font.setUnderline(m_underlineCheckBox->isChecked());
    textItem->setFont(font);

    // 设置对齐方式
    Qt::Alignment alignment;
    switch (m_alignmentComboBox->currentIndex()) {
        case 0: alignment = Qt::AlignLeft; break;
        case 1: alignment = Qt::AlignHCenter; break;
        case 2: alignment = Qt::AlignRight; break;
        case 3: alignment = Qt::AlignJustify; break;
        default: alignment = Qt::AlignLeft;
    }
    textItem->setAlignment(alignment);

    // 设置其他属性
    textItem->setWordWrap(m_wordWrapCheckBox->isChecked());
    textItem->setBorderWidth(m_borderWidthSpinBox->value());

    // 重置正在更新UI标志
    m_updatingUI = false;
}

void PropertiesPanel::applyImageProperties()
{
    if (m_updatingUI || m_selectedItems.isEmpty()) {
        return;
    }

    ImageItem *imageItem = dynamic_cast<ImageItem*>(m_selectedItems.first());
    if (!imageItem) {
        return;
    }

    // 设置正在更新UI标志
    m_updatingUI = true;

    // 应用图像属性
    imageItem->setKeepAspectRatio(m_keepAspectRatioCheckBox->isChecked());
    imageItem->setBorderWidth(m_imageBorderWidthSpinBox->value());
    imageItem->setOpacity(m_opacitySpinBox->value() / 100.0);
    imageItem->setGrayScale(m_grayScaleCheckBox->isChecked());

    // 重置正在更新UI标志
    m_updatingUI = false;
}

void PropertiesPanel::applyBarcodeProperties()
{
    if (m_updatingUI || m_selectedItems.isEmpty()) {
        return;
    }

    BarcodeItem *barcodeItem = dynamic_cast<BarcodeItem*>(m_selectedItems.first());
    if (!barcodeItem) {
        return;
    }

    // 设置正在更新UI标志
    m_updatingUI = true;

    // 应用条形码属性
    barcodeItem->setData(m_barcodeDataEdit->text());

    // 设置条形码类型
    QString typeName = m_barcodeTypeComboBox->currentText();
    BarcodeType type = BarcodeItem::getTypeFromName(typeName);
    barcodeItem->setType(type);

    // 设置其他属性
    barcodeItem->setShowText(m_showTextCheckBox->isChecked());

    QFont font = m_barcodeFontComboBox->currentFont();
    font.setPointSize(m_barcodeFontSizeSpinBox->value());
    barcodeItem->setTextFont(font);

    barcodeItem->setMargin(m_marginSpinBox->value());
    barcodeItem->setIncludeChecksum(m_includeChecksumCheckBox->isChecked());

    // 重置正在更新UI标志
    m_updatingUI = false;
}

void PropertiesPanel::applyQRCodeProperties()
{
    if (m_updatingUI || m_selectedItems.isEmpty()) {
        return;
    }

    QRCodeItem *qrcodeItem = dynamic_cast<QRCodeItem*>(m_selectedItems.first());
    if (!qrcodeItem) {
        return;
    }

    // 设置正在更新UI标志
    m_updatingUI = true;

    // 应用二维码属性
    qrcodeItem->setData(m_qrcodeDataEdit->text());

    // 设置错误校正级别
    QString levelName = m_errorLevelComboBox->currentText();
    QRErrorCorrectionLevel level = QRCodeItem::getErrorCorrectionLevelFromName(levelName);
    qrcodeItem->setErrorCorrectionLevel(level);

    // 设置其他属性
    qrcodeItem->setMargin(m_qrcodeMarginSpinBox->value());
    qrcodeItem->setSize(m_qrcodeSizeSpinBox->value());
    qrcodeItem->setQuietZone(m_quietZoneCheckBox->isChecked());

    // 重置正在更新UI标志
    m_updatingUI = false;
}

void PropertiesPanel::selectForegroundColor()
{
    if (m_selectedItems.isEmpty()) {
        return;
    }

    QPushButton *button = qobject_cast<QPushButton*>(sender());
    if (!button) {
        return;
    }

    // 获取当前颜色
    QColor currentColor;
    if (button == m_textColorButton) {
        TextItem *textItem = dynamic_cast<TextItem*>(m_selectedItems.first());
        if (textItem) {
            currentColor = textItem->textColor();
        }
    } else if (button == m_barcodeForegroundButton) {
        BarcodeItem *barcodeItem = dynamic_cast<BarcodeItem*>(m_selectedItems.first());
        if (barcodeItem) {
            currentColor = barcodeItem->foregroundColor();
        }
    } else if (button == m_qrcodeForegroundButton) {
        QRCodeItem *qrcodeItem = dynamic_cast<QRCodeItem*>(m_selectedItems.first());
        if (qrcodeItem) {
            currentColor = qrcodeItem->foregroundColor();
        }
    }

    // 显示颜色对话框
    QColor color = showColorDialog(currentColor);
    if (!color.isValid()) {
        return;
    }

    // 应用颜色
    if (button == m_textColorButton) {
        TextItem *textItem = dynamic_cast<TextItem*>(m_selectedItems.first());
        if (textItem) {
            textItem->setTextColor(color);
            QString colorStyle = QString("background-color: %1;").arg(color.name());
            button->setStyleSheet(colorStyle);
        }
    } else if (button == m_barcodeForegroundButton) {
        BarcodeItem *barcodeItem = dynamic_cast<BarcodeItem*>(m_selectedItems.first());
        if (barcodeItem) {
            barcodeItem->setForegroundColor(color);
            QString colorStyle = QString("background-color: %1;").arg(color.name());
            button->setStyleSheet(colorStyle);
        }
    } else if (button == m_qrcodeForegroundButton) {
        QRCodeItem *qrcodeItem = dynamic_cast<QRCodeItem*>(m_selectedItems.first());
        if (qrcodeItem) {
            qrcodeItem->setForegroundColor(color);
            QString colorStyle = QString("background-color: %1;").arg(color.name());
            button->setStyleSheet(colorStyle);
        }
    }
}

void PropertiesPanel::selectBackgroundColor()
{
    if (m_selectedItems.isEmpty()) {
        return;
    }

    QPushButton *button = qobject_cast<QPushButton*>(sender());
    if (!button) {
        return;
    }

    // 获取当前颜色
    QColor currentColor;
    if (button == m_backgroundColorButton) {
        TextItem *textItem = dynamic_cast<TextItem*>(m_selectedItems.first());
        if (textItem) {
            currentColor = textItem->backgroundColor();
        }
    } else if (button == m_barcodeBackgroundButton) {
        BarcodeItem *barcodeItem = dynamic_cast<BarcodeItem*>(m_selectedItems.first());
        if (barcodeItem) {
            currentColor = barcodeItem->backgroundColor();
        }
    } else if (button == m_qrcodeBackgroundButton) {
        QRCodeItem *qrcodeItem = dynamic_cast<QRCodeItem*>(m_selectedItems.first());
        if (qrcodeItem) {
            currentColor = qrcodeItem->backgroundColor();
        }
    }

    // 显示颜色对话框
    QColor color = showColorDialog(currentColor);
    if (!color.isValid()) {
        return;
    }

    // 应用颜色
    if (button == m_backgroundColorButton) {
        TextItem *textItem = dynamic_cast<TextItem*>(m_selectedItems.first());
        if (textItem) {
            textItem->setBackgroundColor(color);
            QString colorStyle = QString("background-color: %1;").arg(color.name());
            button->setStyleSheet(colorStyle);
        }
    } else if (button == m_barcodeBackgroundButton) {
        BarcodeItem *barcodeItem = dynamic_cast<BarcodeItem*>(m_selectedItems.first());
        if (barcodeItem) {
            barcodeItem->setBackgroundColor(color);
            QString colorStyle = QString("background-color: %1;").arg(color.name());
            button->setStyleSheet(colorStyle);
        }
    } else if (button == m_qrcodeBackgroundButton) {
        QRCodeItem *qrcodeItem = dynamic_cast<QRCodeItem*>(m_selectedItems.first());
        if (qrcodeItem) {
            qrcodeItem->setBackgroundColor(color);
            QString colorStyle = QString("background-color: %1;").arg(color.name());
            button->setStyleSheet(colorStyle);
        }
    }
}

void PropertiesPanel::selectBorderColor()
{
    if (m_selectedItems.isEmpty()) {
        return;
    }

    QPushButton *button = qobject_cast<QPushButton*>(sender());
    if (!button) {
        return;
    }

    // 获取当前颜色
    QColor currentColor;
    if (button == m_borderColorButton) {
        TextItem *textItem = dynamic_cast<TextItem*>(m_selectedItems.first());
        if (textItem) {
            currentColor = textItem->borderColor();
        }
    } else if (button == m_imageBorderColorButton) {
        ImageItem *imageItem = dynamic_cast<ImageItem*>(m_selectedItems.first());
        if (imageItem) {
            currentColor = imageItem->borderColor();
        }
    }

    // 显示颜色对话框
    QColor color = showColorDialog(currentColor);
    if (!color.isValid()) {
        return;
    }

    // 应用颜色
    if (button == m_borderColorButton) {
        TextItem *textItem = dynamic_cast<TextItem*>(m_selectedItems.first());
        if (textItem) {
            textItem->setBorderColor(color);
            QString colorStyle = QString("background-color: %1;").arg(color.name());
            button->setStyleSheet(colorStyle);
        }
    } else if (button == m_imageBorderColorButton) {
        ImageItem *imageItem = dynamic_cast<ImageItem*>(m_selectedItems.first());
        if (imageItem) {
            imageItem->setBorderColor(color);
            QString colorStyle = QString("background-color: %1;").arg(color.name());
            button->setStyleSheet(colorStyle);
        }
    }
}

void PropertiesPanel::selectImage()
{
    if (m_selectedItems.isEmpty()) {
        return;
    }

    ImageItem *imageItem = dynamic_cast<ImageItem*>(m_selectedItems.first());
    if (!imageItem) {
        return;
    }

    // 显示文件对话框
    QString path = QFileDialog::getOpenFileName(this, tr("选择图像"),
        QDir::homePath(), tr("图像文件 (*.png *.jpg *.jpeg *.bmp *.gif)"));

    if (path.isEmpty()) {
        return;
    }

    // 应用图像
    if (imageItem->setImagePath(path)) {
        m_imagePathLabel->setText(path);
    } else {
        QMessageBox::warning(this, tr("图像加载失败"), tr("无法加载所选图像文件。"));
    }
}

void PropertiesPanel::resetImage()
{
    if (m_selectedItems.isEmpty()) {
        return;
    }

    ImageItem *imageItem = dynamic_cast<ImageItem*>(m_selectedItems.first());
    if (!imageItem) {
        return;
    }

    // 确认重置
    QMessageBox::StandardButton reply = QMessageBox::question(this, tr("重置图像"),
        tr("确定要重置图像吗？这将删除所有的图像处理效果。"),
        QMessageBox::Yes | QMessageBox::No);

    if (reply != QMessageBox::Yes) {
        return;
    }

    // 重置图像
    imageItem->resetImage();
}

void PropertiesPanel::setFontBold(bool bold)
{
    if (m_updatingUI || m_selectedItems.isEmpty()) {
        return;
    }

    TextItem *textItem = dynamic_cast<TextItem*>(m_selectedItems.first());
    if (!textItem) {
        return;
    }

    // 设置正在更新UI标志
    m_updatingUI = true;

    // 应用粗体
    QFont font = textItem->font();
    font.setBold(bold);
    textItem->setFont(font);

    // 重置正在更新UI标志
    m_updatingUI = false;
}

void PropertiesPanel::setFontItalic(bool italic)
{
    if (m_updatingUI || m_selectedItems.isEmpty()) {
        return;
    }

    TextItem *textItem = dynamic_cast<TextItem*>(m_selectedItems.first());
    if (!textItem) {
        return;
    }

    // 设置正在更新UI标志
    m_updatingUI = true;

    // 应用斜体
    QFont font = textItem->font();
    font.setItalic(italic);
    textItem->setFont(font);

    // 重置正在更新UI标志
    m_updatingUI = false;
}

void PropertiesPanel::setFontUnderline(bool underline)
{
    if (m_updatingUI || m_selectedItems.isEmpty()) {
        return;
    }

    TextItem *textItem = dynamic_cast<TextItem*>(m_selectedItems.first());
    if (!textItem) {
        return;
    }

    // 设置正在更新UI标志
    m_updatingUI = true;

    // 应用下划线
    QFont font = textItem->font();
    font.setUnderline(underline);
    textItem->setFont(font);

    // 重置正在更新UI标志
    m_updatingUI = false;
}

void PropertiesPanel::createUI()
{
    // 创建主布局
    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // 创建编辑器栈
    m_editorStack = new QStackedWidget();

    // 创建空编辑器
    m_emptyEditor = new QWidget();
    QVBoxLayout *emptyLayout = new QVBoxLayout(m_emptyEditor);
    emptyLayout->addWidget(new QLabel(tr("请选择一个元素以编辑其属性")));
    emptyLayout->addStretch();

    // 创建各种类型的编辑器
    m_commonEditor = createCommonEditor();
    m_textEditor = createTextEditor();
    m_imageEditor = createImageEditor();
    m_barcodeEditor = createBarcodeEditor();
    m_qrcodeEditor = createQRCodeEditor();

    // 添加到栈
    m_editorStack->addWidget(m_emptyEditor);
    m_editorStack->addWidget(m_commonEditor);
    m_editorStack->addWidget(m_textEditor);
    m_editorStack->addWidget(m_imageEditor);
    m_editorStack->addWidget(m_barcodeEditor);
    m_editorStack->addWidget(m_qrcodeEditor);

    // 创建滚动区域
    QScrollArea *scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setWidget(m_editorStack);

    // 添加到主布局
    mainLayout->addWidget(scrollArea);

    // 创建中心小部件
    QWidget *centralWidget = new QWidget();
    centralWidget->setLayout(mainLayout);

    // 设置中心小部件
    setWidget(centralWidget);

    // 显示空编辑器
    m_editorStack->setCurrentWidget(m_emptyEditor);
}

QWidget* PropertiesPanel::createCommonEditor()
{
    QWidget *editor = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(editor);

    // 通用属性组
    QGroupBox *commonGroup = new QGroupBox(tr("通用属性"));
    QFormLayout *commonLayout = new QFormLayout(commonGroup);

    // 名称
    m_nameEdit = new QLineEdit();
    commonLayout->addRow(tr("名称:"), m_nameEdit);

    // 位置
    QHBoxLayout *posLayout = new QHBoxLayout();
    m_xPosSpinBox = new QDoubleSpinBox();
    m_xPosSpinBox->setRange(0, 10000);
    m_xPosSpinBox->setSuffix(" mm");
    m_yPosSpinBox = new QDoubleSpinBox();
    m_yPosSpinBox->setRange(0, 10000);
    m_yPosSpinBox->setSuffix(" mm");
    posLayout->addWidget(m_xPosSpinBox);
    posLayout->addWidget(m_yPosSpinBox);
    commonLayout->addRow(tr("位置 (X,Y):"), posLayout);

    // 大小
    QHBoxLayout *sizeLayout = new QHBoxLayout();
    m_widthSpinBox = new QDoubleSpinBox();
    m_widthSpinBox->setRange(1, 10000);
    m_widthSpinBox->setSuffix(" mm");
    m_heightSpinBox = new QDoubleSpinBox();
    m_heightSpinBox->setRange(1, 10000);
    m_heightSpinBox->setSuffix(" mm");
    sizeLayout->addWidget(m_widthSpinBox);
    sizeLayout->addWidget(m_heightSpinBox);
    commonLayout->addRow(tr("大小 (宽,高):"), sizeLayout);

    // 旋转
    m_rotationSpinBox = new QDoubleSpinBox();
    m_rotationSpinBox->setRange(0, 359.99);
    m_rotationSpinBox->setSuffix(" °");
    commonLayout->addRow(tr("旋转:"), m_rotationSpinBox);

    // 锁定状态
    m_lockedCheckBox = new QCheckBox(tr("锁定"));
    commonLayout->addRow("", m_lockedCheckBox);

    // 可见性
    m_visibleCheckBox = new QCheckBox(tr("可见"));
    commonLayout->addRow("", m_visibleCheckBox);

    // 添加到主布局
    layout->addWidget(commonGroup);
    layout->addStretch();

    return editor;
}

QWidget* PropertiesPanel::createTextEditor()
{
    QWidget *editor = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(editor);

    // 添加通用属性
    layout->addWidget(createCommonEditor());

    // 文本属性组
    QGroupBox *textGroup = new QGroupBox(tr("文本属性"));
    QFormLayout *textLayout = new QFormLayout(textGroup);

    // 文本内容
    m_textEdit = new QTextEdit();
    m_textEdit->setMaximumHeight(100);
    textLayout->addRow(tr("文本:"), m_textEdit);

    // 字体
    QHBoxLayout *fontLayout = new QHBoxLayout();
    m_fontComboBox = new QFontComboBox();
    m_fontSizeSpinBox = new QSpinBox();
    m_fontSizeSpinBox->setRange(1, 100);
    fontLayout->addWidget(m_fontComboBox);
    fontLayout->addWidget(m_fontSizeSpinBox);
    textLayout->addRow(tr("字体:"), fontLayout);

    // 字体样式
    QHBoxLayout *styleLayout = new QHBoxLayout();
    m_boldCheckBox = new QCheckBox(tr("粗体"));
    m_italicCheckBox = new QCheckBox(tr("斜体"));
    m_underlineCheckBox = new QCheckBox(tr("下划线"));
    styleLayout->addWidget(m_boldCheckBox);
    styleLayout->addWidget(m_italicCheckBox);
    styleLayout->addWidget(m_underlineCheckBox);
    textLayout->addRow(tr("样式:"), styleLayout);

    // 颜色
    QHBoxLayout *colorLayout = new QHBoxLayout();
    m_textColorButton = new QPushButton(tr("文本颜色"));
    m_backgroundColorButton = new QPushButton(tr("背景颜色"));
    colorLayout->addWidget(m_textColorButton);
    colorLayout->addWidget(m_backgroundColorButton);
    textLayout->addRow(tr("颜色:"), colorLayout);

    // 对齐方式
    m_alignmentComboBox = new QComboBox();
    m_alignmentComboBox->addItem(tr("左对齐"));
    m_alignmentComboBox->addItem(tr("居中"));
    m_alignmentComboBox->addItem(tr("右对齐"));
    m_alignmentComboBox->addItem(tr("两端对齐"));
    textLayout->addRow(tr("对齐:"), m_alignmentComboBox);

    // 自动换行
    m_wordWrapCheckBox = new QCheckBox(tr("自动换行"));
    textLayout->addRow("", m_wordWrapCheckBox);

    // 边框
    QHBoxLayout *borderLayout = new QHBoxLayout();
    m_borderWidthSpinBox = new QSpinBox();
    m_borderWidthSpinBox->setRange(0, 10);
    m_borderWidthSpinBox->setSuffix(" px");
    m_borderColorButton = new QPushButton(tr("边框颜色"));
    borderLayout->addWidget(m_borderWidthSpinBox);
    borderLayout->addWidget(m_borderColorButton);
    textLayout->addRow(tr("边框:"), borderLayout);

    // 添加到主布局
    layout->addWidget(textGroup);
    layout->addStretch();

    return editor;
}

QWidget* PropertiesPanel::createImageEditor()
{
    QWidget *editor = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(editor);

    // 添加通用属性
    layout->addWidget(createCommonEditor());

    // 图像属性组
    QGroupBox *imageGroup = new QGroupBox(tr("图像属性"));
    QFormLayout *imageLayout = new QFormLayout(imageGroup);

    // 图像路径
    QHBoxLayout *pathLayout = new QHBoxLayout();
    m_imagePathLabel = new QLabel();
    m_imagePathLabel->setWordWrap(true);
    m_selectImageButton = new QPushButton(tr("选择..."));
    pathLayout->addWidget(m_imagePathLabel);
    pathLayout->addWidget(m_selectImageButton);
    imageLayout->addRow(tr("图像:"), pathLayout);

    // 保持宽高比
    m_keepAspectRatioCheckBox = new QCheckBox(tr("保持宽高比"));
    imageLayout->addRow("", m_keepAspectRatioCheckBox);

    // 边框
    QHBoxLayout *borderLayout = new QHBoxLayout();
    m_imageBorderWidthSpinBox = new QSpinBox();
    m_imageBorderWidthSpinBox->setRange(0, 10);
    m_imageBorderWidthSpinBox->setSuffix(" px");
    m_imageBorderColorButton = new QPushButton(tr("边框颜色"));
    borderLayout->addWidget(m_imageBorderWidthSpinBox);
    borderLayout->addWidget(m_imageBorderColorButton);
    imageLayout->addRow(tr("边框:"), borderLayout);

    // 不透明度
    m_opacitySpinBox = new QDoubleSpinBox();
    m_opacitySpinBox->setRange(0, 100);
    m_opacitySpinBox->setSuffix(" %");
    imageLayout->addRow(tr("不透明度:"), m_opacitySpinBox);

    // 灰度显示
    m_grayScaleCheckBox = new QCheckBox(tr("灰度显示"));
    imageLayout->addRow("", m_grayScaleCheckBox);

    // 重置按钮
    m_resetImageButton = new QPushButton(tr("重置图像"));
    imageLayout->addRow("", m_resetImageButton);

    // 添加到主布局
    layout->addWidget(imageGroup);
    layout->addStretch();

    return editor;
}

QWidget* PropertiesPanel::createBarcodeEditor()
{
    QWidget *editor = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(editor);

    // 添加通用属性
    layout->addWidget(createCommonEditor());

    // 条形码属性组
    QGroupBox *barcodeGroup = new QGroupBox(tr("条形码属性"));
    QFormLayout *barcodeLayout = new QFormLayout(barcodeGroup);

    // 条形码数据
    m_barcodeDataEdit = new QLineEdit();
    barcodeLayout->addRow(tr("数据:"), m_barcodeDataEdit);

    // 条形码类型
    m_barcodeTypeComboBox = new QComboBox();
    m_barcodeTypeComboBox->addItem("Code 128");
    m_barcodeTypeComboBox->addItem("Code 39");
    m_barcodeTypeComboBox->addItem("Code 93");
    m_barcodeTypeComboBox->addItem("EAN-8");
    m_barcodeTypeComboBox->addItem("EAN-13");
    m_barcodeTypeComboBox->addItem("UPC-A");
    m_barcodeTypeComboBox->addItem("UPC-E");
    m_barcodeTypeComboBox->addItem("MSI");
    m_barcodeTypeComboBox->addItem("Interleaved 2 of 5");
    m_barcodeTypeComboBox->addItem("ITF-14");
    m_barcodeTypeComboBox->addItem("Codabar");
    barcodeLayout->addRow(tr("类型:"), m_barcodeTypeComboBox);

    // 颜色
    QHBoxLayout *colorLayout = new QHBoxLayout();
    m_barcodeForegroundButton = new QPushButton(tr("前景色"));
    m_barcodeBackgroundButton = new QPushButton(tr("背景色"));
    colorLayout->addWidget(m_barcodeForegroundButton);
    colorLayout->addWidget(m_barcodeBackgroundButton);
    barcodeLayout->addRow(tr("颜色:"), colorLayout);

    // 显示文本
    m_showTextCheckBox = new QCheckBox(tr("显示文本"));
    barcodeLayout->addRow("", m_showTextCheckBox);

    // 文本字体
    QHBoxLayout *fontLayout = new QHBoxLayout();
    m_barcodeFontComboBox = new QFontComboBox();
    m_barcodeFontSizeSpinBox = new QSpinBox();
    m_barcodeFontSizeSpinBox->setRange(1, 20);
    fontLayout->addWidget(m_barcodeFontComboBox);
    fontLayout->addWidget(m_barcodeFontSizeSpinBox);
    barcodeLayout->addRow(tr("文本字体:"), fontLayout);

    // 边距
    m_marginSpinBox = new QSpinBox();
    m_marginSpinBox->setRange(0, 50);
    m_marginSpinBox->setSuffix(" px");
    barcodeLayout->addRow(tr("边距:"), m_marginSpinBox);

    // 包含校验和
    m_includeChecksumCheckBox = new QCheckBox(tr("包含校验和"));
    barcodeLayout->addRow("", m_includeChecksumCheckBox);

    // 添加到主布局
    layout->addWidget(barcodeGroup);
    layout->addStretch();

    return editor;
}

QWidget* PropertiesPanel::createQRCodeEditor()
{
    QWidget *editor = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(editor);

    // 添加通用属性
    layout->addWidget(createCommonEditor());

    // 二维码属性组
    QGroupBox *qrcodeGroup = new QGroupBox(tr("二维码属性"));
    QFormLayout *qrcodeLayout = new QFormLayout(qrcodeGroup);

    // 二维码数据
    m_qrcodeDataEdit = new QLineEdit();
    qrcodeLayout->addRow(tr("数据:"), m_qrcodeDataEdit);

    // 错误校正级别
    m_errorLevelComboBox = new QComboBox();
    m_errorLevelComboBox->addItem("Low");
    m_errorLevelComboBox->addItem("Medium");
    m_errorLevelComboBox->addItem("Quartile");
    m_errorLevelComboBox->addItem("High");
    qrcodeLayout->addRow(tr("错误校正级别:"), m_errorLevelComboBox);

    // 颜色
    QHBoxLayout *colorLayout = new QHBoxLayout();
    m_qrcodeForegroundButton = new QPushButton(tr("前景色"));
    m_qrcodeBackgroundButton = new QPushButton(tr("背景色"));
    colorLayout->addWidget(m_qrcodeForegroundButton);
    colorLayout->addWidget(m_qrcodeBackgroundButton);
    qrcodeLayout->addRow(tr("颜色:"), colorLayout);

    // 边距
    m_qrcodeMarginSpinBox = new QSpinBox();
    m_qrcodeMarginSpinBox->setRange(0, 50);
    m_qrcodeMarginSpinBox->setSuffix(" px");
    qrcodeLayout->addRow(tr("边距:"), m_qrcodeMarginSpinBox);

    // 尺寸
    m_qrcodeSizeSpinBox = new QSpinBox();
    m_qrcodeSizeSpinBox->setRange(100, 1000);
    m_qrcodeSizeSpinBox->setSuffix(" px");
    qrcodeLayout->addRow(tr("尺寸:"), m_qrcodeSizeSpinBox);

    // 安静区
    m_quietZoneCheckBox = new QCheckBox(tr("包含安静区"));
    qrcodeLayout->addRow("", m_quietZoneCheckBox);

    // 添加到主布局
    layout->addWidget(qrcodeGroup);
    layout->addStretch();

    return editor;
}

void PropertiesPanel::initConnections()
{
    // 通用属性连接
    connect(m_nameEdit, &QLineEdit::editingFinished, this, &PropertiesPanel::applyCommonProperties);
    connect(m_xPosSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &PropertiesPanel::applyCommonProperties);
    connect(m_yPosSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &PropertiesPanel::applyCommonProperties);
    connect(m_widthSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &PropertiesPanel::applyCommonProperties);
    connect(m_heightSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &PropertiesPanel::applyCommonProperties);
    connect(m_rotationSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &PropertiesPanel::applyCommonProperties);
    connect(m_lockedCheckBox, &QCheckBox::toggled, this, &PropertiesPanel::applyCommonProperties);
    connect(m_visibleCheckBox, &QCheckBox::toggled, this, &PropertiesPanel::applyCommonProperties);

    // 文本属性连接
    connect(m_textEdit, &QTextEdit::textChanged, this, &PropertiesPanel::applyTextProperties);
    connect(m_fontComboBox, &QFontComboBox::currentFontChanged, this, &PropertiesPanel::applyTextProperties);
    connect(m_fontSizeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &PropertiesPanel::applyTextProperties);
    connect(m_boldCheckBox, &QCheckBox::toggled, this, &PropertiesPanel::setFontBold);
    connect(m_italicCheckBox, &QCheckBox::toggled, this, &PropertiesPanel::setFontItalic);
    connect(m_underlineCheckBox, &QCheckBox::toggled, this, &PropertiesPanel::setFontUnderline);
    connect(m_textColorButton, &QPushButton::clicked, this, &PropertiesPanel::selectForegroundColor);
    connect(m_backgroundColorButton, &QPushButton::clicked, this, &PropertiesPanel::selectBackgroundColor);
    connect(m_alignmentComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &PropertiesPanel::applyTextProperties);
    connect(m_wordWrapCheckBox, &QCheckBox::toggled, this, &PropertiesPanel::applyTextProperties);
    connect(m_borderWidthSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &PropertiesPanel::applyTextProperties);
    connect(m_borderColorButton, &QPushButton::clicked, this, &PropertiesPanel::selectBorderColor);

    // 图像属性连接
    connect(m_selectImageButton, &QPushButton::clicked, this, &PropertiesPanel::selectImage);
    connect(m_keepAspectRatioCheckBox, &QCheckBox::toggled, this, &PropertiesPanel::applyImageProperties);
    connect(m_imageBorderWidthSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &PropertiesPanel::applyImageProperties);
    connect(m_imageBorderColorButton, &QPushButton::clicked, this, &PropertiesPanel::selectBorderColor);
    connect(m_opacitySpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &PropertiesPanel::applyImageProperties);
    connect(m_grayScaleCheckBox, &QCheckBox::toggled, this, &PropertiesPanel::applyImageProperties);
    connect(m_resetImageButton, &QPushButton::clicked, this, &PropertiesPanel::resetImage);

    // 条形码属性连接
    connect(m_barcodeDataEdit, &QLineEdit::editingFinished, this, &PropertiesPanel::applyBarcodeProperties);
    connect(m_barcodeTypeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &PropertiesPanel::applyBarcodeProperties);
    connect(m_barcodeForegroundButton, &QPushButton::clicked, this, &PropertiesPanel::selectForegroundColor);
    connect(m_barcodeBackgroundButton, &QPushButton::clicked, this, &PropertiesPanel::selectBackgroundColor);
    connect(m_showTextCheckBox, &QCheckBox::toggled, this, &PropertiesPanel::applyBarcodeProperties);
    connect(m_barcodeFontComboBox, &QFontComboBox::currentFontChanged, this, &PropertiesPanel::applyBarcodeProperties);
    connect(m_barcodeFontSizeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &PropertiesPanel::applyBarcodeProperties);
    connect(m_marginSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &PropertiesPanel::applyBarcodeProperties);
    connect(m_includeChecksumCheckBox, &QCheckBox::toggled, this, &PropertiesPanel::applyBarcodeProperties);

    // 二维码属性连接
    connect(m_qrcodeDataEdit, &QLineEdit::editingFinished, this, &PropertiesPanel::applyQRCodeProperties);
    connect(m_errorLevelComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &PropertiesPanel::applyQRCodeProperties);
    connect(m_qrcodeForegroundButton, &QPushButton::clicked, this, &PropertiesPanel::selectForegroundColor);
    connect(m_qrcodeBackgroundButton, &QPushButton::clicked, this, &PropertiesPanel::selectBackgroundColor);
    connect(m_qrcodeMarginSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &PropertiesPanel::applyQRCodeProperties);
    connect(m_qrcodeSizeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &PropertiesPanel::applyQRCodeProperties);
    connect(m_quietZoneCheckBox, &QCheckBox::toggled, this, &PropertiesPanel::applyQRCodeProperties);
}

void PropertiesPanel::blockSignals(bool block)
{
    // 通用属性控件
    m_nameEdit->blockSignals(block);
    m_xPosSpinBox->blockSignals(block);
    m_yPosSpinBox->blockSignals(block);
    m_widthSpinBox->blockSignals(block);
    m_heightSpinBox->blockSignals(block);
    m_rotationSpinBox->blockSignals(block);
    m_lockedCheckBox->blockSignals(block);
    m_visibleCheckBox->blockSignals(block);

    // 文本属性控件
    if (m_textEdit) {
        m_textEdit->blockSignals(block);
        m_fontComboBox->blockSignals(block);
        m_fontSizeSpinBox->blockSignals(block);
        m_boldCheckBox->blockSignals(block);
        m_italicCheckBox->blockSignals(block);
        m_underlineCheckBox->blockSignals(block);
        m_alignmentComboBox->blockSignals(block);
        m_wordWrapCheckBox->blockSignals(block);
        m_borderWidthSpinBox->blockSignals(block);
    }

    // 图像属性控件
    if (m_keepAspectRatioCheckBox) {
        m_keepAspectRatioCheckBox->blockSignals(block);
        m_imageBorderWidthSpinBox->blockSignals(block);
        m_opacitySpinBox->blockSignals(block);
        m_grayScaleCheckBox->blockSignals(block);
    }

    // 条形码属性控件
    if (m_barcodeDataEdit) {
        m_barcodeDataEdit->blockSignals(block);
        m_barcodeTypeComboBox->blockSignals(block);
        m_showTextCheckBox->blockSignals(block);
        m_barcodeFontComboBox->blockSignals(block);
        m_barcodeFontSizeSpinBox->blockSignals(block);
        m_marginSpinBox->blockSignals(block);
        m_includeChecksumCheckBox->blockSignals(block);
    }

    // 二维码属性控件
    if (m_qrcodeDataEdit) {
        m_qrcodeDataEdit->blockSignals(block);
        m_errorLevelComboBox->blockSignals(block);
        m_qrcodeMarginSpinBox->blockSignals(block);
        m_qrcodeSizeSpinBox->blockSignals(block);
        m_quietZoneCheckBox->blockSignals(block);
    }
}

void PropertiesPanel::updateEditors()
{
    if (m_selectedItems.isEmpty()) {
        // 显示空编辑器
        m_editorStack->setCurrentWidget(m_emptyEditor);
        return;
    }

    // 如果选中了多个元素，只显示通用属性
    if (m_selectedItems.size() > 1) {
        m_editorStack->setCurrentWidget(m_commonEditor);
        updateCommonProperties();
        return;
    }

    // 根据元素类型显示对应的编辑器
    LabelItem *item = m_selectedItems.first();

    if (dynamic_cast<TextItem*>(item)) {
        m_editorStack->setCurrentWidget(m_textEditor);
        updateCommonProperties();
        updateTextProperties();
    } else if (dynamic_cast<ImageItem*>(item)) {
        m_editorStack->setCurrentWidget(m_imageEditor);
        updateCommonProperties();
        updateImageProperties();
    } else if (dynamic_cast<BarcodeItem*>(item)) {
        m_editorStack->setCurrentWidget(m_barcodeEditor);
        updateCommonProperties();
        updateBarcodeProperties();
    } else if (dynamic_cast<QRCodeItem*>(item)) {
        m_editorStack->setCurrentWidget(m_qrcodeEditor);
        updateCommonProperties();
        updateQRCodeProperties();
    } else {
        m_editorStack->setCurrentWidget(m_commonEditor);
        updateCommonProperties();
    }
}

QColor PropertiesPanel::showColorDialog(const QColor &currentColor)
{
    return QColorDialog::getColor(currentColor, this, tr("选择颜色"), QColorDialog::ShowAlphaChannel);
}