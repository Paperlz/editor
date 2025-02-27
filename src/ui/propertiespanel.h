#ifndef PROPERTIESPANEL_H
#define PROPERTIESPANEL_H

#include <QDockWidget>
#include <QStackedWidget>
#include <QList>

class LabelDocument;
class LabelItem;
class TextItem;
class ImageItem;
class BarcodeItem;
class QRCodeItem;
class QVBoxLayout;
class QFormLayout;
class QLineEdit;
class QSpinBox;
class QDoubleSpinBox;
class QCheckBox;
class QComboBox;
class QFontComboBox;
class QColorDialog;
class QPushButton;
class QLabel;
class QGroupBox;

/**
 * @brief 属性面板类
 *
 * 显示和编辑选中元素的属性
 */
class PropertiesPanel : public QDockWidget
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父窗口
     */
    explicit PropertiesPanel(QWidget *parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~PropertiesPanel();

    /**
     * @brief 设置文档
     * @param document 标签文档
     */
    void setDocument(LabelDocument *document);

    /**
     * @brief 更新选择
     * @param items 选中的元素
     */
    void updateSelection(const QList<LabelItem*> &items);

private slots:
    /**
     * @brief 更新通用属性
     */
    void updateCommonProperties();

    /**
     * @brief 更新文本属性
     */
    void updateTextProperties();

    /**
     * @brief 更新图像属性
     */
    void updateImageProperties();

    /**
     * @brief 更新条形码属性
     */
    void updateBarcodeProperties();

    /**
     * @brief 更新二维码属性
     */
    void updateQRCodeProperties();

    /**
     * @brief 应用通用属性
     */
    void applyCommonProperties();

    /**
     * @brief 应用文本属性
     */
    void applyTextProperties();

    /**
     * @brief 应用图像属性
     */
    void applyImageProperties();

    /**
     * @brief 应用条形码属性
     */
    void applyBarcodeProperties();

    /**
     * @brief 应用二维码属性
     */
    void applyQRCodeProperties();

    /**
     * @brief 选择前景色
     */
    void selectForegroundColor();

    /**
     * @brief 选择背景色
     */
    void selectBackgroundColor();

    /**
     * @brief 选择边框色
     */
    void selectBorderColor();

    /**
     * @brief 选择图像
     */
    void selectImage();

    /**
     * @brief 重置图像
     */
    void resetImage();

    /**
     * @brief 设置字体粗体
     * @param bold 是否粗体
     */
    void setFontBold(bool bold);

    /**
     * @brief 设置字体斜体
     * @param italic 是否斜体
     */
    void setFontItalic(bool italic);

    /**
     * @brief 设置字体下划线
     * @param underline 是否下划线
     */
    void setFontUnderline(bool underline);

private:
    /**
     * @brief 创建界面
     */
    void createUI();

    /**
     * @brief 创建通用属性编辑器
     * @return 通用属性编辑器控件
     */
    QWidget* createCommonEditor();

    /**
     * @brief 创建文本属性编辑器
     * @return 文本属性编辑器控件
     */
    QWidget* createTextEditor();

    /**
     * @brief 创建图像属性编辑器
     * @return 图像属性编辑器控件
     */
    QWidget* createImageEditor();

    /**
     * @brief 创建条形码属性编辑器
     * @return 条形码属性编辑器控件
     */
    QWidget* createBarcodeEditor();

    /**
     * @brief 创建二维码属性编辑器
     * @return 二维码属性编辑器控件
     */
    QWidget* createQRCodeEditor();

    /**
     * @brief 初始化连接
     */
    void initConnections();

    /**
     * @brief 阻止信号
     *
     * 临时阻止属性编辑器的信号，避免循环更新
     *
     * @param block 是否阻止
     */
    void blockSignals(bool block);

    /**
     * @brief 更新属性编辑器
     */
    void updateEditors();

    /**
     * @brief 显示颜色对话框
     * @param currentColor 当前颜色
     * @return 选择的颜色
     */
    QColor showColorDialog(const QColor &currentColor);

private:
    LabelDocument *m_document;                 ///< 标签文档
    QList<LabelItem*> m_selectedItems;         ///< 选中的元素

    QStackedWidget *m_editorStack;            ///< 编辑器堆栈
    QWidget *m_emptyEditor;                   ///< 空编辑器
    QWidget *m_commonEditor;                  ///< 通用属性编辑器
    QWidget *m_textEditor;                    ///< 文本属性编辑器
    QWidget *m_imageEditor;                   ///< 图像属性编辑器
    QWidget *m_barcodeEditor;                 ///< 条形码属性编辑器
    QWidget *m_qrcodeEditor;                  ///< 二维码属性编辑器

    bool m_updatingUI;                        ///< 是否正在更新UI

    // 通用属性控件
    QLineEdit *m_nameEdit;                    ///< 名称编辑框
    QDoubleSpinBox *m_xPosSpinBox;            ///< X坐标微调框
    QDoubleSpinBox *m_yPosSpinBox;            ///< Y坐标微调框
    QDoubleSpinBox *m_widthSpinBox;           ///< 宽度微调框
    QDoubleSpinBox *m_heightSpinBox;          ///< 高度微调框
    QDoubleSpinBox *m_rotationSpinBox;        ///< 旋转角度微调框
    QCheckBox *m_lockedCheckBox;              ///< 锁定复选框
    QCheckBox *m_visibleCheckBox;             ///< 可见复选框

    // 文本属性控件
    QTextEdit *m_textEdit;                    ///< 文本编辑框
    QFontComboBox *m_fontComboBox;            ///< 字体下拉框
    QSpinBox *m_fontSizeSpinBox;              ///< 字体大小微调框
    QCheckBox *m_boldCheckBox;                ///< 粗体复选框
    QCheckBox *m_italicCheckBox;              ///< 斜体复选框
    QCheckBox *m_underlineCheckBox;           ///< 下划线复选框
    QPushButton *m_textColorButton;           ///< 文本颜色按钮
    QPushButton *m_backgroundColorButton;     ///< 背景颜色按钮
    QComboBox *m_alignmentComboBox;           ///< 对齐方式下拉框
    QCheckBox *m_wordWrapCheckBox;            ///< 自动换行复选框
    QSpinBox *m_borderWidthSpinBox;           ///< 边框宽度微调框
    QPushButton *m_borderColorButton;         ///< 边框颜色按钮

    // 图像属性控件
    QLabel *m_imagePathLabel;                 ///< 图像路径标签
    QPushButton *m_selectImageButton;         ///< 选择图像按钮
    QCheckBox *m_keepAspectRatioCheckBox;     ///< 保持宽高比复选框
    QSpinBox *m_imageBorderWidthSpinBox;      ///< 边框宽度微调框
    QPushButton *m_imageBorderColorButton;    ///< 边框颜色按钮
    QDoubleSpinBox *m_opacitySpinBox;         ///< 不透明度微调框
    QCheckBox *m_grayScaleCheckBox;           ///< 灰度显示复选框
    QPushButton *m_resetImageButton;          ///< 重置图像按钮

    // 条形码属性控件
    QLineEdit *m_barcodeDataEdit;             ///< 条形码数据编辑框
    QComboBox *m_barcodeTypeComboBox;         ///< 条形码类型下拉框
    QPushButton *m_barcodeForegroundButton;   ///< 前景色按钮
    QPushButton *m_barcodeBackgroundButton;   ///< 背景色按钮
    QCheckBox *m_showTextCheckBox;            ///< 显示文本复选框
    QFontComboBox *m_barcodeFontComboBox;     ///< 文本字体下拉框
    QSpinBox *m_barcodeFontSizeSpinBox;       ///< 文本字体大小微调框
    QSpinBox *m_marginSpinBox;                ///< 边距微调框
    QCheckBox *m_includeChecksumCheckBox;     ///< 包含校验和复选框

    // 二维码属性控件
    QLineEdit *m_qrcodeDataEdit;              ///< 二维码数据编辑框
    QComboBox *m_errorLevelComboBox;          ///< 错误校正级别下拉框
    QPushButton *m_qrcodeForegroundButton;    ///< 前景色按钮
    QPushButton *m_qrcodeBackgroundButton;    ///< 背景色按钮
    QSpinBox *m_qrcodeMarginSpinBox;          ///< 边距微调框
    QSpinBox *m_qrcodeSizeSpinBox;            ///< 尺寸微调框
    QCheckBox *m_quietZoneCheckBox;           ///< 安静区复选框
};

#endif // PROPERTIESPANEL_H