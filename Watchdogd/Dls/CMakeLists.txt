pkg_check_modules(GLIB      REQUIRED glib-2.0)
pkg_check_modules(GTHREAD   REQUIRED gthread-2.0)
pkg_check_modules(JSON_GLIB REQUIRED json-glib-1.0)

add_library(watchdog-dls SHARED
  plugins.c
  heure.c
  The_dls.c
  The_dls_archive.c
  The_dls_data.c
  The_dls_AI.c
  The_dls_AO.c
  The_dls_DI.c
  The_dls_DO.c
  The_dls_CH.c
  The_dls_CI.c
  The_dls_MESSAGE.c
  The_dls_REGISTRE.c
  The_dls_WATCHDOG.c
  The_dls_HORLOGE.c
  The_dls_TEMPO.c
  The_dls_BI.c
  The_dls_MONO.c
  The_dls_VISUEL.c
)

set_target_properties(watchdog-dls PROPERTIES
  VERSION   7.0.0
  SOVERSION 7
  OUTPUT_NAME watchdog-dls
)

target_compile_options(watchdog-dls PRIVATE
  ${GLIB_CFLAGS} ${GTHREAD_CFLAGS} ${JSON_GLIB_CFLAGS}
)

target_link_options(watchdog-dls PRIVATE -rdynamic)

target_link_libraries(watchdog-dls PRIVATE
  ${GLIB_LIBRARIES} ${GTHREAD_LIBRARIES} ${JSON_GLIB_LIBRARIES}
)

target_link_directories(watchdog-dls PRIVATE
  ${GLIB_LIBRARY_DIRS} ${GTHREAD_LIBRARY_DIRS} ${JSON_GLIB_LIBRARY_DIRS}
)

install(TARGETS watchdog-dls LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
