cmake_minimum_required(VERSION 3.1.0)
project(autopanorama VERSION 1.2.1)
set(TITLE_PROJECT_NAME "AutoPanorama")

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

# Find and use OpenCV
set(OPENCV_INSTALL_PATH "${CMAKE_SOURCE_DIR}/install")
find_package(OpenCV REQUIRED HINTS ${OPENCV_INSTALL_PATH})
include_directories(${OpenCV_INCLUDE_DIRS})
message(STATUS "Found OpenCV version ${OpenCV_VERSION}")
message(STATUS "Found OpenCV headers: ${OpenCV_INCLUDE_DIRS}")
message(STATUS "Using OpenCV libraries: ${OpenCV_LIBRARIES}")

# Custom definition and flags
add_definitions(-DAPP_VERSION="${PROJECT_VERSION}")
set(CMAKE_CXX_STANDARD 11)

if(UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror")
endif()

# Handle rpath before declaring executable
if(UNIX)
    # Add rpath to our custom OpenCV library
    set(AUTOPANORAMA_RPATH "$ORIGIN/../share/${PROJECT_NAME}/lib/")
    list(APPEND CMAKE_INSTALL_RPATH "${AUTOPANORAMA_RPATH}")
    message(STATUS "Setting RPATH: ${CMAKE_INSTALL_RPATH}")
endif()

# Files
set(MAIN_SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/qfilewidget.cpp
    src/videopreprocessor.cpp
    src/panoramamaker.cpp
    src/innercutfinder.cpp
)

set(ICO
    res/autopanorama.ico
)

set(RC
    res/application.qrc
)

if(WIN32)
    set(RC
        ${RC}
        res/autopanorama.rc
    )
endif()

# Main binary
add_executable(${PROJECT_NAME}
    WIN32
    ${MAIN_SOURCES}
    ${RC}
    ${MOC_SRCS}
    ${RES_SOURCES}
    ${UI_HEADERS}
)
target_link_libraries(${PROJECT_NAME}
    ${Qt5Widgets_LIBRARIES}
    ${OpenCV_LIBRARIES}
)

# Install tool function
if(UNIX)
    set(SH_EXT "so")
    set(SH_PREFIX "lib")
    set(DESTINATION_BINARY "bin")
    set(DESTINATION_SH "share/${PROJECT_NAME}/lib")
else()
    set(SH_EXT "dll")
    set(SH_PREFIX "")
    set(DESTINATION_BINARY "bin")
    set(DESTINATION_SH "bin")
endif()

# Install main binary
install(TARGETS
    ${PROJECT_NAME}
    RUNTIME DESTINATION ${DESTINATION_BINARY}
)

# Install desktop file
if(UNIX)
    install(FILES
        res/autopanorama.desktop
        DESTINATION share/applications
    )
    install(FILES
        res/autopanorama.png
        DESTINATION share/${PROJECT_NAME}
    )
endif()

# Install dependencies
if(UNIX)
    set(OPENCV_LIBRARIES_PATH "${OPENCV_INSTALL_PATH}/lib")

    # Add libpng as it breaks between ubuntu 16 and 18
    find_library(libpng_location png)
    get_filename_component(libpng_location ${libpng_location} REALPATH)
    if(NOT libpng_location)
        message(SEND_ERROR "Not found: libpng")
    else()
        message(STATUS "Found libpng: ${all_locations}")
    endif()

    get_filename_component(libpng_name ${libpng_location} NAME)
    file(COPY ${libpng_location} DESTINATION ${CMAKE_BINARY_DIR})
    file(RENAME ${CMAKE_BINARY_DIR}/${libpng_name} ${CMAKE_BINARY_DIR}/libpng12.so.0)
    list(APPEND REQUIRED_LIBS_PATHS ${CMAKE_BINARY_DIR}/libpng12.so.0)
else()
    set(OPENCV_LIBRARIES_PATH "${OPENCV_INSTALL_PATH}/x64/vc15/bin/")

    # Install some Qt libraries
    set(QT_REQUIRED_LIBS_TARGET_NAMES
        Qt5::Widgets
        Qt5::Core
        Qt5::Gui
    )
    foreach(target_name ${QT_REQUIRED_LIBS_TARGET_NAMES})
        get_target_property(location ${target_name} LOCATION_Release)
        get_filename_component(dir ${location} DIRECTORY)
        get_filename_component(basename ${location} NAME_WE)
        file(GLOB all_locations ${dir}/${basename}.${SH_EXT}*)

        if(NOT all_locations)
            message(SEND_ERROR "Not found: ${target_name}")
        else()
            message(STATUS "Found ${target_name}: ${all_locations}")
        endif()
        list(APPEND REQUIRED_LIBS_PATHS ${all_locations})
    endforeach()

    # Install required Qt plugins
    get_filename_component(QT_ROOT ${Qt5_DIR}/../../.. ABSOLUTE)
    set(QT_PLATFORMS ${QT_ROOT}/plugins/platforms)
    set(QT_IMAGE_FORMATS ${QT_ROOT}/plugins/imageformats)

    file(GLOB qjpeg_library "${QT_IMAGE_FORMATS}/${SH_PREFIX}qjpeg.${SH_EXT}")
    if(NOT qjpeg_library)
        message(SEND_ERROR "Not found: qjpeg in ${QT_IMAGE_FORMATS}")
    else()
        message(STATUS "Found qjpeg: ${qjpeg_library}")
    endif()
    install(FILES ${qjpeg_library} DESTINATION ${DESTINATION_SH}/imageformats)

    file(GLOB qico_library "${QT_IMAGE_FORMATS}/${SH_PREFIX}qico.${SH_EXT}")
    if(NOT qico_library)
        message(SEND_ERROR "Not found: qico in ${QT_IMAGE_FORMATS}")
    else()
        message(STATUS "Found qico: ${qico_library}")
    endif()
    install(FILES ${qico_library} DESTINATION ${DESTINATION_SH}/imageformats)

    file(GLOB qwindows_library "${QT_PLATFORMS}/${SH_PREFIX}qwindows.${SH_EXT}")
    if(NOT qwindows_library)
        message(SEND_ERROR "Not found: qwindows in ${QT_PLATFORMS}")
    else()
        message(STATUS "Found qjpeg: ${qwindows_library}")
    endif()
    install(FILES ${qwindows_library} DESTINATION ${DESTINATION_SH}/platforms)
endif()

# Select opencv libs to install
set(OPENCV_REQUIRED_LIBS_NAMES
    opencv_stitching
    opencv_features2d
    opencv_calib3d
    opencv_flann
    opencv_videoio
    opencv_imgcodecs
    opencv_imgproc
    opencv_core
)

# Install required OpenCV libraries
foreach(required_lib_name ${OPENCV_REQUIRED_LIBS_NAMES})
    file(GLOB all_locations "${OPENCV_LIBRARIES_PATH}/${SH_PREFIX}${required_lib_name}*.${SH_EXT}*")
    if(NOT all_locations)
        message(SEND_ERROR "Not found: ${required_lib_name}")
    else()
        message(STATUS "Found ${required_lib_name}: ${all_locations}")
    endif()
    list(APPEND REQUIRED_LIBS_PATHS ${all_locations})
endforeach()

# Install all required libraries
install(FILES
    ${REQUIRED_LIBS_PATHS}
    DESTINATION ${DESTINATION_SH}
)

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
