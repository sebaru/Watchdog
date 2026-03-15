pkg_check_modules(GLIB      REQUIRED glib-2.0)
pkg_check_modules(GTHREAD   REQUIRED gthread-2.0)
pkg_check_modules(JSON_GLIB REQUIRED json-glib-1.0)

add_library(watchdog-server-audio SHARED
  Audio.c
)

set_target_properties(watchdog-server-audio PROPERTIES
  VERSION   7.0.0
  SOVERSION 7
  OUTPUT_NAME watchdog-server-audio
)

target_compile_options(watchdog-server-audio PRIVATE
  ${GLIB_CFLAGS} ${GTHREAD_CFLAGS} ${JSON_GLIB_CFLAGS}
)

target_link_options(watchdog-server-audio PRIVATE -rdynamic)

target_link_libraries(watchdog-server-audio PRIVATE
  ${GLIB_LIBRARIES} ${GTHREAD_LIBRARIES} ${JSON_GLIB_LIBRARIES}
)

target_link_directories(watchdog-server-audio PRIVATE
  ${GLIB_LIBRARY_DIRS} ${GTHREAD_LIBRARY_DIRS} ${JSON_GLIB_LIBRARY_DIRS}
)

install(TARGETS watchdog-server-audio LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
