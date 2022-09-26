add_executable(ll-box main.cpp init.cpp monitor.cpp ll-box.cpp commands.cpp
                      rootfs_preparer.cpp)

install(TARGETS ll-box RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
