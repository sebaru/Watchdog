pkg_check_modules(GLIB      REQUIRED glib-2.0)
pkg_check_modules(GTHREAD   REQUIRED gthread-2.0)
pkg_check_modules(JSON_GLIB REQUIRED json-glib-1.0)

add_library(watchdog-server-modbus SHARED
  Modbus.c
)

set_target_properties(watchdog-server-modbus PROPERTIES
  VERSION   7.0.0
  SOVERSION 7
  OUTPUT_NAME watchdog-server-modbus
)

target_compile_options(watchdog-server-modbus PRIVATE
  ${GLIB_CFLAGS} ${GTHREAD_CFLAGS} ${JSON_GLIB_CFLAGS}
)

target_link_options(watchdog-server-modbus PRIVATE -rdynamic)

target_link_libraries(watchdog-server-modbus PRIVATE
  ${GLIB_LIBRARIES} ${GTHREAD_LIBRARIES} ${JSON_GLIB_LIBRARIES}
)

target_link_directories(watchdog-server-modbus PRIVATE
  ${GLIB_LIBRARY_DIRS} ${GTHREAD_LIBRARY_DIRS} ${JSON_GLIB_LIBRARY_DIRS}
)

install(TARGETS watchdog-server-modbus LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
