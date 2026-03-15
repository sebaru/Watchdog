pkg_check_modules(GLIB      REQUIRED glib-2.0)
pkg_check_modules(GTHREAD   REQUIRED gthread-2.0)
pkg_check_modules(JSON_GLIB REQUIRED json-glib-1.0)
pkg_check_modules(MOSQUITTO REQUIRED libmosquitto)

add_library(watchdog-server-shelly SHARED
  Shelly.c
)

set_target_properties(watchdog-server-shelly PROPERTIES
  VERSION   7.0.0
  SOVERSION 7
  OUTPUT_NAME watchdog-server-shelly
)

target_compile_options(watchdog-server-shelly PRIVATE
  ${GLIB_CFLAGS} ${GTHREAD_CFLAGS} ${JSON_GLIB_CFLAGS} ${MOSQUITTO_CFLAGS}
)

target_link_options(watchdog-server-shelly PRIVATE -rdynamic)

target_link_libraries(watchdog-server-shelly PRIVATE
  ${GLIB_LIBRARIES} ${GTHREAD_LIBRARIES} ${JSON_GLIB_LIBRARIES} ${MOSQUITTO_LIBRARIES}
)

target_link_directories(watchdog-server-shelly PRIVATE
  ${GLIB_LIBRARY_DIRS} ${GTHREAD_LIBRARY_DIRS} ${JSON_GLIB_LIBRARY_DIRS} ${MOSQUITTO_LIBRARY_DIRS}
)

install(TARGETS watchdog-server-shelly LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
