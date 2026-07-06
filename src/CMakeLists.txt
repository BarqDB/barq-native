add_subdirectory(barq_native/internal/bridge/generator)

include(GetVersion)
# Configure source code to use right version number
configure_file(barq_native/internal/version_numbers.hpp.in barq_native/version_numbers.hpp)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/barq_native/version_numbers.hpp
        DESTINATION include/barq_native
        COMPONENT devel)

set(SOURCES
    barq_native/db.cpp
    barq_native/client_reset.cpp
    barq_native/managed_binary.cpp
    barq_native/managed_decimal.cpp
    barq_native/managed_numeric.cpp
    barq_native/managed_objectid.cpp
    barq_native/managed_primary_key.cpp
    barq_native/managed_string.cpp
    barq_native/managed_timestamp.cpp
    barq_native/managed_uuid.cpp
    barq_native/types.cpp
    barq_native/flex_sync.cpp
    barq_native/internal/bridge/async_open_task.cpp
    barq_native/internal/bridge/binary.cpp
    barq_native/internal/bridge/col_key.cpp
    barq_native/internal/bridge/decimal128.cpp
    barq_native/internal/bridge/dictionary.cpp
    barq_native/internal/bridge/list.cpp
    barq_native/internal/bridge/lnklst.cpp
    barq_native/internal/bridge/mixed.cpp
    barq_native/internal/bridge/obj.cpp
    barq_native/internal/bridge/obj_key.cpp
    barq_native/internal/bridge/object.cpp
    barq_native/internal/bridge/object_id.cpp
    barq_native/internal/bridge/object_schema.cpp
    barq_native/internal/bridge/property.cpp
    barq_native/internal/bridge/query.cpp
    barq_native/internal/bridge/barq.cpp
    barq_native/internal/bridge/results.cpp
    barq_native/internal/bridge/schema.cpp
    barq_native/internal/bridge/set.cpp
    barq_native/internal/bridge/status.cpp
    barq_native/internal/bridge/sync_error.cpp
    barq_native/internal/bridge/sync_manager.cpp
    barq_native/internal/bridge/sync_session.cpp
    barq_native/internal/bridge/table.cpp
    barq_native/internal/bridge/thread_safe_reference.cpp
    barq_native/internal/bridge/timestamp.cpp
    barq_native/internal/bridge/uuid.cpp
    barq_native/internal/networking/network_transport.cpp
    barq_native/internal/networking/shims.cpp
    barq_native/internal/networking/utils.cpp
    barq_native/internal/scheduler/barq_core_scheduler.cpp
    barq_native/networking/http.cpp
    barq_native/networking/websocket.cpp
    barq_native/schedulers/default_scheduler.cpp
    barq_native/logger.cpp
    barq_native/sdk.cpp) # BARQ_SOURCES

set(HEADERS
    ../include/barq_native/accessors.hpp
    ../include/barq_native/db.hpp
    ../include/barq_native/client_reset.hpp
    ../include/barq_native/link.hpp
    ../include/barq_native/macros.hpp
    ../include/barq_native/managed_binary.hpp
    ../include/barq_native/managed_decimal.hpp
    ../include/barq_native/managed_dictionary.hpp
    ../include/barq_native/managed_list.hpp
    ../include/barq_native/managed_mixed.hpp
    ../include/barq_native/managed_numeric.hpp
    ../include/barq_native/managed_objectid.hpp
    ../include/barq_native/managed_primary_key.hpp
    ../include/barq_native/managed_set.hpp
    ../include/barq_native/managed_string.hpp
    ../include/barq_native/managed_timestamp.hpp
    ../include/barq_native/managed_uuid.hpp
    ../include/barq_native/observation.hpp
    ../include/barq_native/results.hpp
    ../include/barq_native/types.hpp
    ../include/barq_native/flex_sync.hpp
    ../include/barq_native/internal/bridge/async_open_task.hpp
    ../include/barq_native/internal/bridge/binary.hpp
    ../include/barq_native/internal/bridge/col_key.hpp
    ../include/barq_native/internal/bridge/decimal128.hpp
    ../include/barq_native/internal/bridge/dictionary.hpp
    ../include/barq_native/internal/bridge/list.hpp
    ../include/barq_native/internal/bridge/lnklst.hpp
    ../include/barq_native/internal/bridge/mixed.hpp
    ../include/barq_native/internal/bridge/obj.hpp
    ../include/barq_native/internal/bridge/obj_key.hpp
    ../include/barq_native/internal/bridge/object.hpp
    ../include/barq_native/internal/bridge/object_id.hpp
    ../include/barq_native/internal/bridge/object_schema.hpp
    ../include/barq_native/internal/bridge/property.hpp
    ../include/barq_native/internal/bridge/query.hpp
    ../include/barq_native/internal/bridge/barq.hpp
    ../include/barq_native/internal/bridge/results.hpp
    ../include/barq_native/internal/bridge/schema.hpp
    ../include/barq_native/internal/bridge/set.hpp
    ../include/barq_native/internal/bridge/status.hpp
    ../include/barq_native/internal/bridge/sync_error.hpp
    ../include/barq_native/internal/bridge/sync_manager.hpp
    ../include/barq_native/internal/bridge/sync_session.hpp
    ../include/barq_native/internal/bridge/table.hpp
    ../include/barq_native/internal/bridge/thread_safe_reference.hpp
    ../include/barq_native/internal/bridge/timestamp.hpp
    ../include/barq_native/internal/bridge/utils.hpp
    ../include/barq_native/internal/bridge/uuid.hpp
    ../include/barq_native/internal/type_info.hpp
    ../include/barq_native/internal/networking/shims.hpp
    ../include/barq_native/internal/networking/utils.hpp
    ../include/barq_native/internal/scheduler/barq_core_scheduler.hpp
    ../include/barq_native/schedulers/default_scheduler.hpp
    ../include/barq_native/networking/http.hpp
    ../include/barq_native/networking/websocket.hpp
    ../include/barq_native/logger.hpp
    ../include/barq_native/notifications.hpp
    ../include/barq_native/rbool.hpp
    ../include/barq_native/scheduler.hpp
    ../include/barq_native/schema.hpp
    ../include/barq_native/thread_safe_reference.hpp
    ../include/barq_native/sdk.hpp) # BARQ_INSTALL_HEADERS

if (NOT BARQ_DISABLE_ALIGNED_STORAGE)
    add_custom_command(
            COMMAND ${CMAKE_COMMAND} -D SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}/barq_native/internal/bridge
            -D BINARY_DIR=${CMAKE_CURRENT_BINARY_DIR}/barq_native/internal/bridge
            -D BRIDGE_TYPE_INFO_BIN=$<TARGET_FILE:BridgeTypeInfoGenerator>
            -P ${CMAKE_CURRENT_SOURCE_DIR}/barq_native/internal/bridge/generator/bridge_type_info_parser.cmake
            DEPENDS BridgeTypeInfoGenerator
            DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/barq_native/internal/bridge/generator/bridge_type_info_parser.cmake
            DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/barq_native/internal/bridge/bridge_types.hpp.in
            OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/barq_native/internal/bridge/bridge_types.hpp
    )

    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/barq_native/internal/bridge/bridge_types.hpp
            DESTINATION include/barq_native/internal/bridge
            COMPONENT devel)

    set(BRIDGING_HEADER ${CMAKE_CURRENT_BINARY_DIR}/barq_native/internal/bridge/bridge_types.hpp)
endif()

include(GNUInstallDirs)

add_library(barq_native ${SOURCES} ${HEADERS} ${BRIDGING_HEADER})
add_library(Barq::Native ALIAS barq_native)

set(INSTALL_TARGETS ObjectStore Storage QueryParser Sync)

install(TARGETS ${PROJECT_NAME} ${INSTALL_TARGETS}
        EXPORT "${PROJECT_NAME}Targets"
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} # include
        )

foreach(header ${HEADERS})
    get_filename_component(dir ${header} DIRECTORY)
    install(FILES ${header}
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${dir}
            COMPONENT devel)
endforeach()

if (NOT BARQ_DISABLE_ALIGNED_STORAGE)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/barq_native/internal/bridge/bridge_types.hpp
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/barq_native/internal/bridge
            COMPONENT devel)
endif()

install()
