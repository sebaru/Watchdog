pkg_check_modules(GLIB       REQUIRED glib-2.0)
pkg_check_modules(GTHREAD    REQUIRED gthread-2.0)
pkg_check_modules(JSON_GLIB  REQUIRED json-glib-1.0)
pkg_check_modules(UPSCLIENT  REQUIRED libupsclient)

add_library(watchdog-server-ups SHARED
  Onduleur.c
)

set_target_properties(watchdog-server-ups PROPERTIES
  VERSION   7.0.0
  SOVERSION 7
  OUTPUT_NAME watchdog-server-ups
)

target_compile_options(watchdog-server-ups PRIVATE
  ${GLIB_CFLAGS} ${GTHREAD_CFLAGS} ${JSON_GLIB_CFLAGS} ${UPSCLIENT_CFLAGS}
)

target_link_options(watchdog-server-ups PRIVATE -rdynamic)

target_link_libraries(watchdog-server-ups PRIVATE
  ${GLIB_LIBRARIES} ${GTHREAD_LIBRARIES} ${JSON_GLIB_LIBRARIES} ${UPSCLIENT_LIBRARIES}
)

target_link_directories(watchdog-server-ups PRIVATE
  ${GLIB_LIBRARY_DIRS} ${GTHREAD_LIBRARY_DIRS} ${JSON_GLIB_LIBRARY_DIRS} ${UPSCLIENT_LIBRARY_DIRS}
)

install(TARGETS watchdog-server-ups LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
