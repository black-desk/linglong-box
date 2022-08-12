pkg_check_modules(SECCOMP REQUIRED libseccomp)

set(LINK_LIBS seccomp stdc++)

add_executable(
  ll-box
  main.cpp
  commands.cpp
  oci/runtime.cpp
  oci/config.cpp
  util/common.cpp
  util/filesystem.cpp
  util/semaphore.cpp
  util/debug/debug.cpp
  util/platform.cpp
  util/logger.cpp
  util/message_reader.cpp
  container/container.cpp
  container/mount/host_mount.cpp
  container/mount/filesystem_driver.cpp
  container/seccomp.cpp)

target_link_libraries(ll-box ${LINK_LIBS})

install(TARGETS ll-box RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
