# add_subdirectory(lib)

add_executable(ll-box main.cpp init.cpp monitor.cpp)
# cli/commands.cpp oci/parse.cpp oci/runtime.cpp oci/util.cpp util/fd.cpp
# util/lock.cpp container/container.cpp container/init.cpp container/rootfs.cpp

# target_link_libraries(ll-box ${LINK_LIBS})

install(TARGETS ll-box RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
