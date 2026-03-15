pkg_check_modules(GLIB      REQUIRED glib-2.0)
pkg_check_modules(JSON_GLIB REQUIRED json-glib-1.0)

add_library(watchdog-server-teleinfoedf SHARED
  Teleinfo.c
)

set_target_properties(watchdog-server-teleinfoedf PROPERTIES
  VERSION   7.0.0
  SOVERSION 7
  OUTPUT_NAME watchdog-server-teleinfoedf
)

target_compile_options(watchdog-server-teleinfoedf PRIVATE
  ${GLIB_CFLAGS} ${JSON_GLIB_CFLAGS}
)

target_link_options(watchdog-server-teleinfoedf PRIVATE -rdynamic)

target_link_libraries(watchdog-server-teleinfoedf PRIVATE
  ${GLIB_LIBRARIES} ${JSON_GLIB_LIBRARIES}
)

target_link_directories(watchdog-server-teleinfoedf PRIVATE
  ${GLIB_LIBRARY_DIRS} ${JSON_GLIB_LIBRARY_DIRS}
)

install(TARGETS watchdog-server-teleinfoedf LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
