#ifndef LINGLONG_BOX_SRC_OCI_CONFIG_H
#define LINGLONG_BOX_SRC_OCI_CONFIG_H

#include <string>

#include <sys/resource.h>
#include <sys/capability.h>
#include <linux/seccomp.h>
#include <seccomp.h>

#include "serializer.h"

namespace linglong::OCI {

// https://github.com/opencontainers/runtime-spec/blob/v1.0.2/config.md
// LINUX https://github.com/opencontainers/runtime-spec/blob/v1.0.2/config-linux.md
struct Config {
    // https://github.com/opencontainers/runtime-spec/blob/v1.0.2/config.md#root
    struct Root {
        std::filesystem::path path;
        std::optional<bool> readonly;

        void parse(std::filesystem::path bundlePath);
    };

    // https://github.com/opencontainers/runtime-spec/blob/main/config-linux.md#user-namespace-mappings
    struct IDMapping {
        uint32_t containerID;
        uint32_t hostID;
        uint32_t size;
    };

    // https://github.com/opencontainers/runtime-spec/blob/v1.0.2/config.md#mounts
    //
    // POSIX https://github.com/opencontainers/runtime-spec/blob/v1.0.2/config.md#posix-platform-mounts
    struct Mount {
        using Type = std::string;

        static const Type Bind, Proc, Sysfs, Devpts, Mqueue, Tmpfs, Cgroup, Cgroup2;

        std::filesystem::path destination;
        std::optional<std::filesystem::path> source;
        std::optional<std::vector<std::string>> options;
        std::optional<Type> type; // POSIX

        // POSIX, NOT RELEASE
        //
        // https://github.com/opencontainers/runtime-spec/blob/main/config.md#posix-platform-mounts
        std::optional<IDMapping> uidMappings, gidMappings;

        struct Parsed {
            std::vector<std::string> data;
            uint32_t flags = 0;
        };

        std::unique_ptr<Parsed> parsed;

        void parse(std::filesystem::path bundlePath);
    };

    // https://github.com/opencontainers/runtime-spec/blob/main/config.md#process
    //
    // POSIX https://github.com/opencontainers/runtime-spec/blob/v1.0.2/config.md#posix-process
    //
    // LINUX https://github.com/opencontainers/runtime-spec/blob/v1.0.2/config.md#linux-process
    struct Process {
        struct ConsoleSize {
            uint height;
            uint width;
        };

        struct Rlimit {
            typedef std::string Type;

#ifdef RLIMIT_AS
            static const Type AS;
#endif
#ifdef RLIMIT_CORE
            static const Type Core;
#endif
#ifdef RLIMIT_CPU
            static const Type CPU;
#endif
#ifdef RLIMIT_DATA
            static const Type Data;
#endif
#ifdef RLIMIT_FSIZE
            static const Type FSize;
#endif
#ifdef RLIMIT_LOCKS
            static const Type Locks;
#endif
#ifdef RLIMIT_MEMLOCK
            static const Type MemLock;
#endif
#ifdef RLIMIT_MSGQUEUE
            static const Type MsgQueue;
#endif
#ifdef RLIMIT_NICE
            static const Type Nice;
#endif
#ifdef RLIMIT_NOFILE
            static const Type NOFile;
#endif
#ifdef RLIMIT_NPROC
            static const Type NProc;
#endif
#ifdef RLIMIT_RSS
            static const Type RSS;
#endif
#ifdef RLIMIT_RTPRIO
            static const Type RTPrio;
#endif
#ifdef RLIMIT_RTTIME
            static const Type RTTime;
#endif
#ifdef RLIMIT_SIGPENDING
            static const Type SigPending;
#endif
#ifdef RLIMIT_STACK
            static const Type Stack;
#endif

            Type type;
            uint64_t soft;
            uint64_t hard;
        };

        struct Capabilities {
            typedef std::string Capability;

#ifdef CAP_AUDIT_CONTROL
            static const Capability AUDIT_CONTROL;
#endif
#ifdef CAP_AUDIT_READ
            static const Capability AUDIT_READ;
#endif
#ifdef CAP_AUDIT_WRITE
            static const Capability AUDIT_WRITE;
#endif
#ifdef CAP_BLOCK_SUSPEND
            static const Capability BLOCK_SUSPEND;
#endif
#ifdef CAP_BPF
            static const Capability BPF;
#endif
#ifdef CAP_CHECKPOINT_RESTORE
            static const Capability CHECKPOINT_RESTORE;
#endif
#ifdef CAP_CHOWN
            static const Capability CHOWN;
#endif
#ifdef CAP_DAC_OVERRIDE
            static const Capability DAC_OVERRIDE;
#endif
#ifdef CAP_DAC_READ_SEARCH
            static const Capability DAC_READ_SEARCH;
#endif
#ifdef CAP_FOWNER
            static const Capability FOWNER;
#endif
#ifdef CAP_FSETID
            static const Capability FSETID;
#endif
#ifdef CAP_IPC_LOCK
            static const Capability IPC_LOCK;
#endif
#ifdef CAP_IPC_OWNER
            static const Capability IPC_OWNER;
#endif
#ifdef CAP_KILL
            static const Capability KILL;
#endif
#ifdef CAP_LEASE
            static const Capability LEASE;
#endif
#ifdef CAP_LINUX_IMMUTABLE
            static const Capability LINUX_IMMUTABLE;
#endif
#ifdef CAP_MAC_ADMIN
            static const Capability MAC_ADMIN;
#endif
#ifdef CAP_MAC_OVERRIDE
            static const Capability MAC_OVERRIDE;
#endif
#ifdef CAP_MKNOD
            static const Capability MKNOD;
#endif
#ifdef CAP_NET_ADMIN
            static const Capability NET_ADMIN;
#endif
#ifdef CAP_NET_BIND_SERVICE
            static const Capability NET_BIND_SERVICE;
#endif
#ifdef CAP_NET_BROADCAST
            static const Capability NET_BROADCAST;
#endif
#ifdef CAP_NET_RAW
            static const Capability NET_RAW;
#endif
#ifdef CAP_PERFMON
            static const Capability PERFMON;
#endif
#ifdef CAP_SETGID
            static const Capability SETGID;
#endif
#ifdef CAP_SETFCAP
            static const Capability SETFCAP;
#endif
#ifdef CAP_SETPCAP
            static const Capability SETPCAP;
#endif
#ifdef CAP_SETUID
            static const Capability SETUID;
#endif
#ifdef CAP_SYS_ADMIN
            static const Capability SYS_ADMIN;
#endif
#ifdef CAP_SYS_BOOT
            static const Capability SYS_BOOT;
#endif
#ifdef CAP_SYS_CHROOT
            static const Capability SYS_CHROOT;
#endif
#ifdef CAP_SYS_MODULE
            static const Capability SYS_MODULE;
#endif
#ifdef CAP_SYS_NICE
            static const Capability SYS_NICE;
#endif
#ifdef CAP_SYS_PACCT
            static const Capability SYS_PACCT;
#endif
#ifdef CAP_SYS_PTRACE
            static const Capability SYS_PTRACE;
#endif
#ifdef CAP_SYS_RAWIO
            static const Capability SYS_RAWIO;
#endif
#ifdef CAP_SYS_RESOURCE
            static const Capability SYS_RESOURCE;
#endif
#ifdef CAP_SYS_TIME
            static const Capability SYS_TIME;
#endif
#ifdef CAP_SYS_TTY_CONFIG
            static const Capability SYS_TTY_CONFIG;
#endif
#ifdef CAP_SYSLOG
            static const Capability SYSLOG;
#endif
#ifdef CAP_WAKE_ALARM
            static const Capability WAKE_ALARM;
#endif

            std::optional<std::vector<Capability>> effective;
            std::optional<std::vector<Capability>> bounding;
            std::optional<std::vector<Capability>> inheritable;
            std::optional<std::vector<Capability>> permitted;
            std::optional<std::vector<Capability>> ambient;
        };

        // https://github.com/opencontainers/runtime-spec/blob/v1.0.2/config.md#user
        //
        // POSIX https://github.com/opencontainers/runtime-spec/blob/v1.0.2/config.md#posix-platform-user
        struct User {
            int uid; // POSIX
            int gid; // POSIX
            std::optional<int> umask; // POSIX
            std::optional<std::vector<int>> additionalGids; // POSIX
        };

        std::optional<bool> terminal;
        std::optional<ConsoleSize> consoleSize;
        std::filesystem::path cwd;
        std::optional<std::vector<std::string>> env;
        std::vector<std::string> args;
        std::optional<std::vector<Rlimit>> rlimits; // POSIX
        std::optional<std::string> apparmorProfile; // LINUX
        std::optional<Capabilities> capabilities; // LINUX
        std::optional<bool> noNewPrivileges; // LINUX
        std::optional<int> oomScoreAdj; // LINUX
        std::optional<std::string> selinuxLabel; // LINUX

        User user;

        void parse(const std::filesystem::path &bundlePath);
    };

    // POSIX https://github.com/opencontainers/runtime-spec/blob/v1.0.2/config.md#posix-platform-hooks
    struct Hooks {
        // POSIX
        struct Hook {
            std::filesystem::path path;
            std::optional<std::vector<std::string>> args;
            std::optional<std::vector<std::string>> env;
            std::optional<int> timeout;
        };

        // POSIX https://github.com/opencontainers/runtime-spec/blob/v1.0.2/config.md#prestart
        std::optional<std::vector<Hook>> prestart;
        // POSIX https://github.com/opencontainers/runtime-spec/blob/v1.0.2/config.md#createruntime-hooks
        std::optional<std::vector<Hook>> createRuntime;
        // POSIX https://github.com/opencontainers/runtime-spec/blob/v1.0.2/config.md#createcontainer-hooks
        std::optional<std::vector<Hook>> createContainer;
        // POSIX https://github.com/opencontainers/runtime-spec/blob/v1.0.2/config.md#startcontainer-hooks
        std::optional<std::vector<Hook>> startContainer;
        // POSIX https://github.com/opencontainers/runtime-spec/blob/v1.0.2/config.md#poststart
        std::optional<std::vector<Hook>> poststart;
        // POSIX https://github.com/opencontainers/runtime-spec/blob/v1.0.2/config.md#poststop
        std::optional<std::vector<Hook>> poststop;
    };

    // https://github.com/opencontainers/runtime-spec/blob/v1.0.2/config.md#annotations
    struct Annotations {
        // TODO: Document
        struct Rootfs {
            struct Overlayfs {
                std::filesystem::path lowerParent;
                std::filesystem::path upper;
                std::filesystem::path workdir;
                std::vector<Config::Mount> mounts;
            };

            struct Native {
                std::vector<Config::Mount> mounts;
            };

            // TODO: maybe this api should be change?
            struct DBus {
                std::filesystem::path host;
                std::filesystem::path container;
                std::filesystem::path config;
            };

            std::optional<Overlayfs> overlayfs;
            std::optional<Native> native;

            std::optional<std::vector<DBus>> dbus;

            std::optional<std::vector<IDMapping>> uidMappings;
            std::optional<std::vector<IDMapping>> gidMappings;
        };

        std::optional<Rootfs> rootfs; // NOT STANDER
        nlohmann::json raw;
    };

    // https://github.com/opencontainers/runtime-spec/blob/main/config-linux.md#namespaces
    struct Namespace {
        typedef std::string Type;

        static const Type PID;
        static const Type Network;
        static const Type Mount;
        static const Type IPC;
        static const Type UTS;
        static const Type User;
        static const Type Cgroup;

        Type type;
        std::optional<std::filesystem::path> path;

        void parse(const std::filesystem::path &bundlePath);
    };

    // https://github.com/opencontainers/runtime-spec/blob/main/config-linux.md#devices
    struct Device {
        typedef std::string Type;

        static const Type All;
        static const Type Char;
        static const Type Block;

        Type type;
        std::filesystem::path path;
        int64_t major;
        int64_t minor;
        std::optional<uint32_t> fileMode;
        std::optional<uint32_t> uid;
        std::optional<uint32_t> gid;
    };

    struct Resources {
        struct Device {
            bool allow;
            std::optional<Config::Device::Type> type;
            std::optional<int64_t> major;
            std::optional<int64_t> minor;
            std::optional<std::string> access;
        };

        struct Memory {
            std::optional<int64_t> limit;
            std::optional<int64_t> reservation;
            std::optional<int64_t> swap;
            std::optional<int64_t> kernel; // NOT RECOMMENDED
            std::optional<int64_t> kernelTCP; // NOT RECOMMENDED
            std::optional<uint64_t> swappiness;
            std::optional<bool> disableOOMKiller;
            std::optional<bool> useHierarchy;
        };

        struct CPU {
            std::optional<uint64_t> shares;
            std::optional<int64_t> quota;
            std::optional<uint64_t> period;
            std::optional<int64_t> realtimeRuntime;
            std::optional<uint64_t> realtimePeriod;
            std::optional<std::vector<std::string>> cpus;
            std::optional<std::vector<std::string>> mems;
            std::optional<int64_t> idle;
        };

        struct BlockIO {
            struct WeightDevice {
                int64_t major;
                int64_t minor;
                std::optional<uint16_t> weight;
                std::optional<uint16_t> leafWeight;
            };

            struct ThrottleIOPSDevice {
                int64_t major;
                int64_t minor;
                uint64_t rate;
            };

            std::optional<uint16_t> weight;
            std::optional<uint16_t> leafWeight;
            std::optional<std::vector<WeightDevice>> weightDevice;
            std::optional<std::vector<ThrottleIOPSDevice>> throttleReadIOPSDevice;
            std::optional<std::vector<ThrottleIOPSDevice>> throttleWriteIOPSDevice;
        };

        struct HugepageLimit {
            std::string pageSize;
            uint64_t limit;
        };

        struct Network {
            struct Priority {
                std::string name;
                uint32_t priority;
            };

            std::optional<uint32_t> classID;
            std::optional<std::vector<Priority>> priorities;
        };

        struct PIDs {
            int64_t limit;
        };

        struct RDMALimit {
            std::optional<uint32_t> hcaHandles;
            std::optional<uint32_t> hcaObjects;
        };

        std::optional<std::vector<Device>> devices;
        std::optional<Memory> memory;
        std::optional<CPU> cpu;
        std::optional<BlockIO> blockIO;
        std::optional<std::vector<HugepageLimit>> hugepageLimit;
        std::optional<Network> network;
        std::optional<PIDs> pids;
        std::optional<std::map<std::string, RDMALimit>> rdma;
    };

    struct Seccomp {
        typedef std::string Architecture;

#ifdef SCMP_ARCH_X86
        static const Architecture X86;
#endif
#ifdef SCMP_ARCH_X86_64
        static const Architecture X86_64;
#endif
#ifdef SCMP_ARCH_X32
        static const Architecture X32;
#endif
#ifdef SCMP_ARCH_ARM
        static const Architecture ARM;
#endif
#ifdef SCMP_ARCH_AARCH64
        static const Architecture AARCH64;
#endif
#ifdef SCMP_ARCH_MIPS
        static const Architecture MIPS;
#endif
#ifdef SCMP_ARCH_MIPS64
        static const Architecture MIPS64;
#endif
#ifdef SCMP_ARCH_MIPS64N32
        static const Architecture MIPS64N32;
#endif
#ifdef SCMP_ARCH_MIPSEL
        static const Architecture MIPSEL;
#endif
#ifdef SCMP_ARCH_MIPSEL64
        static const Architecture MIPSEL64;
#endif
#ifdef SCMP_ARCH_MIPSEL64N32
        static const Architecture MIPSEL64N32;
#endif
#ifdef SCMP_ARCH_PPC
        static const Architecture PPC;
#endif
#ifdef SCMP_ARCH_PPC64
        static const Architecture PPC64;
#endif
#ifdef SCMP_ARCH_PPC64LE
        static const Architecture PPC64LE;
#endif
#ifdef SCMP_ARCH_S390
        static const Architecture S390;
#endif
#ifdef SCMP_ARCH_S390X
        static const Architecture S390X;
#endif
#ifdef SCMP_ARCH_PARISC
        static const Architecture PARISC;
#endif
#ifdef SCMP_ARCH_PARISC64
        static const Architecture PARISC64;
#endif
#ifdef SCMP_ARCH_RISCV64
        static const Architecture RISCV64;
#endif

        typedef std::string Flag;

#ifdef SECCOMP_FILTER_FLAG_TSYNC
        static const Flag TSync;
#endif
#ifdef SECCOMP_FILTER_FLAG_LOG
        static const Flag FlagLog;
#endif
#ifdef SECCOMP_FILTER_FLAG_SPEC_ALLOW
        static const Flag SpecAllow;
#endif
#ifdef SECCOMP_FILTER_FLAG_NEW_LISTENER
        static const Flag NewListener;
#endif
#ifdef SECCOMP_FILTER_FLAG_TSYNC_ESRCH
        static const Flag TSyncESRCH;
#endif

        typedef std::string Action;

#ifdef SCMP_ACT_KILL_PROCESS
        static const Action KillProcess;
#endif
#ifdef SCMP_ACT_KILL_THREAD
        static const Action KillThread;
#endif
#ifdef SCMP_ACT_KILL
        static const Action Kill;
#endif
#ifdef SCMP_ACT_TRAP
        static const Action Trap;
#endif
#ifdef SCMP_ACT_NOTIFY
        static const Action Notify;
#endif
#ifdef SCMP_ACT_ERRNO
        static const Action Errno;
#endif
#ifdef SCMP_ACT_TRACE
        static const Action Trace;
#endif
#ifdef SCMP_ACT_LOG
        static const Action ActionLog;
#endif
#ifdef SCMP_ACT_ALLOW
        static const Action Allow;
#endif

        struct Syscall {
            struct Argument {
                typedef std::string Operator;

                static const Operator NotEqual;
                static const Operator LessThan;
                static const Operator LessThanOrEqual;
                static const Operator Equal;
                static const Operator GreaterThanOrEqual;
                static const Operator GreaterThan;
                static const Operator MaskedEquality;

                uint index;
                uint64_t value;
                std::optional<uint64_t> valueTwo;
                std::optional<Operator> op;
            };

            std::vector<std::string> names;
            Action action;
            std::optional<uint> errnoRet;
            std::optional<std::vector<Argument>> args;
        };

        Action defaultAction;
        std::optional<uint> defaultErrnoRet;
        std::optional<std::vector<Architecture>> architectures;
        std::optional<std::vector<Flag>> flags;
        std::optional<std::filesystem::path> listenerPath;
        std::optional<std::vector<Syscall>> syscalls;
    };

    typedef std::string MountPropagation;

    static const MountPropagation Share;
    static const MountPropagation Slave;
    static const MountPropagation Private;
    static const MountPropagation Unbindable;

    struct Personality {
        typedef std::string Domain;

        static const Domain Linux;
        static const Domain Linux32;

        typedef std::string Flag;

        Domain domain;
        std::optional<std::vector<Flag>> flags;
    };

    // https://github.com/opencontainers/runtime-spec/blob/v1.0.2/config.md#specification-version
    std::string ociVersion;

    Root root;
    std::optional<std::vector<Mount>> mounts;
    // https://github.com/opencontainers/runtime-spec/blob/v1.0.2/config.md#process
    std::optional<Process> process;
    std::optional<std::string> hostname;
    std::optional<Hooks> hooks; // POSIX
    std::optional<Annotations> annotations;

    std::vector<Namespace> namespaces; // LINUX
    std::optional<std::vector<IDMapping>> uidMappings; // LINUX
    std::optional<std::vector<IDMapping>> gidMappings; // LINUX
    std::optional<std::vector<Device>> devices; // LINUX
    std::optional<std::filesystem::path> cgroupsPath; // LINUX
    std::optional<Resources> resources; // LINUX
    std::optional<std::map<std::string, std::string>> unified; // LINUX
    // TODO: std::optional<IntelRdt> intelRdt // LINUX
    std::optional<std::map<std::string, std::string>> sysctl; // LINUX
    std::optional<Seccomp> seccomp; // LINUX
    std::optional<MountPropagation> rootfsPropagation; // LINUX
    std::optional<std::vector<std::filesystem::path>> maskedPaths; // LINUX
    std::optional<std::vector<std::filesystem::path>> readonlyPaths; // LINUX
    std::optional<std::string> mountLabel; // LINUX
    std::optional<Personality> personality; // LINUX

    void parse(const std::filesystem::path &bundlePath);
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Config::Root, path, readonly);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Config::IDMapping, containerID, hostID, size);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Config::Mount, type, uidMappings, gidMappings, destination, source,
                                                options);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Config::Process::Rlimit, type, soft, hard);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Config::Process::Capabilities, effective, bounding, inheritable,
                                                permitted, ambient);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Config::Process::ConsoleSize, height, width);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Config::Process::User, uid, gid, umask, additionalGids);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Config::Process, terminal, consoleSize, cwd, env, args, rlimits,
                                                apparmorProfile, capabilities, noNewPrivileges, oomScoreAdj,
                                                selinuxLabel, user);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Config::Hooks::Hook, path, args, env, timeout);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Config::Hooks, prestart, createRuntime, createContainer, startContainer,
                                                poststart, poststop);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Config::Annotations::Rootfs::Overlayfs, lowerParent, upper, workdir,
                                                mounts);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Config::Annotations::Rootfs::Native, mounts);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Config::Annotations::Rootfs::DBus, host, container, config);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Config::Annotations::Rootfs, overlayfs, native, dbus, uidMappings,
                                                gidMappings);

inline void to_json(nlohmann::json &j, const Config::Annotations &a)
{
    j = a.raw;
    j["rootfs"] = a.rootfs;
}

inline void from_json(const nlohmann::json &j, Config::Annotations &a)
{
    a.raw = j;
    j.at("rootfs").get_to(a.rootfs);
}

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Config::Namespace, type, path);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Config::Device, type, path, major, minor, fileMode, uid, gid);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Config::Resources::Device, allow, type, major, minor, access);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Config::Resources::Memory, limit, reservation, swap, kernel, kernelTCP,
                                                swappiness, disableOOMKiller, useHierarchy);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Config::Resources::CPU, shares, quota, period, realtimeRuntime,
                                                realtimePeriod, cpus, mems, idle);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Config::Resources::BlockIO::WeightDevice, major, minor, weight,
                                                leafWeight);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Config::Resources::BlockIO::ThrottleIOPSDevice, major, minor, rate);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Config::Resources::BlockIO, weight, leafWeight, weightDevice,
                                                throttleReadIOPSDevice, throttleWriteIOPSDevice);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Config::Resources::HugepageLimit, pageSize, limit);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Config::Resources::Network::Priority, name, priority);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Config::Resources::Network, classID, priorities);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Config::Resources::PIDs, limit);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Config::Resources::RDMALimit, hcaHandles, hcaObjects);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Config::Resources, devices, memory, cpu, blockIO, hugepageLimit,
                                                network, pids, rdma);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Config::Seccomp::Syscall::Argument, index, value, valueTwo, op);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Config::Seccomp::Syscall, names, action, errnoRet, args);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Config::Seccomp, defaultAction, defaultErrnoRet, architectures, flags,
                                                listenerPath, syscalls);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Config::Personality, domain, flags);

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Config, ociVersion, root, mounts, process, hostname, hooks, annotations,
                                                namespaces, uidMappings, gidMappings, devices, cgroupsPath, resources,
                                                unified, sysctl, seccomp, rootfsPropagation, maskedPaths, readonlyPaths,
                                                mountLabel, personality);
} // namespace linglong::OCI
#endif
