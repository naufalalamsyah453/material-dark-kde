set(CMAKE_AUTOMOC ON)

add_library(materialdecoration MODULE
    materialdecoration.cpp
    materialbutton.cpp
)
set_target_properties(materialdecoration PROPERTIES OUTPUT_NAME org.kde.material PREFIX "")
target_link_libraries(materialdecoration PRIVATE
    Qt6::Core
    Qt6::Gui
    KF6::CoreAddons
    KDecoration3::KDecoration
)

install(TARGETS materialdecoration DESTINATION ${KDE_INSTALL_PLUGINDIR}/${KDECORATION_PLUGIN_DIR})
