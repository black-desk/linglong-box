#include "fd.h"

#include <sys/socket.h>
#include <sys/un.h>

#include <optional>

namespace linglong::util {

void readWithRetry(int fd, void *buffer, std::size_t len, std::vector<std::shared_ptr<FD>> *fds = nullptr)
{
    if (len <= 0) {
        throw fmt::system_error(EINVAL, "readWithRetry cannot be called with non positive len(={})", len);
    }

    if (fds) {
        // TODO: add more checks

        auto control = fds ? new char[CMSG_SPACE(sizeof(int) * fds->size())] : nullptr;

        iovec iov({buffer, 1});

        struct msghdr msg({
            NULL,
            0,
            &iov,
            1,
            control,
            CMSG_SPACE(sizeof(int) * (fds ? 0 : fds->size())),
        });

        int ret = -1;
        do {
            ret = recvmsg(fd, &msg, 0);
        } while (ret == -1 && (errno == EAGAIN || errno == EINTR));
        if (ret == -1) {
            throw fmt::system_error(errno, "sendmsg() to {} failed", fd);
        }

        int *ptr = (int *)CMSG_DATA(CMSG_FIRSTHDR(&msg));
        for (auto &fd : *fds) {
            fd.reset(new FD(*ptr++));
        }

        buffer = (void *)((char *)buffer + 1);
        len--;
    }

    const char *_buffer = (const char *)buffer;

    while (len > 0) {
        int readBytes;
        do {
            readBytes = read(fd, (void *)_buffer, len);
        } while (readBytes == -1 && (errno == EAGAIN || errno == EINTR));
        if (readBytes == -1) {
            throw fmt::system_error(errno, "read() {} bytes from {} failed", len, fd);
        }
        len -= readBytes;
        _buffer += readBytes;
    }
}

void writeWithRetry(int fd, void *buffer, std::size_t len, const std::vector<std::shared_ptr<FD>> *fds = nullptr)
{
    if (len <= 0) {
        throw fmt::system_error(EINVAL, "writeWithRetry cannot be called with non positive len(={})", len);
    }
    if (fds) {
        if (!fds->size()) {
            throw fmt::system_error(EINVAL, "writeWithRetry cannot be called with fds which is empty");
        }
        for (auto fd : *fds) {
            if (!fd) {
                throw fmt::system_error(EINVAL, "writeWithRetry cannot be called with fds with nullptr in it");
            }
            if (fd->fd < 0) {
                throw fmt::system_error(EINVAL, "writeWithRetry cannot be called with fds reference to negative fd");
            }
        }
        auto control = fds ? new char[CMSG_SPACE(sizeof(int) * fds->size())] : nullptr;

        iovec iov({buffer, 1});

        struct msghdr msg({
            NULL,
            0,
            &iov,
            1,
            control,
            CMSG_SPACE(sizeof(int) * (fds ? 0 : fds->size())),
        });

        if (fds) {
            auto cmsg = CMSG_FIRSTHDR(&msg);
            cmsg->cmsg_len = CMSG_LEN(sizeof(int) * fds->size());
            cmsg->cmsg_level = SOL_SOCKET;
            cmsg->cmsg_type = SCM_RIGHTS;
            int *ptr = (int *)CMSG_DATA(cmsg);
            for (auto it : *fds) {
                *ptr++ = it->fd;
            }
        }

        int ret = -1;
        do {
            ret = sendmsg(fd, &msg, 0);
        } while (ret == -1 && (errno == EAGAIN || errno == EINTR));
        if (ret == -1) {
            throw fmt::system_error(errno, "sendmsg() to {} failed", fd);
        }
        buffer = (void *)((char *)buffer + 1);
        len--;
    }

    const char *_buffer = (const char *)buffer + 1;

    while (len > 0) {
        int writeBytes;
        do {
            writeBytes = write(fd, (void *)_buffer, len);
        } while (writeBytes == -1 && (errno == EAGAIN || errno == EINTR));
        if (writeBytes == -1) {
            throw fmt::system_error(errno, "write() {} bytes to {} failed", len, fd);
        }
        len -= writeBytes;
        _buffer += writeBytes;
    }
}

PipeReadEnd::PipeReadEnd(int fd)
    : FD(fd)
{
}

PipeReadEnd &PipeReadEnd::operator>>(int &x)
{
    try {
        readWithRetry(fd, &x, sizeof(x));
        return *this;
    } catch (...) {
        std::throw_with_nested(std::runtime_error(fmt::format("read from pipe {} failed", fd)));
    }
}

PipeWriteEnd &PipeWriteEnd::operator<<(const int &x)
{
    try {
        writeWithRetry(fd, (void *)&x, sizeof(x));
        return *this;
    } catch (...) {
        std::throw_with_nested(std::runtime_error(fmt::format("write to pipe {} failed", fd)));
    }
}

PipeWriteEnd::PipeWriteEnd(int fd)
    : FD(fd)
{
}

std::pair<std::shared_ptr<PipeReadEnd>, std::shared_ptr<PipeWriteEnd>> pipe()
{
    int ends[2];
    if (::pipe(ends)) {
        throw fmt::system_error(errno, "pipe() failed");
    }
    return std::make_pair(std::shared_ptr<PipeReadEnd>(new PipeReadEnd(ends[0])),
                          std::shared_ptr<PipeWriteEnd>(new PipeWriteEnd(ends[1])));
}

Socket::Socket(int fd)
    : FD(fd)
{
}

Socket &Socket::operator>>(Message &x)
{
    int len, fdslen;
    readWithRetry(fd, &len, sizeof(len));
    readWithRetry(fd, &fdslen, sizeof(fdslen));
    char *buffer = new char[len];
    x.fds.clear();
    for (int i = 0; i < fdslen; i++) {
        x.fds.push_back(nullptr);
    }
    readWithRetry(fd, buffer, len, &x.fds);
    x.raw = nlohmann::json::parse(buffer);
    return *this;
}

Socket &Socket::operator<<(const Message &x)
{
    auto rawJson = to_string(x.raw);
    int len = rawJson.length();
    int fdslen = x.fds.size();
    writeWithRetry(fd, &len, sizeof(len));
    writeWithRetry(fd, &fdslen, sizeof(fdslen));
    writeWithRetry(fd, (void *)rawJson.c_str(), len, &x.fds);
    return *this;
}

Pipe::Pipe()
{
    int ends[2];
    if (::pipe(ends)) {
        throw fmt::system_error(errno, "pipe() failed");
    }
    read.reset(new PipeReadEnd(ends[0]));
    write.reset(new PipeWriteEnd(ends[1]));
}

PipeReadEnd &Pipe::operator>>(int &x)
{
    return *this->read >> x;
}

PipeWriteEnd &Pipe::operator<<(const int &x)
{
    return *this->write << x;
}

} // namespace linglong::util
