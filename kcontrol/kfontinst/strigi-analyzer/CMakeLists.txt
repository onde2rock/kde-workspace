include_directories( ${KDEBASE_WORKSPACE_SOURCE_DIR}/kcontrol/kfontinst/lib/
                     ${STRIGI_INCLUDE_DIR}
                   )

kde4_add_library(font MODULE FontThroughAnalyzer.cpp FontEngine.cpp )

target_link_libraries(font ${STRIGI_STREAMANALYZER_LIBRARY} ${FREETYPE_LIBRARIES} ${FONTCONFIG_LIBRARIES} ${KDE4_KIO_LIBS} kfontinst )

set_target_properties(font PROPERTIES  PREFIX strigita_)

install(TARGETS font LIBRARY DESTINATION ${LIB_INSTALL_DIR}/strigi)

