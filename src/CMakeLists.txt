set(SOURCES
    sjv/sjv.cpp
)

sjv_prepend_current_path(SOURCES)
sjv_set_source_group(${SOURCES})
target_sources(sjv PRIVATE ${SOURCES})
