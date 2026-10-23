cmake_minimum_required(VERSION 3.5)

project(DLDSimulator VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
        CircuitScene.h
        CircuitScene.cpp
        CircuitSerializer.h
        CircuitSerializer.cpp
        SelectionBox.h
        SelectionBox.cpp
        GroupSelectionBox.h
        GroupSelectionBox.cpp
        UndoManager.h
        UndoManager.cpp

        ZoomGraphicsView.cpp
        ZoomGraphicsView.h

        overlapping.cpp

        gates/Gate.cpp
        gates/Gate.h
        gates/AndGate.cpp
        gates/AndGate.h
        gates/OrGate.h
        gates/OrGate.cpp
        gates/NotGate.h
        gates/NotGate.cpp
        gates/NandGate.h
        gates/NandGate.cpp
        gates/NorGate.h
        gates/NorGate.cpp
        gates/XorGate.h
        gates/XorGate.cpp
        gates/XnorGate.h
        gates/XnorGate.cpp

        elements/Point.cpp
        elements/Point.h
        elements/Wire.h
        elements/Wire.cpp
        elements/LED.h
        elements/LED.cpp
        elements/Input.h
        elements/Input.cpp
        elements/GND.h
        elements/GND.cpp
        elements/Const1.h
        elements/Const1.cpp
        elements/Switch.h
        elements/Switch.cpp
        elements/NodeTagItem.h
        elements/NodeTagItem.cpp

        logic/LogicTypes.h
        logic/Node.h
        logic/Node.cpp
        logic/CircuitDAG.h
        logic/CircuitDAG.cpp
        logic/CircuitAdapter.h
        logic/CircuitAdapter.cpp
        logic/EquationGenerator.h
        logic/EquationGenerator.cpp
        logic/EquationParser.h
        logic/EquationParser.cpp
        logic/CircuitBuilder.h
        logic/CircuitBuilder.cpp
        EquationDialog.h
        EquationDialog.cpp
        TruthTableDialog.h
        TruthTableDialog.cpp

        ReverseEquationDialog.h
        ReverseEquationDialog.cpp
        resources.qrc

)

if(QT_VERSION_MAJOR GREATER_EQUAL 6)
    qt_add_executable(DLDSimulator
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
else()
    if(ANDROID)
        add_library(DLDSimulator SHARED
            ${PROJECT_SOURCES}
        )
    else()
        add_executable(DLDSimulator
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_include_directories(DLDSimulator PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(DLDSimulator PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

set_target_properties(DLDSimulator PROPERTIES
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)

install(TARGETS DLDSimulator
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(DLDSimulator)
endif()
