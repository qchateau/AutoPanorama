cmake_minimum_required(VERSION 3.1.0)
project(autopanorama VERSION 1.3.1)
set(TITLE_PROJECT_NAME "AutoPanorama")

# Configure project
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_EXTENSIONS OFF)
set(BUILD_SHARED_LIBS OFF)
if (ENABLE_LOG)
    add_definitions(-DENABLE_LOG=1)
endif ()

# Include OpenCV
set(WITH_EIGEN ON)
set(WITH_LAPACK OFF)
set(WITH_MATLAB OFF)
set(WITH_VTK OFF)
add_subdirectory(opencv EXCLUDE_FROM_ALL)
file(GLOB OPENCV_INCLUDE_DIRS opencv/modules/*/include)
include_directories(BEFORE SYSTEM opencv/include ${OPENCV_INCLUDE_DIRS})

# Find and use Qt
find_package(Qt5 COMPONENTS Widgets REQUIRED)
include_directories(${Qt5Widgets_INCLUDE_DIRS})
add_definitions(${Qt5Widgets_DEFINITIONS})
message(STATUS "Found Qt version ${Qt5Core_VERSION}")
message(STATUS "Found Qt headers: ${Qt5Widgets_INCLUDE_DIRS}")
message(STATUS "Using Qt libraries: ${Qt5Widgets_LIBRARIES}")
message(STATUS "Using Qt definitions: ${Qt5Widgets_DEFINITIONS}")

# Use auto-eveything for Qt
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Custom definition and flags
add_definitions(-DAPP_VERSION="${PROJECT_VERSION}")

if(UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror")
    # https://stackoverflow.com/questions/47117443/dynamic-linking-with-rpath-not-working-under-ubuntu-17-10
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--disable-new-dtags")
elseif(WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /WX")
endif()

# Files
set(ICO res/autopanorama.ico)
set(RC res/application.qrc)
if(WIN32)
    list(APPEND RC res/autopanorama.rc)
endif()

set(
    SOURCES

    src/main.cpp
    src/mainwindow.cpp
    src/qfilewidget.cpp
    src/videopreprocessor.cpp
    src/panoramamaker.cpp
    src/innercutfinder.cpp
    src/post_process.cpp

    ${ICO}
    ${RC}
)

set(
    OPENCV_LIBRARIES

    opencv_stitching
    opencv_features2d
    opencv_calib3d
    opencv_flann
    opencv_videoio
    opencv_imgcodecs
    opencv_imgproc
    opencv_core
)

# Main binary
add_executable(
    ${PROJECT_NAME}

    WIN32
    ${SOURCES}
)
target_link_libraries(
    ${PROJECT_NAME}

    ${Qt5Widgets_LIBRARIES}
    ${OPENCV_LIBRARIES}
)

# Install main binary
install(
    TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
)

# Install desktop file
if(UNIX)
    install(
        FILES

        res/autopanorama.desktop
        DESTINATION share/applications
    )
    install(
        FILES

        res/autopanorama.png
        DESTINATION share/${PROJECT_NAME}
    )
endif()

# Install dependencies
if(WIN32)
    # Install OpenCV libraries
    install(
        TARGETS ${OPENCV_LIBRARIES}
        RUNTIME DESTINATION bin
    )

    # Install Qt libraries
    set(QT_REQUIRED_LIBS_TARGET_NAMES
        Qt5::Widgets
        Qt5::Core
        Qt5::Gui
    )
    foreach(target_name ${QT_REQUIRED_LIBS_TARGET_NAMES})
        get_target_property(location ${target_name} LOCATION_Release)
        get_filename_component(dir ${location} DIRECTORY)
        get_filename_component(basename ${location} NAME_WE)
        file(GLOB all_locations ${dir}/${basename}.dll*)

        if(NOT all_locations)
            message(SEND_ERROR "Not found: ${target_name}")
        else()
            message(STATUS "Found ${target_name}: ${all_locations}")
        endif()
        list(APPEND QT_LIBS_PATHS ${all_locations})
    endforeach()
    install(FILES ${QT_LIBS_PATHS} DESTINATION bin)

    # Install required Qt plugins
    get_filename_component(QT_ROOT ${Qt5_DIR}/../../.. ABSOLUTE)
    set(QT_PLATFORMS ${QT_ROOT}/plugins/platforms)
    set(QT_IMAGE_FORMATS ${QT_ROOT}/plugins/imageformats)

    file(GLOB qjpeg_library "${QT_IMAGE_FORMATS}/qjpeg.dll")
    if(NOT qjpeg_library)
        message(SEND_ERROR "Not found: qjpeg in ${QT_IMAGE_FORMATS}")
    else()
        message(STATUS "Found qjpeg: ${qjpeg_library}")
    endif()
    install(FILES ${qjpeg_library} DESTINATION bin/imageformats)

    file(GLOB qico_library "${QT_IMAGE_FORMATS}/qico.dll")
    if(NOT qico_library)
        message(SEND_ERROR "Not found: qico in ${QT_IMAGE_FORMATS}")
    else()
        message(STATUS "Found qico: ${qico_library}")
    endif()
    install(FILES ${qico_library} DESTINATION bin/imageformats)

    file(GLOB qwindows_library "${QT_PLATFORMS}/qwindows.dll")
    if(NOT qwindows_library)
        message(SEND_ERROR "Not found: qwindows in ${QT_PLATFORMS}")
    else()
        message(STATUS "Found qjpeg: ${qwindows_library}")
    endif()
    install(FILES ${qwindows_library} DESTINATION bin/platforms)
endif()

# Package
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Compose panoramas from multiple photos or movies")
set(CPACK_PACKAGE_CONTACT "Quentin Chateau (quentin.chateau@gmail.com)")
set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}-${PROJECT_VERSION}")

if(UNIX)
    set(CPACK_GENERATOR "DEB")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "libqt5core5a, libqt5gui5, libqt5widgets5")
elseif(WIN32)
    set(CPACK_GENERATOR "NSIS")

    get_filename_component(ico_abspath ${ICO} REALPATH)
    string(REPLACE "/" "\\\\" ico_abspath ${ico_abspath})

    set(CPACK_PACKAGE_EXECUTABLES ${PROJECT_NAME} ${TITLE_PROJECT_NAME})
    set(CPACK_PACKAGE_INSTALL_REGISTRY_KEY "${PROJECT_NAME}-${PROJECT_VERSION}")

    set(CPACK_NSIS_EXTRA_INSTALL_COMMANDS "
        CreateShortCut \\\"$DESKTOP\\\\${TITLE_PROJECT_NAME}.lnk\\\" \\\"$INSTDIR\\\\bin\\\\${PROJECT_NAME}.exe\\\"
    ")
    set(CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS "
        Delete \\\"$DESKTOP\\\\${TITLE_PROJECT_NAME}.lnk\\\"
    ")

    set(CPACK_NSIS_INSTALLED_ICON_NAME "bin\\\\${PROJECT_NAME}.exe")
    set(CPACK_NSIS_MUI_ICON ${ico_abspath})
    set(CPACK_NSIS_MUI_UNIICON ${ico_abspath})
    set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
    set(CPACK_PACKAGE_INSTALL_DIRECTORY ${TITLE_PROJECT_NAME})
    set(CPACK_NSIS_DISPLAY_NAME ${TITLE_PROJECT_NAME})
    set(CPACK_NSIS_PACKAGE_NAME ${TITLE_PROJECT_NAME})
    set(CPACK_NSIS_URL_INFO_ABOUT "https://github.com/qchateau/AutoPanorama/")
    set(CPACK_NSIS_HELP_LINK "https://github.com/qchateau/AutoPanorama/")
    set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE.md")
else ()
    message(FATAL_ERROR "Unsupported system")
endif()

include(CPack)
