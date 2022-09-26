add_subdirectory(lib)

add_executable(
  ll-box
  commands.cpp
  init.cpp
  ll-box.cpp
  main.cpp
  monitor.cpp
  rootfs_preparer.cpp
)

target_link_libraries(ll-box llbox)

install(TARGETS ll-box RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
