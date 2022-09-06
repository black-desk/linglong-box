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
        enum Type { Bind, Proc, Sysfs, Devpts, Mqueue, Tmpfs, Cgroup, Cgroup2 };

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
            unsigned short int height;
            unsigned short int width;
        };

        struct Rlimit {
            enum Type {
#ifdef RLIMIT_AS
                AS,
#endif
#ifdef RLIMIT_CORE
                Core,
#endif
#ifdef RLIMIT_CPU
                CPU,
#endif
#ifdef RLIMIT_DATA
                Data,
#endif
#ifdef RLIMIT_FSIZE
                FSize,
#endif
#ifdef RLIMIT_LOCKS
                Locks,
#endif
#ifdef RLIMIT_MEMLOCK
                MemLock,
#endif
#ifdef RLIMIT_MSGQUEUE
                MsgQueue,
#endif
#ifdef RLIMIT_NICE
                Nice,
#endif
#ifdef RLIMIT_NOFILE
                NOFile,
#endif
#ifdef RLIMIT_NPROC
                NProc,
#endif
#ifdef RLIMIT_RSS
                RSS,
#endif
#ifdef RLIMIT_RTPRIO
                RTPrio,
#endif
#ifdef RLIMIT_RTTIME
                RTTime,
#endif
#ifdef RLIMIT_SIGPENDING
                SigPending,
#endif
#ifdef RLIMIT_STACK
                Stack,
#endif
            };

            Type type;
            uint64_t soft;
            uint64_t hard;
        };

        struct Capabilities {
            enum Capability {
#ifdef CAP_AUDIT_CONTROL
                AUDIT_CONTROL,
#endif
#ifdef CAP_AUDIT_READ
                AUDIT_READ,
#endif
#ifdef CAP_AUDIT_WRITE
                AUDIT_WRITE,
#endif
#ifdef CAP_BLOCK_SUSPEND
                BLOCK_SUSPEND,
#endif
#ifdef CAP_BPF
                BPF,
#endif
#ifdef CAP_CHECKPOINT_RESTORE
                CHECKPOINT_RESTORE,
#endif
#ifdef CAP_CHOWN
                CHOWN,
#endif
#ifdef CAP_DAC_OVERRIDE
                DAC_OVERRIDE,
#endif
#ifdef CAP_DAC_READ_SEARCH
                DAC_READ_SEARCH,
#endif
#ifdef CAP_FOWNER
                FOWNER,
#endif
#ifdef CAP_FSETID
                FSETID,
#endif
#ifdef CAP_IPC_LOCK
                IPC_LOCK,
#endif
#ifdef CAP_IPC_OWNER
                IPC_OWNER,
#endif
#ifdef CAP_KILL
                KILL,
#endif
#ifdef CAP_LEASE
                LEASE,
#endif
#ifdef CAP_LINUX_IMMUTABLE
                LINUX_IMMUTABLE,
#endif
#ifdef CAP_MAC_ADMIN
                MAC_ADMIN,
#endif
#ifdef CAP_MAC_OVERRIDE
                MAC_OVERRIDE,
#endif
#ifdef CAP_MKNOD
                MKNOD,
#endif
#ifdef CAP_NET_ADMIN
                NET_ADMIN,
#endif
#ifdef CAP_NET_BIND_SERVICE
                NET_BIND_SERVICE,
#endif
#ifdef CAP_NET_BROADCAST
                NET_BROADCAST,
#endif
#ifdef CAP_NET_RAW
                NET_RAW,
#endif
#ifdef CAP_PERFMON
                PERFMON,
#endif
#ifdef CAP_SETGID
                SETGID,
#endif
#ifdef CAP_SETFCAP
                SETFCAP,
#endif
#ifdef CAP_SETPCAP
                SETPCAP,
#endif
#ifdef CAP_SETUID
                SETUID,
#endif
#ifdef CAP_SYS_ADMIN
                SYS_ADMIN,
#endif
#ifdef CAP_SYS_BOOT
                SYS_BOOT,
#endif
#ifdef CAP_SYS_CHROOT
                SYS_CHROOT,
#endif
#ifdef CAP_SYS_MODULE
                SYS_MODULE,
#endif
#ifdef CAP_SYS_NICE
                SYS_NICE,
#endif
#ifdef CAP_SYS_PACCT
                SYS_PACCT,
#endif
#ifdef CAP_SYS_PTRACE
                SYS_PTRACE,
#endif
#ifdef CAP_SYS_RAWIO
                SYS_RAWIO,
#endif
#ifdef CAP_SYS_RESOURCE
                SYS_RESOURCE,
#endif
#ifdef CAP_SYS_TIME
                SYS_TIME,
#endif
#ifdef CAP_SYS_TTY_CONFIG
                SYS_TTY_CONFIG,
#endif
#ifdef CAP_SYSLOG
                SYSLOG,
#endif
#ifdef CAP_WAKE_ALARM
                WAKE_ALARM,
#endif
            };

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
                std::vector<std::filesystem::path> lower;
                std::filesystem::path upper;
                std::filesystem::path workdir;
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
        enum Type {
            PID = CLONE_NEWPID,
            Network = CLONE_NEWNET,
            Mount = CLONE_NEWNS,
            IPC = CLONE_NEWIPC,
            UTS = CLONE_NEWUTS,
            User = CLONE_NEWUSER,
            Cgroup = CLONE_NEWCGROUP
        };

        Type type;
        std::optional<std::filesystem::path> path;

        void parse(const std::filesystem::path &bundlePath);
    };

    // https://github.com/opencontainers/runtime-spec/blob/main/config-linux.md#devices
    struct Device {
        enum Type { All, Char, Block };

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
        enum Architecture {
#ifdef SCMP_ARCH_X86
            X86 = SCMP_ARCH_X86,
#endif
#ifdef SCMP_ARCH_X86_64
            X86_64 = SCMP_ARCH_X86_64,
#endif
#ifdef SCMP_ARCH_X32
            X32 = SCMP_ARCH_X32,
#endif
#ifdef SCMP_ARCH_ARM
            ARM = SCMP_ARCH_ARM,
#endif
#ifdef SCMP_ARCH_AARCH64
            AARCH64 = SCMP_ARCH_AARCH64,
#endif
#ifdef SCMP_ARCH_MIPS
            MIPS = SCMP_ARCH_MIPS,
#endif
#ifdef SCMP_ARCH_MIPS64
            MIPS64 = SCMP_ARCH_MIPS64,
#endif
#ifdef SCMP_ARCH_MIPS64N32
            MIPS64N32 = SCMP_ARCH_MIPS64N32,
#endif
#ifdef SCMP_ARCH_MIPSEL
            MIPSEL = SCMP_ARCH_MIPSEL,
#endif
#ifdef SCMP_ARCH_MIPSEL64
            MIPSEL64 = SCMP_ARCH_MIPSEL64,
#endif
#ifdef SCMP_ARCH_MIPSEL64N32
            MIPSEL64N32 = SCMP_ARCH_MIPSEL64N32,
#endif
#ifdef SCMP_ARCH_PPC
            PPC = SCMP_ARCH_PPC,
#endif
#ifdef SCMP_ARCH_PPC64
            PPC64 = SCMP_ARCH_PPC64,
#endif
#ifdef SCMP_ARCH_PPC64LE
            PPC64LE = SCMP_ARCH_PPC64LE,
#endif
#ifdef SCMP_ARCH_S390
            S390 = SCMP_ARCH_S390,
#endif
#ifdef SCMP_ARCH_S390X
            S390X = SCMP_ARCH_S390X,
#endif
#ifdef SCMP_ARCH_PARISC
            PARISC = SCMP_ARCH_PARISC,
#endif
#ifdef SCMP_ARCH_PARISC64
            PARISC64 = SCMP_ARCH_PARISC64,
#endif
#ifdef SCMP_ARCH_RISCV64
            RISCV64 = SCMP_ARCH_RISCV64,
#endif
        };

        enum Flag {
#ifdef SECCOMP_FILTER_FLAG_TSYNC
            TSync = SECCOMP_FILTER_FLAG_TSYNC,
#endif
#ifdef SECCOMP_FILTER_FLAG_LOG
            Log = SECCOMP_FILTER_FLAG_LOG,
#endif
#ifdef SECCOMP_FILTER_FLAG_SPEC_ALLOW
            SpecAllow = SECCOMP_FILTER_FLAG_SPEC_ALLOW,
#endif
#ifdef SECCOMP_FILTER_FLAG_NEW_LISTENER
            NewListener = SECCOMP_FILTER_FLAG_NEW_LISTENER,
#endif
#ifdef SECCOMP_FILTER_FLAG_TSYNC_ESRCH
            TSyncESRCH = SECCOMP_FILTER_FLAG_TSYNC_ESRCH,
#endif
        };

        enum Action {
#ifdef SCMP_ACT_KILL_PROCESS
            KillProcess = SCMP_ACT_KILL_PROCESS,
#endif
#ifdef SCMP_ACT_KILL_THREAD
            KillThread = SCMP_ACT_KILL_THREAD,
#endif
#ifdef SCMP_ACT_KILL
            Kill = SCMP_ACT_KILL,
#endif
#ifdef SCMP_ACT_TRAP
            Trap = SCMP_ACT_TRAP,
#endif
#ifdef SCMP_ACT_NOTIFY
            Notify = SCMP_ACT_NOTIFY,
#endif
#ifdef SCMP_ACT_ERRNO
            Errno = SCMP_ACT_ERRNO(0),
#endif
#ifdef SCMP_ACT_TRACE
            Trace = SCMP_ACT_TRACE(0),
#endif
#ifdef SCMP_ACT_LOG
            ActionLog = SCMP_ACT_LOG,
#endif
#ifdef SCMP_ACT_ALLOW
            Allow = SCMP_ACT_ALLOW,
#endif
        };

        struct Syscall {
            struct Argument {
                enum Operator {
                    NotEqual = SCMP_CMP_NE,
                    LessThan = SCMP_CMP_LT,
                    LessThanOrEqual = SCMP_CMP_LE,
                    Equal = SCMP_CMP_EQ,
                    GreaterThanOrEqual = SCMP_CMP_GE,
                    GreaterThan = SCMP_CMP_GE,
                    MaskedEquality = SCMP_CMP_MASKED_EQ,
                };

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

    enum MountPropagation { Share, Slave, Private, Unbindable };

    struct Personality {
        enum Domain {
            Linux,
            Linux32,
        };

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
#define JSON_DISABLE_ENUM_SERIALIZATION 1
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Config::Root, path, readonly);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Config::IDMapping, containerID, hostID, size);
NLOHMANN_JSON_SERIALIZE_ENUM(Config::Mount::Type, {
                                                      {Config::Mount::Type::Bind, "bind"},
                                                      {Config::Mount::Type::Proc, "proc"},
                                                      {Config::Mount::Type::Sysfs, "sysfs"},
                                                      {Config::Mount::Type::Devpts, "devpts"},
                                                      {Config::Mount::Type::Mqueue, "mqueue"},
                                                      {Config::Mount::Type::Tmpfs, "tmpfs"},
                                                      {Config::Mount::Type::Cgroup, "cgroup"},
                                                      {Config::Mount::Type::Cgroup2, "cgroup2"},
                                                  });
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Config::Mount, type, uidMappings, gidMappings, destination, source,
                                                options);

NLOHMANN_JSON_SERIALIZE_ENUM(Config::Process::Rlimit::Type,
                             {
#ifdef RLIMIT_AS
                                 {Config::Process::Rlimit::Type::AS, "RLIMIT_AS"},
#endif
#ifdef RLIMIT_CORE
                                 {Config::Process::Rlimit::Type::Core, "RLIMIT_CORE"},
#endif
#ifdef RLIMIT_CPU
                                 {Config::Process::Rlimit::Type::CPU, "RLIMIT_CPU"},
#endif
#ifdef RLIMIT_DATA
                                 {Config::Process::Rlimit::Type::Data, "RLIMIT_DATA"},
#endif
#ifdef RLIMIT_FSIZE
                                 {Config::Process::Rlimit::Type::FSize, "RLIMIT_FSIZE"},
#endif
#ifdef RLIMIT_LOCKS
                                 {Config::Process::Rlimit::Type::Locks, "RLIMIT_LOCKS"},
#endif
#ifdef RLIMIT_MEMLOCK
                                 {Config::Process::Rlimit::Type::MemLock, "RLIMIT_MEMLOCK"},
#endif
#ifdef RLIMIT_MSGQUEUE
                                 {Config::Process::Rlimit::Type::MsgQueue, "RLIMIT_MSGQUEUE"},
#endif
#ifdef RLIMIT_NICE
                                 {Config::Process::Rlimit::Type::Nice, "RLIMIT_NICE"},
#endif
#ifdef RLIMIT_NOFILE
                                 {Config::Process::Rlimit::Type::NOFile, "RLIMIT_NOFILE"},
#endif
#ifdef RLIMIT_NPROC
                                 {Config::Process::Rlimit::Type::NProc, "RLIMIT_NPROC"},
#endif
#ifdef RLIMIT_RSS
                                 {Config::Process::Rlimit::Type::RSS, "RLIMIT_RSS"},
#endif
#ifdef RLIMIT_RTPRIO
                                 {Config::Process::Rlimit::Type::RTPrio, "RLIMIT_RTPRIO"},
#endif
#ifdef RLIMIT_RTTIME
                                 {Config::Process::Rlimit::Type::RTTime, "RLIMIT_RTTIME"},
#endif
#ifdef RLIMIT_SIGPENDING
                                 {Config::Process::Rlimit::Type::SigPending, "RLIMIT_SIGPENDING"},
#endif
#ifdef RLIMIT_STACK
                                 {Config::Process::Rlimit::Type::Stack, "RLIMIT_STACK"},
#endif
                             });
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Config::Process::Rlimit, type, soft, hard);
NLOHMANN_JSON_SERIALIZE_ENUM(Config::Process::Capabilities::Capability,
                             {
#ifdef CAP_AUDIT_CONTROL
                                 {Config::Process::Capabilities::Capability::AUDIT_CONTROL, "CAP_AUDIT_CONTROL"},
#endif
#ifdef CAP_AUDIT_READ
                                 {Config::Process::Capabilities::Capability::AUDIT_READ, "CAP_AUDIT_READ"},
#endif
#ifdef CAP_AUDIT_WRITE
                                 {Config::Process::Capabilities::Capability::AUDIT_WRITE, "CAP_AUDIT_WRITE"},
#endif
#ifdef CAP_BLOCK_SUSPEND
                                 {Config::Process::Capabilities::Capability::BLOCK_SUSPEND, "CAP_BLOCK_SUSPEND"},
#endif
#ifdef CAP_BPF
                                 {Config::Process::Capabilities::Capability::BPF, "CAP_BPF"},
#endif
                                 {Config::Process::Capabilities::Capability::CHECKPOINT_RESTORE,
#ifdef CAP_CHECKPOINT_RESTORE
                                  "CAP_CHECKPOINT_RESTORE"},
#endif
#ifdef CAP_CHOWN
                                 {Config::Process::Capabilities::Capability::CHOWN, "CAP_CHOWN"},
#endif
#ifdef CAP_DAC_OVERRIDE
                                 {Config::Process::Capabilities::Capability::DAC_OVERRIDE, "CAP_DAC_OVERRIDE"},
#endif
#ifdef CAP_DAC_READ_SEARCH
                                 {Config::Process::Capabilities::Capability::DAC_READ_SEARCH, "CAP_DAC_READ_SEARCH"},
#endif
#ifdef CAP_FOWNER
                                 {Config::Process::Capabilities::Capability::FOWNER, "CAP_FOWNER"},
#endif
#ifdef CAP_FSETID
                                 {Config::Process::Capabilities::Capability::FSETID, "CAP_FSETID"},
#endif
#ifdef CAP_IPC_LOCK
                                 {Config::Process::Capabilities::Capability::IPC_LOCK, "CAP_IPC_LOCK"},
#endif
#ifdef CAP_IPC_OWNER
                                 {Config::Process::Capabilities::Capability::IPC_OWNER, "CAP_IPC_OWNER"},
#endif
#ifdef CAP_KILL
                                 {Config::Process::Capabilities::Capability::KILL, "CAP_KILL"},
#endif
#ifdef CAP_LEASE
                                 {Config::Process::Capabilities::Capability::LEASE, "CAP_LEASE"},
#endif
#ifdef CAP_LINUX_IMMUTABLE
                                 {Config::Process::Capabilities::Capability::LINUX_IMMUTABLE, "CAP_LINUX_IMMUTABLE"},
#endif
#ifdef CAP_MAC_ADMIN
                                 {Config::Process::Capabilities::Capability::MAC_ADMIN, "CAP_MAC_ADMIN"},
#endif
#ifdef CAP_MAC_OVERRIDE
                                 {Config::Process::Capabilities::Capability::MAC_OVERRIDE, "CAP_MAC_OVERRIDE"},
#endif
#ifdef CAP_MKNOD
                                 {Config::Process::Capabilities::Capability::MKNOD, "CAP_MKNOD"},
#endif
#ifdef CAP_NET_ADMIN
                                 {Config::Process::Capabilities::Capability::NET_ADMIN, "CAP_NET_ADMIN"},
#endif
#ifdef CAP_NET_BIND_SERVICE
                                 {Config::Process::Capabilities::Capability::NET_BIND_SERVICE, "CAP_NET_BIND_SERVICE"},
#endif
#ifdef CAP_NET_BROADCAST
                                 {Config::Process::Capabilities::Capability::NET_BROADCAST, "CAP_NET_BROADCAST"},
#endif
#ifdef CAP_NET_RAW
                                 {Config::Process::Capabilities::Capability::NET_RAW, "CAP_NET_RAW"},
#endif
#ifdef CAP_PERFMON
                                 {Config::Process::Capabilities::Capability::PERFMON, "CAP_PERFMON"},
#endif
#ifdef CAP_SETGID
                                 {Config::Process::Capabilities::Capability::SETGID, "CAP_SETGID"},
#endif
#ifdef CAP_SETFCAP
                                 {Config::Process::Capabilities::Capability::SETFCAP, "CAP_SETFCAP"},
#endif
#ifdef CAP_SETPCAP
                                 {Config::Process::Capabilities::Capability::SETPCAP, "CAP_SETPCAP"},
#endif
#ifdef CAP_SETUID
                                 {Config::Process::Capabilities::Capability::SETUID, "CAP_SETUID"},
#endif
#ifdef CAP_SYS_ADMIN
                                 {Config::Process::Capabilities::Capability::SYS_ADMIN, "CAP_SYS_ADMIN"},
#endif
#ifdef CAP_SYS_BOOT
                                 {Config::Process::Capabilities::Capability::SYS_BOOT, "CAP_SYS_BOOT"},
#endif
#ifdef CAP_SYS_CHROOT
                                 {Config::Process::Capabilities::Capability::SYS_CHROOT, "CAP_SYS_CHROOT"},
#endif
#ifdef CAP_SYS_MODULE
                                 {Config::Process::Capabilities::Capability::SYS_MODULE, "CAP_SYS_MODULE"},
#endif
#ifdef CAP_SYS_NICE
                                 {Config::Process::Capabilities::Capability::SYS_NICE, "CAP_SYS_NICE"},
#endif
#ifdef CAP_SYS_PACCT
                                 {Config::Process::Capabilities::Capability::SYS_PACCT, "CAP_SYS_PACCT"},
#endif
#ifdef CAP_SYS_PTRACE
                                 {Config::Process::Capabilities::Capability::SYS_PTRACE, "CAP_SYS_PTRACE"},
#endif
#ifdef CAP_SYS_RAWIO
                                 {Config::Process::Capabilities::Capability::SYS_RAWIO, "CAP_SYS_RAWIO"},
#endif
#ifdef CAP_SYS_RESOURCE
                                 {Config::Process::Capabilities::Capability::SYS_RESOURCE, "CAP_SYS_RESOURCE"},
#endif
#ifdef CAP_SYS_TIME
                                 {Config::Process::Capabilities::Capability::SYS_TIME, "CAP_SYS_TIME"},
#endif
#ifdef CAP_SYS_TTY_CONFIG
                                 {Config::Process::Capabilities::Capability::SYS_TTY_CONFIG, "CAP_SYS_TTY_CONFIG"},
#endif
#ifdef CAP_SYSLOG
                                 {Config::Process::Capabilities::Capability::SYSLOG, "CAP_SYSLOG"},
#endif
#ifdef CAP_WAKE_ALARM
                                 {Config::Process::Capabilities::Capability::WAKE_ALARM, "CAP_WAKE_ALARM"},
#endif
                             });
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
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Config::Annotations::Rootfs::Overlayfs, lower, upper, workdir);
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
NLOHMANN_JSON_SERIALIZE_ENUM(Config::Namespace::Type, {
                                                          {Config::Namespace::Type::PID, "pid"},
                                                          {Config::Namespace::Type::Network, "network"},
                                                          {Config::Namespace::Type::Mount, "mount"},
                                                          {Config::Namespace::Type::IPC, "ipc"},
                                                          {Config::Namespace::Type::UTS, "uts"},
                                                          {Config::Namespace::Type::User, "user"},
                                                          {Config::Namespace::Type::Cgroup, "cgroup"},
                                                      });
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Config::Namespace, type, path);
NLOHMANN_JSON_SERIALIZE_ENUM(Config::Device::Type, {
                                                       {Config::Device::Type::All, "a"},
                                                       {Config::Device::Type::Block, "b"},
                                                       {Config::Device::Type::Char, "c"},
                                                   });
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
NLOHMANN_JSON_SERIALIZE_ENUM(Config::Seccomp::Syscall::Argument::Operator,
                             {
                                 {Config::Seccomp::Syscall::Argument::Operator::NotEqual, "SCMP_CMP_NE"},
                                 {Config::Seccomp::Syscall::Argument::Operator::LessThan, "SCMP_CMP_LT"},
                                 {Config::Seccomp::Syscall::Argument::Operator::LessThanOrEqual, "SCMP_CMP_LE"},
                                 {Config::Seccomp::Syscall::Argument::Operator::Equal, "SCMP_CMP_EQ"},
                                 {Config::Seccomp::Syscall::Argument::Operator::GreaterThanOrEqual, "SCMP_CMP_GE"},
                                 {Config::Seccomp::Syscall::Argument::Operator::GreaterThan, "SCMP_CMP_GE"},
                                 {Config::Seccomp::Syscall::Argument::Operator::MaskedEquality, "SCMP_CMP_MASKED_EQ"},
                             });
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Config::Seccomp::Syscall::Argument, index, value, valueTwo, op);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Config::Seccomp::Syscall, names, action, errnoRet, args);
NLOHMANN_JSON_SERIALIZE_ENUM(Config::Seccomp::Architecture,
                             {
#ifdef SCMP_ARCH_X86
                                 {Config::Seccomp::Architecture::X86, "SCMP_ARCH_X86"},
#endif
#ifdef SCMP_ARCH_X86_64
                                 {Config::Seccomp::Architecture::X86_64, "SCMP_ARCH_X86_64"},
#endif
#ifdef SCMP_ARCH_X32
                                 {Config::Seccomp::Architecture::X32, "SCMP_ARCH_X32"},
#endif
#ifdef SCMP_ARCH_ARM
                                 {Config::Seccomp::Architecture::ARM, "SCMP_ARCH_ARM"},
#endif
#ifdef SCMP_ARCH_AARCH64
                                 {Config::Seccomp::Architecture::AARCH64, "SCMP_ARCH_AARCH64"},
#endif
#ifdef SCMP_ARCH_MIPS
                                 {Config::Seccomp::Architecture::MIPS, "SCMP_ARCH_MIPS"},
#endif
#ifdef SCMP_ARCH_MIPS64
                                 {Config::Seccomp::Architecture::MIPS64, "SCMP_ARCH_MIPS64"},
#endif
#ifdef SCMP_ARCH_MIPS64N32
                                 {Config::Seccomp::Architecture::MIPS64N32, "SCMP_ARCH_MIPS64N32"},
#endif
#ifdef SCMP_ARCH_MIPSEL
                                 {Config::Seccomp::Architecture::MIPSEL, "SCMP_ARCH_MIPSEL"},
#endif
#ifdef SCMP_ARCH_MIPSEL64
                                 {Config::Seccomp::Architecture::MIPSEL64, "SCMP_ARCH_MIPSEL64"},
#endif
#ifdef SCMP_ARCH_MIPSEL64N32
                                 {Config::Seccomp::Architecture::MIPSEL64N32, "SCMP_ARCH_MIPSEL64N32"},
#endif
#ifdef SCMP_ARCH_PPC
                                 {Config::Seccomp::Architecture::PPC, "SCMP_ARCH_PPC"},
#endif
#ifdef SCMP_ARCH_PPC64
                                 {Config::Seccomp::Architecture::PPC64, "SCMP_ARCH_PPC64"},
#endif
#ifdef SCMP_ARCH_PPC64LE
                                 {Config::Seccomp::Architecture::PPC64LE, "SCMP_ARCH_PPC64LE"},
#endif
#ifdef SCMP_ARCH_S390
                                 {Config::Seccomp::Architecture::S390, "SCMP_ARCH_S390"},
#endif
#ifdef SCMP_ARCH_S390X
                                 {Config::Seccomp::Architecture::S390X, "SCMP_ARCH_S390X"},
#endif
#ifdef SCMP_ARCH_PARISC
                                 {Config::Seccomp::Architecture::PARISC, "SCMP_ARCH_PARISC"},
#endif
#ifdef SCMP_ARCH_PARISC64
                                 {Config::Seccomp::Architecture::PARISC64, "SCMP_ARCH_PARISC64"},
#endif
#ifdef SCMP_ARCH_RISCV64
                                 {Config::Seccomp::Architecture::RISCV64, "SCMP_ARCH_RISCV64"},
#endif
                             });
NLOHMANN_JSON_SERIALIZE_ENUM(Config::Seccomp::Flag,
                             {
#ifdef SECCOMP_FILTER_FLAG_TSYNC
                                 {Config::Seccomp::Flag::TSync, "SECCOMP_FILTER_FLAG_TSYNC"},
#endif
#ifdef SECCOMP_FILTER_FLAG_LOG
                                 {Config::Seccomp::Flag::Log, "SECCOMP_FILTER_FLAG_LOG"},
#endif
#ifdef SECCOMP_FILTER_FLAG_SPEC_ALLOW
                                 {Config::Seccomp::Flag::SpecAllow, "SECCOMP_FILTER_FLAG_SPEC_ALLOW"},
#endif
#ifdef SECCOMP_FILTER_FLAG_NEW_LISTENER
                                 {Config::Seccomp::Flag::NewListener, "SECCOMP_FILTER_FLAG_NEW_LISTENER"},
#endif
#ifdef SECCOMP_FILTER_FLAG_TSYNC_ESRCH
                                 {Config::Seccomp::Flag::TSyncESRCH, "SECCOMP_FILTER_FLAG_TSYNC_ESRCH"},
#endif
                             });
NLOHMANN_JSON_SERIALIZE_ENUM(Config::Seccomp::Action,
                             {
#ifdef SCMP_ACT_KILL_PROCESS
                                 {Config::Seccomp::Action::KillProcess, "SCMP_ACT_KILL_PROCESS"},
#endif
#ifdef SCMP_ACT_KILL_THREAD
                                 {Config::Seccomp::Action::KillThread, "SCMP_ACT_KILL_THREAD"},
#endif
#ifdef SCMP_ACT_KILL
                                 {Config::Seccomp::Action::Kill, "SCMP_ACT_KILL"},
#endif
#ifdef SCMP_ACT_TRAP
                                 {Config::Seccomp::Action::Trap, "SCMP_ACT_TRAP"},
#endif
#ifdef SCMP_ACT_NOTIFY
                                 {Config::Seccomp::Action::Notify, "SCMP_ACT_NOTIFY"},
#endif
#ifdef SCMP_ACT_ERRNO
                                 {Config::Seccomp::Action::Errno, "SCMP_ACT_ERRNO(0)"},
#endif
#ifdef SCMP_ACT_TRACE
                                 {Config::Seccomp::Action::Trace, "SCMP_ACT_TRACE(0)"},
#endif
#ifdef SCMP_ACT_LOG
                                 {Config::Seccomp::Action::ActionLog, "SCMP_ACT_LOG"},
#endif
#ifdef SCMP_ACT_ALLOW
                                 {Config::Seccomp::Action::Allow, "SCMP_ACT_ALLOW"},
#endif
                             });
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Config::Seccomp, defaultAction, defaultErrnoRet, architectures, flags,
                                                listenerPath, syscalls);
NLOHMANN_JSON_SERIALIZE_ENUM(Config::Personality::Domain, {{Config::Personality::Domain::Linux, "LINUX"},
                                                           {Config::Personality::Domain::Linux32, "LINUX32"}});
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Config::Personality, domain, flags);
NLOHMANN_JSON_SERIALIZE_ENUM(Config::MountPropagation, {
                                                           {Config::MountPropagation::Private, "private"},
                                                           {Config::MountPropagation::Share, "share"},
                                                           {Config::MountPropagation::Slave, "slare"},
                                                           {Config::MountPropagation::Unbindable, "unbindable"},
                                                       });
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Config, ociVersion, root, mounts, process, hostname, hooks, annotations,
                                                namespaces, uidMappings, gidMappings, devices, cgroupsPath, resources,
                                                unified, sysctl, seccomp, rootfsPropagation, maskedPaths, readonlyPaths,
                                                mountLabel, personality);
} // namespace linglong::OCI
#endif
