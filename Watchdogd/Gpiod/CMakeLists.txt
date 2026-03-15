pkg_check_modules(GLIB      REQUIRED glib-2.0)
pkg_check_modules(JSON_GLIB REQUIRED json-glib-1.0)
pkg_check_modules(LIBGPIOD  REQUIRED libgpiod)

add_library(watchdog-server-gpiod SHARED
  Gpiod.c
)

set_target_properties(watchdog-server-gpiod PROPERTIES
  VERSION   7.0.0
  SOVERSION 7
  OUTPUT_NAME watchdog-server-gpiod
)

target_compile_options(watchdog-server-gpiod PRIVATE
  ${GLIB_CFLAGS} ${JSON_GLIB_CFLAGS} ${LIBGPIOD_CFLAGS}
)

target_link_options(watchdog-server-gpiod PRIVATE -rdynamic)

target_link_libraries(watchdog-server-gpiod PRIVATE
  ${GLIB_LIBRARIES} ${JSON_GLIB_LIBRARIES} ${LIBGPIOD_LIBRARIES}
)

target_link_directories(watchdog-server-gpiod PRIVATE
  ${GLIB_LIBRARY_DIRS} ${JSON_GLIB_LIBRARY_DIRS} ${LIBGPIOD_LIBRARY_DIRS}
)

install(TARGETS watchdog-server-gpiod LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
