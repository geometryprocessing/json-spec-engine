set(SOURCES
    jse/jse.cpp
)

jse_prepend_current_path(SOURCES)
jse_set_source_group(${SOURCES})
target_sources(jse PRIVATE ${SOURCES})
