cmake_minimum_required(VERSION 3.13)

project(BackyardBrains
    LANGUAGES CXX
    VERSION 1.13.0)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(LSL REQUIRED)

#: Needed for ui_XY.h files
set(CMAKE_INCLUDE_CURRENT_DIR ON)
#: Enable automatic compilation of .cpp->.moc, xy.ui->ui_xy.h and resource files
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5 REQUIRED COMPONENTS Widgets)

find_package(Threads REQUIRED)

if(WIN32)
    if (CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(HIDAPI_SUBFOLDER x64)
    else()
        set(HIDAPI_SUBFOLDER x86)
    endif()
    if(NOT DEFINED HIDAPI_ROOT_DIR)
        set(HIDAPI_ROOT_DIR
                "${CMAKE_CURRENT_LIST_DIR}/hidapi-win/${HIDAPI_SUBFOLDER}/")
    endif(NOT DEFINED HIDAPI_ROOT_DIR)
endif(WIN32)
find_package(HIDAPI REQUIRED COMPONENTS libusb)

set(PROJECT_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/main.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mainwindow.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mainwindow.h
    ${CMAKE_CURRENT_LIST_DIR}/mainwindow.ui
    ${CMAKE_CURRENT_LIST_DIR}/src/HIDUsbManager.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/HIDUsbManager.h
    ${CMAKE_CURRENT_LIST_DIR}/src/constants.h
    ${CMAKE_CURRENT_LIST_DIR}/src/Paths.h
)
if(WIN32)
    list(APPEND PROJECT_SOURCES
        ${CMAKE_CURRENT_LIST_DIR}/src/native/PathsWin.cpp
    )
elseif(APPLE)
    list(APPEND PROJECT_SOURCES
        ${CMAKE_CURRENT_LIST_DIR}/src/native/PathsMac.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/native/KeyboardGeneratorMac.cpp
    )
else()
    list(APPEND PROJECT_SOURCES
        ${CMAKE_CURRENT_LIST_DIR}/src/native/PathsLinux.cpp
    )
endif()

add_executable(${PROJECT_NAME} MACOSX_BUNDLE WIN32
    ${PROJECT_SOURCES}
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Qt5::Widgets
        Threads::Threads
        LSL::lsl
        HIDAPI::hidapi-libusb
)

# The following shouldn't be necessary because it should be handled by above...
target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${HIDAPI_INCLUDE_DIRS}
        ${LSL_INCLUDE_DIRS}
)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 14)

set(THIRD_PARTY_DLLS
   LSL::lsl
   Qt5::Core
   Qt5::Gui
   Qt5::Widgets
   HIDAPI::hidapi-libusb
)
foreach(_lib ${THIRD_PARTY_DLLS})
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different  
            $<TARGET_FILE:${_lib}>
            $<TARGET_FILE_DIR:${PROJECT_NAME}>)
endforeach()

installLSLApp(${PROJECT_NAME})
installLSLAuxFiles(${PROJECT_NAME}
    ${PROJECT_NAME}.cfg
)

LSLGenerateCPackConfig()
