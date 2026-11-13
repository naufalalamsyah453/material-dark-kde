add_library(materialstyle MODULE
    materialstyle.cpp
    materialstyleplugin.cpp
)
set_target_properties(materialstyle PROPERTIES OUTPUT_NAME materialdesign PREFIX "")
target_link_libraries(materialstyle PRIVATE Qt6::Widgets)

install(TARGETS materialstyle DESTINATION ${MATERIAL_QT_PLUGIN_DIR}/styles)
install(FILES materialdesign.themerc DESTINATION ${MATERIAL_DATA_DIR}/kstyle/themes)
