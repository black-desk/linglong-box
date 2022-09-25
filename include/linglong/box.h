#ifndef LINGLONG_BOX_INCLUDE_LINGLONG_BOX_H_
#define LINGLONG_BOX_INCLUDE_LINGLONG_BOX_H_

#ifdef __cplusplus
extern "C" {
#endif

int linglong_box_create(const char *const container_id, const char *const path_to_bundle,
                        const char *const notify_socket);
int linglong_box_create_fd(const char *const container_id, int path_fd_to_bundle, const char *const notify_socket);

int linglong_box_start(const char *const container_id, const char *const console_socket,
                       const char *const notify_socket);
int linglong_box_kill(const char *const container_id, int sig);
int linglong_box_delete(const char *const container_id);
int linglong_box_state(const char *const container_id, const char *state);

int linglong_box_list(const char *containers[], int n);
int linglong_box_exec(const char *const container_id, const char *const path_to_process, int fds[], int nfds,
                      const char *const console_socket, const char *const notify_socket);
int linglong_box_exec_fd(const char *const container_id, int path_fd_to_bundle, int fds[], int nfds,
                         const char *const console_socket, const char *const notify_socket);

#ifdef __cplusplus
}
#endif

#endif
