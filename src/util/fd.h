#ifndef LINGLONG_BOX_SRC_UTIL_FD_H
#define LINGLONG_BOX_SRC_UTIL_FD_H

namespace linglong::util {
struct FD {
    int fd;

    FD(int);
    FD(FD &&);
    FD(const FD &) = delete;
    ~FD();
};
} // namespace linglong::util

#endif /* ifndef SYMBOL */
