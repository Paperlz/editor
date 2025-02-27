cmake_minimum_required(VERSION 3.15)
project(printer VERSION 1.0.0 LANGUAGES CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 开启自动处理Qt特性的CMake选项
set(CMAKE_AUTOMOC ON) # 自动处理Q_OBJECT宏
set(CMAKE_AUTOUIC ON) # 自动处理.ui文件
set(CMAKE_AUTORCC ON) # 自动处理.qrc文件

# 查找Qt包
find_package(Qt6 COMPONENTS Core Gui Widgets PrintSupport Xml REQUIRED)
if (NOT Qt6_FOUND)
    find_package(Qt5 5.15 COMPONENTS Core Gui Widgets PrintSupport Xml REQUIRED)
endif()

# 项目源文件
set(PROJECT_SOURCES
        src/main.cpp
        src/application.cpp
        src/mainwindow.cpp

        # 标签元素类
        src/items/labelitem.cpp
        src/items/textitem.cpp
        src/items/imageitem.cpp
        src/items/barcodeitem.cpp
        src/items/qrcodeitem.cpp

        # 数据模型
        src/models/labelmodels.cpp

        # UI类
        src/ui/labeleditview.cpp
        src/ui/propertiespanel.cpp
        src/mainwindow.cpp
        src/mainwindow.h
        src/application.cpp
        src/application.h
        src/items/labelitem.cpp
        src/items/labelitem.h
        src/items/textitem.cpp
        src/items/textitem.h
        src/items/imageitem.cpp
        src/items/imageitem.h
        src/items/barcodeitem.cpp
        src/items/barcodeitem.h
        src/models/labelmodels.cpp
        src/models/labelmodels.h
        src/items/qrcodeitem.cpp
        src/items/qrcodeitem.h
        src/ui/labeleditview.cpp
        src/ui/labeleditview.h
        src/ui/propertiespanel.cpp
        src/ui/propertiespanel.h
)

# 项目头文件
set(PROJECT_HEADERS
        src/application.h
        src/mainwindow.h

        # 标签元素类
        src/items/labelitem.h
        src/items/textitem.h
        src/items/imageitem.h
        src/items/barcodeitem.h
        src/items/qrcodeitem.h

        # 数据模型
        src/models/labelmodels.h

        # UI类
        src/ui/labeleditview.h
        src/ui/propertiespanel.h
        src/mainwindow.cpp
        src/mainwindow.h
        src/application.cpp
        src/application.h
        src/items/labelitem.cpp
        src/items/labelitem.h
        src/items/textitem.cpp
        src/items/textitem.h
        src/items/imageitem.cpp
        src/items/imageitem.h
        src/items/barcodeitem.cpp
        src/items/barcodeitem.h
        src/models/labelmodels.cpp
        src/models/labelmodels.h
        src/items/qrcodeitem.cpp
        src/items/qrcodeitem.h
        src/ui/labeleditview.cpp
        src/ui/labeleditview.h
        src/ui/propertiespanel.cpp
        src/ui/propertiespanel.h
)

# 项目UI文件
set(PROJECT_FORMS
        forms/mainwindow.ui
        forms/propertiespanel.ui
        src/mainwindow.cpp
        src/mainwindow.h
        src/application.cpp
        src/application.h
        src/items/labelitem.cpp
        src/items/labelitem.h
        src/items/textitem.cpp
        src/items/textitem.h
        src/items/imageitem.cpp
        src/items/imageitem.h
        src/items/barcodeitem.cpp
        src/items/barcodeitem.h
        src/models/labelmodels.cpp
        src/models/labelmodels.h
        src/items/qrcodeitem.cpp
        src/items/qrcodeitem.h
        src/ui/labeleditview.cpp
        src/ui/labeleditview.h
        src/ui/propertiespanel.cpp
        src/ui/propertiespanel.h
)

# 项目资源文件
set(PROJECT_RESOURCES
        resources/resources.qrc
        src/mainwindow.cpp
        src/mainwindow.h
        src/application.cpp
        src/application.h
        src/items/labelitem.cpp
        src/items/labelitem.h
        src/items/textitem.cpp
        src/items/textitem.h
        src/items/imageitem.cpp
        src/items/imageitem.h
        src/items/barcodeitem.cpp
        src/items/barcodeitem.h
        src/models/labelmodels.cpp
        src/models/labelmodels.h
        src/items/qrcodeitem.cpp
        src/items/qrcodeitem.h
        src/ui/labeleditview.cpp
        src/ui/labeleditview.h
        src/ui/propertiespanel.cpp
        src/ui/propertiespanel.h
)

# 创建可执行文件
add_executable(${PROJECT_NAME}
        ${PROJECT_SOURCES}
        ${PROJECT_HEADERS}
        ${PROJECT_FORMS}
        ${PROJECT_RESOURCES}
        src/mainwindow.cpp
        src/mainwindow.h
        src/application.cpp
        src/application.h
        src/items/labelitem.cpp
        src/items/labelitem.h
        src/items/textitem.cpp
        src/items/textitem.h
        src/items/imageitem.cpp
        src/items/imageitem.h
        src/items/barcodeitem.cpp
        src/items/barcodeitem.h
        src/models/labelmodels.cpp
        src/models/labelmodels.h
        src/items/qrcodeitem.cpp
        src/items/qrcodeitem.h
        src/ui/labeleditview.cpp
        src/ui/labeleditview.h
        src/ui/propertiespanel.cpp
        src/ui/propertiespanel.h
)

# 查找QRencode库
find_package(QRencode REQUIRED)

# 链接QRencode库
target_link_libraries(${PROJECT_NAME} PRIVATE QRencode::QRencode)
# 在项目的CMakeLists.txt中添加

# 指定自定义模块路径
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# 寻找QRencode库
find_package(QRencode)

# 如果找到库，则编译相关文件
if(QRENCODE_FOUND)
    add_definitions(-DHAVE_QRENCODE)
    include_directories(${QRENCODE_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} PRIVATE ${QRENCODE_LIBRARIES})
endif()

find_package(ZXing REQUIRED)  # 条形码库
find_package(QRencode REQUIRED)  # 二维码库

# 链接库
target_link_libraries(${PROJECT_NAME}
        PRIVATE
        Qt5::Widgets
        Qt5::PrintSupport
        ZXing::ZXing
        QRencode::QRencode
)

# 链接Qt库
target_link_libraries(${PROJECT_NAME} PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::PrintSupport
        Qt${QT_VERSION_MAJOR}::Xml
)

# 设置包含目录，使得源文件可以找到头文件
target_include_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# 安装配置
install(TARGETS ${PROJECT_NAME}
        BUNDLE DESTINATION .
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Windows特定配置
if(WIN32)
    # 复制所需的Qt DLL到输出目录
    set(DEBUG_SUFFIX)
    if(CMAKE_BUILD_TYPE MATCHES "Debug")
        set(DEBUG_SUFFIX "d")
    endif()
    set(QT_INSTALL_PATH "${CMAKE_PREFIX_PATH}")
    if(NOT EXISTS "${QT_INSTALL_PATH}")
        set(QT_INSTALL_PATH "${Qt${QT_VERSION_MAJOR}_DIR}/..")
    endif()

    if(EXISTS "${QT_INSTALL_PATH}/plugins/platforms/qwindows${DEBUG_SUFFIX}.dll")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E make_directory
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/platforms/")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                "${QT_INSTALL_PATH}/plugins/platforms/qwindows${DEBUG_SUFFIX}.dll"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/platforms/")
    endif()

    # 复制Qt核心DLL
    foreach(QT_LIB Core Gui Widgets PrintSupport Xml)
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                "${QT_INSTALL_PATH}/bin/Qt${QT_VERSION_MAJOR}${QT_LIB}${DEBUG_SUFFIX}.dll"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>")
    endforeach(QT_LIB)
endif()