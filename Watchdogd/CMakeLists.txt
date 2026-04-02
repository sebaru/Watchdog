##################################################################################################################################
# Watchdogd/CMakeLists.txt    Profil de compilation du daemon Watchdog et de ses sous-modules
# Projet Abls-Habitat version 4.7      Gestion d'habitat
# Auteur: LEFEVRE Sebastien
##################################################################################################################################

include_directories(
  ${CMAKE_SOURCE_DIR}/Watchdogd/Include
)

# Sous-modules (shared libraries dlopen-ables)
add_subdirectory(Dls)
add_subdirectory(Smsg)
add_subdirectory(Modbus)
add_subdirectory(Audio)
add_subdirectory(Onduleur)
add_subdirectory(TeleinfoEDF)
add_subdirectory(Radio)
add_subdirectory(Imsgs)
add_subdirectory(Meteo)
add_subdirectory(Phidget)
add_subdirectory(Gpiod)
add_subdirectory(Shelly)

##################################################################################################################################
# Exécutable principal Watchdogd
##################################################################################################################################
add_executable(Watchdogd
  Watchdogd.c
  thread.c
  Erreur.c
  shm.c
  json.c
  Config.c
  http_common.c
  Mnemonique.c
  audio.c
  distrib_Events.c
  mqtt.c
  mqtt_api.c
  mqtt_local.c
  mqtt_archives.c
  mqtt_msgs.c
  mqtt_visuels.c
  mail.c
  uuid.c
)

target_compile_options(Watchdogd PRIVATE
  ${GLIB_CFLAGS}
  ${GTHREAD_CFLAGS}
  ${OPENSSL_CFLAGS}
  ${JSON_GLIB_CFLAGS}
  ${UUID_CFLAGS}
  ${LIBSYSTEMD_CFLAGS}
  ${LIBJWT_CFLAGS}
  ${MOSQUITTO_CFLAGS}
)

target_link_options(Watchdogd PRIVATE -rdynamic)

target_link_libraries(Watchdogd PRIVATE
  watchdog-dls
  -lpopt
  -ldl
  -lcurl
  ${GLIB_LIBRARIES}
  ${GTHREAD_LIBRARIES}
  ${OPENSSL_LIBRARIES}
  ${JSON_GLIB_LIBRARIES}
  ${UUID_LIBRARIES}
  ${LIBSYSTEMD_LIBRARIES}
  ${LIBJWT_LIBRARIES}
  ${MOSQUITTO_LIBRARIES}
)

target_link_directories(Watchdogd PRIVATE
  ${GLIB_LIBRARY_DIRS}
  ${GTHREAD_LIBRARY_DIRS}
  ${OPENSSL_LIBRARY_DIRS}
  ${JSON_GLIB_LIBRARY_DIRS}
  ${UUID_LIBRARY_DIRS}
  ${LIBSYSTEMD_LIBRARY_DIRS}
  ${LIBJWT_LIBRARY_DIRS}
  ${MOSQUITTO_LIBRARY_DIRS}
)

##################################################################################################################################
# Installation
##################################################################################################################################

# Récupération des répertoires systemd via pkg-config
execute_process(
  COMMAND pkg-config --variable=systemdsystemunitdir systemd
  OUTPUT_VARIABLE SYSTEMD_SYSTEM_UNIT_DIR
  OUTPUT_STRIP_TRAILING_WHITESPACE
  ERROR_QUIET
)
if(NOT SYSTEMD_SYSTEM_UNIT_DIR)
  set(SYSTEMD_SYSTEM_UNIT_DIR "${CMAKE_INSTALL_PREFIX}/lib/systemd/system")
endif()

install(TARGETS Watchdogd RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES Watchdogd.service DESTINATION ${SYSTEMD_SYSTEM_UNIT_DIR})
install(FILES watchdogd.abls.conf.sample DESTINATION ${CMAKE_INSTALL_SYSCONFDIR})
install(FILES watchdogd_sudo
        DESTINATION /etc/sudoers.d
        PERMISSIONS OWNER_READ GROUP_READ)
install(FILES 49-modemmanager-abls.rules
  DESTINATION /etc/polkit-1/rules.d
  PERMISSIONS OWNER_READ GROUP_READ WORLD_READ)

# En-tête publique
install(FILES Include/Module_dls.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
