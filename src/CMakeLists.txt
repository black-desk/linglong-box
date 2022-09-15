pkg_check_modules(SECCOMP REQUIRED libseccomp)

set(LINK_LIBS seccomp stdc++)

add_executable(
  ll-box
  main.cpp
  commands.cpp
  init.cpp
  monitor.cpp
  oci/parse.cpp
  oci/runtime.cpp
  oci/util.cpp
  util/fd.cpp
  util/lock.cpp
  container/container.cpp
  container/container_option.cpp
  container/init.cpp
  container/rootfs.cpp)

target_link_libraries(ll-box ${LINK_LIBS})

install(TARGETS ll-box RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
