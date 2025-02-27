if(IVW_APP_INVIWO OR IVW_APP_QTBASE)

    find_package(Qt6 REQUIRED COMPONENTS Core QUIET)
    if(NOT Qt6Core_FOUND) 
        message(FATAL_ERROR 
            "The inviwo app requiers Qt, we were not able to find it. Consider adding Qt to the "
            "CMAKE_PREFIX_PATH, i.e. start cmake with the option -DCMAKE_PREFIX_PATH=\"/path/to/qt\"")
    endif()

    # Create a qt.conf file in the same dir as the executable to help qt find its plugins
    # http://doc.qt.io/qt-6/qt-conf.html
    set(QT_BASE_PATH "${Qt6Core_DIR}/../../..")
    foreach(conf ${CMAKE_CONFIGURATION_TYPES})
        configure_file("${IVW_CMAKE_TEMPLATES}/qt_template.conf" "${EXECUTABLE_OUTPUT_PATH}/${conf}/qt.conf" @ONLY)
    endforeach()

    if(WIN32 AND QtCore_FOUND)
        set(QT_BINARY_DIR "${Qt6Core_DIR}/../../../bin")
        install(DIRECTORY ${QT_BINARY_DIR} DESTINATION bin/../ COMPONENT qt_widgets FILES_MATCHING PATTERN "icudt5*")
        install(DIRECTORY ${QT_BINARY_DIR} DESTINATION bin/../ COMPONENT qt_widgets FILES_MATCHING PATTERN "icuin5*")
        install(DIRECTORY ${QT_BINARY_DIR} DESTINATION bin/../ COMPONENT qt_widgets FILES_MATCHING PATTERN "icuuc5*")
    endif()

    if(IVW_APP_QTBASE)
        add_subdirectory(applicationbase)
    endif()

    if(IVW_APP_INVIWO)
        add_subdirectory(editor)
    endif()
endif()
