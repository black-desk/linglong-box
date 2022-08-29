#include "config.h"

using linglong::OCI::Config;

const Config::Mount::Type Config::Mount::Bind = "bind";
const Config::Mount::Type Config::Mount::Proc = "proc";
const Config::Mount::Type Config::Mount::Sysfs = "sysfs";
const Config::Mount::Type Config::Mount::Devpts = "devpts";
const Config::Mount::Type Config::Mount::Mqueue = "mqueue";
const Config::Mount::Type Config::Mount::Tmpfs = "tmpfs";
const Config::Mount::Type Config::Mount::Cgroup = "cgroup";
const Config::Mount::Type Config::Mount::Cgroup2 = "cgroup2";

const Config::MountPropagation Config::Share = "share";
const Config::MountPropagation Config::Slave = "slave";
const Config::MountPropagation Config::Unbindable = "unbindable";
const Config::MountPropagation Config::Private = "private";

#ifdef RLIMIT_AS
const Config::Process::Rlimit::Type Config::Process::Rlimit::AS = "RLIMIT_AS";
#endif
#ifdef RLIMIT_CORE
const Config::Process::Rlimit::Type Config::Process::Rlimit::Core = "RLIMIT_CORE";
#endif
#ifdef RLIMIT_CPU
const Config::Process::Rlimit::Type Config::Process::Rlimit::CPU = "RLIMIT_CPU";
#endif
#ifdef RLIMIT_DATA
const Config::Process::Rlimit::Type Config::Process::Rlimit::Data = "RLIMIT_DATA";
#endif
#ifdef RLIMIT_FSIZE
const Config::Process::Rlimit::Type Config::Process::Rlimit::FSize = "RLIMIT_FSIZE";
#endif
#ifdef RLIMIT_LOCKS
const Config::Process::Rlimit::Type Config::Process::Rlimit::Locks = "RLIMIT_LOCKS";
#endif
#ifdef RLIMIT_MEMLOCK
const Config::Process::Rlimit::Type Config::Process::Rlimit::MemLock = "RLIMIT_MEMLOCK";
#endif
#ifdef RLIMIT_MSGQUEUE
const Config::Process::Rlimit::Type Config::Process::Rlimit::MsgQueue = "RLIMIT_MSGQUEUE";
#endif
#ifdef RLIMIT_NICE
const Config::Process::Rlimit::Type Config::Process::Rlimit::Nice = "RLIMIT_NICE";
#endif
#ifdef RLIMIT_NOFILE
const Config::Process::Rlimit::Type Config::Process::Rlimit::NOFile = "RLIMIT_NOFILE";
#endif
#ifdef RLIMIT_NPROC
const Config::Process::Rlimit::Type Config::Process::Rlimit::NProc = "RLIMIT_NPROC";
#endif
#ifdef RLIMIT_RSS
const Config::Process::Rlimit::Type Config::Process::Rlimit::RSS = "RLIMIT_RSS";
#endif
#ifdef RLIMIT_RTPRIO
const Config::Process::Rlimit::Type Config::Process::Rlimit::RTPrio = "RLIMIT_RTPRIO";
#endif
#ifdef RLIMIT_RTTIME
const Config::Process::Rlimit::Type Config::Process::Rlimit::RTTime = "RLIMIT_RTTIME";
#endif
#ifdef RLIMIT_SIGPENDING
const Config::Process::Rlimit::Type Config::Process::Rlimit::SigPending = "RLIMIT_SIGPENDING";
#endif
#ifdef RLIMIT_STACK
const Config::Process::Rlimit::Type Config::Process::Rlimit::Stack = "RLIMIT_STACK";
#endif

#ifdef CAP_AUDIT_CONTROL
const Config::Process::Capabilities::Capability AuditContol = "CAP_AUDIT_CONTROL";
#endif
#ifdef CAP_AUDIT_READ
const Config::Process::Capabilities::Capability AuditRead = "CAP_AUDIT_READ";
#endif
#ifdef CAP_AUDIT_WRITE
const Config::Process::Capabilities::Capability AuditWrite = "CAP_AUDIT_WRITE";
#endif
#ifdef CAP_BLOCK_SUSPEND
const Config::Process::Capabilities::Capability BlockSuspend = "CAP_BLOCK_SUSPEND";
#endif
#ifdef CAP_BPF
const Config::Process::Capabilities::Capability BPF = "CAP_BPF";
#endif
#ifdef CAP_CHECKPOINT_RESTORE
const Config::Process::Capabilities::Capability CheckpointRestore = "CAP_CHECKPOINT_RESTORE";
#endif
#ifdef CAP_CHOWN
const Config::Process::Capabilities::Capability Chown = "CAP_CHOWN";
#endif
#ifdef CAP_DAC_OVERRIDE
const Config::Process::Capabilities::Capability DACOverride = "CAP_DAC_OVERRIDE";
#endif
#ifdef CAP_DAC_READ_SEARCH
const Config::Process::Capabilities::Capability DACReadSearch = "CAP_DAC_READ_SEARCH";
#endif
#ifdef CAP_FOWNER
const Config::Process::Capabilities::Capability FOwner = "CAP_FOWNER";
#endif
#ifdef CAP_FSETID
const Config::Process::Capabilities::Capability FSetID = "CAP_FSETID";
#endif
#ifdef CAP_IPC_LOCK
const Config::Process::Capabilities::Capability IPCLock = "CAP_IPC_LOCK";
#endif
#ifdef CAP_IPC_OWNER
const Config::Process::Capabilities::Capability IPCOwner = "CAP_IPC_OWNER";
#endif
#ifdef CAP_KILL
const Config::Process::Capabilities::Capability Kill = "CAP_KILL";
#endif
#ifdef CAP_LEASE
const Config::Process::Capabilities::Capability Lease = "CAP_LEASE";
#endif
#ifdef CAP_LINUX_IMMUTABLE
const Config::Process::Capabilities::Capability LinuxImmutable = "CAP_LINUX_IMMUTABLE";
#endif
#ifdef CAP_MAC_ADMIN
const Config::Process::Capabilities::Capability MACAdmin = "CAP_MAC_ADMIN";
#endif
#ifdef CAP_MAC_OVERRIDE
const Config::Process::Capabilities::Capability MACOverride = "CAP_MAC_OVERRIDE";
#endif
#ifdef CAP_MKNOD
const Config::Process::Capabilities::Capability MkNod = "CAP_MKNOD";
#endif
#ifdef CAP_NET_ADMIN
const Config::Process::Capabilities::Capability NetAdmin = "CAP_NET_ADMIN";
#endif
#ifdef CAP_NET_BIND_SERVICE
const Config::Process::Capabilities::Capability NetBindService = "CAP_NET_BIND_SERVICE";
#endif
#ifdef CAP_NET_BROADCAST
const Config::Process::Capabilities::Capability NetBroadcast = "CAP_NET_BROADCAST";
#endif
#ifdef CAP_NET_RAW
const Config::Process::Capabilities::Capability NetRaw = "CAP_NET_RAW";
#endif
#ifdef CAP_PERFMON
const Config::Process::Capabilities::Capability PerfMon = "CAP_PERFMON";
#endif
#ifdef CAP_SETGID
const Config::Process::Capabilities::Capability SetGID = "CAP_SETGID";
#endif
#ifdef CAP_SETFCAP
const Config::Process::Capabilities::Capability SetFCap = "CAP_SETFCAP";
#endif
#ifdef CAP_SETPCAP
const Config::Process::Capabilities::Capability SetPCap = "CAP_SETPCAP";
#endif
#ifdef CAP_SETUID
const Config::Process::Capabilities::Capability SetUID = "CAP_SETUID";
#endif
#ifdef CAP_SYS_ADMIN
const Config::Process::Capabilities::Capability SysAdmin = "CAP_SYS_ADMIN";
#endif
#ifdef CAP_SYS_BOOT
const Config::Process::Capabilities::Capability SysBoot = "CAP_SYS_BOOT";
#endif
#ifdef CAP_SYS_CHROOT
const Config::Process::Capabilities::Capability SysChroot = "CAP_SYS_CHROOT";
#endif
#ifdef CAP_SYS_MODULE
const Config::Process::Capabilities::Capability SysModule = "CAP_SYS_MODULE";
#endif
#ifdef CAP_SYS_NICE
const Config::Process::Capabilities::Capability SysNice = "CAP_SYS_NICE";
#endif
#ifdef CAP_SYS_PACCT
const Config::Process::Capabilities::Capability SysPAcct = "CAP_SYS_PACCT";
#endif
#ifdef CAP_SYS_PTRACE
const Config::Process::Capabilities::Capability SysPTrace = "CAP_SYS_PTRACE";
#endif
#ifdef CAP_SYS_RAWIO
const Config::Process::Capabilities::Capability SysRawIO = "CAP_SYS_RAWIO";
#endif
#ifdef CAP_SYS_RESOURCE
const Config::Process::Capabilities::Capability SysResource = "CAP_SYS_RESOURCE";
#endif
#ifdef CAP_SYS_TIME
const Config::Process::Capabilities::Capability SysTime = "CAP_SYS_TIME";
#endif
#ifdef CAP_SYS_TTY_CONFIG
const Config::Process::Capabilities::Capability SysTTYConfig = "CAP_SYS_TTY_CONFIG";
#endif
#ifdef CAP_SYSLOG
const Config::Process::Capabilities::Capability SysLog = "CAP_SYSLOG";
#endif
#ifdef CAP_WAKE_ALARM
const Config::Process::Capabilities::Capability WakeAlarm = "CAP_WAKE_ALARM";
#endif
