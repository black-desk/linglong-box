#include <filesystem>
#include <stdexcept>
#include <set>

#include <fmt/core.h>
#include <spdlog/spdlog.h>

#include <sys/mount.h>

#include "config.h"

#include "util/exception.h"
// #include "util/oci_runtime.h"
#include "util/semver2.h"

using linglong::util::SemVer;

static const SemVer llboxOciSemVersion("1.0.2");

namespace linglong::OCI {

void Config::parse(const std::filesystem::path &bundlePath)
{
    // FIXME: should parse each member in their sturct

    {
        util::SemVer ociSemVersion(this->ociVersion);

        if (!ociSemVersion.isCompatibleWith(llboxOciSemVersion)) {
            spdlog::warn("OCI Version of config.json \"{}\" not compatible with ll-box", ociVersion);
        }
    }

    {
        auto &root = this->root;
        root.parse(bundlePath);
    }

    {
        for (auto &mount : this->mounts.value_or(std::vector<Mount>())) {
            mount.parse(bundlePath);
        }
    }

    {
        if (this->process.has_value()) {
            auto &process = this->process.value();
            process.parse(bundlePath);
        } else {
            spdlog::warn("config.json doesn't have \"process\", this container cannot be started.");
        }
    }

    {
        if (this->hooks.has_value()) {
            spdlog::warn("\"hooks\" not support yet");
            // TODO: merge prestart and createRuntime
        }
    }

    {
        auto &namespaces = this->namespaces;
        std::set<Namespace::Type> set;

        for (auto &ns : namespaces) {
            if (set.find(ns.type) != set.end()) {
                throw util::RuntimeError(
                    fmt::format("config.json is not valid: namespace type \"{}\" duplicate", ns.type));
            }
            set.insert(ns.type);
            ns.parse(bundlePath);
        }

        // TODO: make sure always unshare User and Mount

    }

    {
        if (this->devices.has_value()) {
            auto &devices = this->devices.value();

            std::set<std::tuple<Device::Type, int64_t, int64_t>> deviceSet;

            for (auto &device : devices) {
                if (!device.path.is_absolute()) {
                    throw util::RuntimeError(fmt::format(
                        "config.json is not valid: device path \"{}\" is not a absolute path", device.path));
                }

                auto deviceTuple = std::make_tuple(device.type, device.major, device.minor);
                if (deviceSet.find(deviceTuple) != deviceSet.end()) {
                    throw util::RuntimeError(fmt::format(
                        "config.json is not valid: device (type=\"{}\", major={}, minor={}) used for multiple devices",
                        device.type, device.major, device.minor));
                } else {
                    deviceSet.insert(deviceTuple);
                }
            }
        }
    }

    {
        if (this->cgroupsPath.has_value()) {
            // TODO: support "cgroupsPath"
            spdlog::warn("\"cgroupsPath\" is not support yet.");
        }
    }

    {
        if (this->resources.has_value()) {
            // TODO: support "resources"
            spdlog::warn("\"resources\" is not support yet.");
        }
    }

    {
        if (this->unified.has_value()) {
            // TODO: support "unified"
            spdlog::warn("\"unified\" is not support yet.");
        }
    }

    {
        if (this->sysctl.has_value()) {
            // TODO: support "sysctl"
            spdlog::warn("\"sysctl\" is not support yet.");
        }
    }

    {
        if (this->seccomp.has_value()) {
            // TODO: support "seccomp"
            spdlog::warn("\"seccomp\" is not support yet.");
        }
    }

    {
        // FIXME: what is the default value?
        this->rootfsPropagation = this->rootfsPropagation.value_or(Config::Slave);
    }

    {
        if (this->maskedPaths.has_value()) {
            auto &maskedPaths = this->maskedPaths.value();
            for (auto &maskedPath : maskedPaths) {
                if (!maskedPath.is_absolute()) {
                    throw util::RuntimeError(fmt::format("maskedPath \"{}\" is not a absolute path.", maskedPath));
                }
            }
        }
    }

    {
        if (this->readonlyPaths.has_value()) {
            auto &readonlyPaths = this->readonlyPaths.value();
            for (auto &readonlyPath : readonlyPaths) {
                if (!readonlyPath.is_absolute()) {
                    throw util::RuntimeError(fmt::format("readonlyPath \"{}\" is not a absolute path.", readonlyPath));
                }
            }
        }
    }

    {
        if (this->mountLabel.has_value()) {
            // TODO: support "mountLabel"
            spdlog::warn("\"mountLabel\" is not support yet.");
        }
    }

    {
        if (this->personality) {
            // TODO: support "personality"
            spdlog::warn("\"personality\" is not support yet.");
        }
    }
}

void linglong::OCI::Config::Root::parse(std::filesystem::path bundlePath)
{
    this->readonly = this->readonly.value_or(false);
    if (this->path.is_relative()) {
        this->path = bundlePath / this->path;
    }
    if (!std::filesystem::exists(this->path)) {
        throw util::RuntimeError(fmt::format("config.json not valid: root.path (\"{}\") not exists.", this->path));
    }
}

void linglong::OCI::Config::Mount::parse(std::filesystem::path bundlePath)
{
    // NOTE: Maybe we should just use libmount.

    // TODO: support "propagation flags" and "recursive mount attrs"
    // https://github.com/opencontainers/runc/blob/c83abc503de7e8b3017276e92e7510064eee02a8/libcontainer/specconv/spec_linux.go#L958

    struct mountFlag {
        bool clear;
        uint32_t flag;
    };

    static std::map<std::string, mountFlag> mountOptionFlags = {
#ifdef MS_POSIXACL
        {"acl", {false, MS_POSIXACL}},
#endif
#ifdef MS_SYNCHRONOUS
        {"async", {true, MS_SYNCHRONOUS}},
#endif
#ifdef MS_NOATIME
        {"atime", {true, MS_NOATIME}},
#endif
#ifdef MS_BIND
        {"bind", {false, MS_BIND}},
#endif
        {"defaults", {false, 0}},
#ifdef MS_NODEV
        {"dev", {true, MS_NODEV}},
#endif
#ifdef MS_NODIRATIME
        {"diratime", {true, MS_NODIRATIME}},
#endif
#ifdef MS_DIRSYNC
        {"dirsync", {false, MS_DIRSYNC}},
#endif
#ifdef MS_NOEXEC
        {"exec", {true, MS_NOEXEC}},
#endif
#ifdef MS_I_VERSION
        {"iversion", {false, MS_I_VERSION}},
#endif
#ifdef MS_LAZYTIME
        {"lazytime", {false, MS_LAZYTIME}},
#endif
#ifdef MS_SILENT
        {"loud", {true, MS_SILENT}},
#endif
#ifdef MS_MANDLOCK
        {"mand", {false, MS_MANDLOCK}},
#endif
#ifdef MS_POSIXACL
        {"noacl", {true, MS_POSIXACL}},
#endif
#ifdef MS_NOATIME
        {"noatime", {false, MS_NOATIME}},
#endif
#ifdef MS_NODEV
        {"nodev", {false, MS_NODEV}},
#endif
#ifdef MS_NODIRATIME
        {"nodiratime", {false, MS_NODIRATIME}},
#endif
#ifdef MS_NOEXEC
        {"noexec", {false, MS_NOEXEC}},
#endif
#ifdef MS_I_VERSION
        {"noiversion", {true, MS_I_VERSION}},
#endif
#ifdef MS_LAZYTIME
        {"nolazytime", {true, MS_LAZYTIME}},
#endif
#ifdef MS_MANDLOCK
        {"nomand", {true, MS_MANDLOCK}},
#endif
#ifdef MS_RELATIME
        {"norelatime", {true, MS_RELATIME}},
#endif
#ifdef MS_STRICTATIME
        {"nostrictatime", {true, MS_STRICTATIME}},
#endif
#ifdef MS_NOSUID
        {"nosuid", {false, MS_NOSUID}},
#endif
#ifdef MS_NOSYMFOLLOW
        {"nosymfollow", {false, MS_NOSYMFOLLOW}},
#endif
#ifdef MS_BIND
#ifdef MS_REC
        {"rbind", {false, MS_BIND | MS_REC}},
#endif
#endif
#ifdef MS_RELATIME
        {"relatime", {false, MS_RELATIME}},
#endif
#ifdef MS_REMOUNT
        {"remount", {false, MS_REMOUNT}},
#endif
#ifdef MS_RDONLY
        {"ro", {false, MS_RDONLY}},
#endif
#ifdef MS_RDONLY
        {"rw", {true, MS_RDONLY}},
#endif
#ifdef MS_SILENT
        {"silent", {false, MS_SILENT}},
#endif
#ifdef MS_STRICTATIME
        {"strictatime", {false, MS_STRICTATIME}},
#endif
#ifdef MS_NOSUID
        {"suid", {true, MS_NOSUID}},
#endif
#ifdef MS_SYNCHRONOUS
        {"sync", {false, MS_SYNCHRONOUS}},
#endif
#ifdef MS_NOSYMFOLLOW
        {"symfollow", {true, MS_NOSYMFOLLOW}},
#endif
    };

    for (auto &option : this->options.value_or(std::vector<std::string>())) {
        auto it = mountOptionFlags.find(option);
        if (it != mountOptionFlags.end()) {
            if (it->second.clear) {
                this->parsed->flags &= ~it->second.flag;
            } else
                this->parsed->flags |= it->second.flag;
        } else {
            this->parsed->data.push_back(option);
        }
    }

    if (!this->destination.is_absolute()) {
        throw util::RuntimeError(fmt::format("mount destination \"{}\" is not a absolute path.", this->destination));
    }

    if (this->source.has_value() && this->source->is_relative()) {
        this->source = bundlePath / this->source.value();
    }
}

void Config::Process::parse(const std::filesystem::path &bundlePath)
{
    this->terminal = this->terminal.value_or(false);

    if (!this->terminal.value()) {
        this->consoleSize = std::nullopt;
    }

    if (!this->cwd.is_absolute()) {
        throw util::RuntimeError(fmt::format("config.json not valid: \"process.cwd\" is not a absolute path."));
    }

    if (!this->env.has_value()) {
        std::set<std::string> envSet;
        auto &environ = this->env.value();
        for (auto &env : environ) {
            auto pos = env.find("=");
            if (pos == std::string::npos) {
                throw util::RuntimeError(fmt::format(
                    "config.json not valid: \"process.env\" item \"{}\" is not a vaild environ string.", env));
            }
            auto key = env.substr(0, env.find("="));
            if (envSet.find(key) != envSet.end()) {
                spdlog::warn("config.json: \"process.env\" has duplicate key (\"{}\"). the consequences are undefined.",
                             key);
            } else {
                envSet.insert(key);
            }
        }
    }

    if (this->args.size() < 1) {
        throw util::RuntimeError(fmt::format("config.json not valid: \"process.args\" must have at least one entry."));
    }

    if (this->rlimits.has_value()) {
        // TODO: support "porcess.rlimits"
        spdlog::error("\"porcess.rlimits\" not support yet");
    }

    if (this->apparmorProfile.has_value()) {
        // TODO: support "porcess.apparmorProfile"
        spdlog::error("\"porcess.apparmorProfile\" not support yet");
    }

    if (this->capabilities.has_value()) {
        // TODO: support "process.capabilities"
        spdlog::error("\"process.capabilities\" not support yet");
    }

    if (this->noNewPrivileges.has_value()) {
        // TODO: support "process.noNewPrivileges"
        spdlog::error("\"process.noNewPrivileges\" not support yet");
    }

    if (this->oomScoreAdj.has_value()) {
        // TODO: support "process.oomScoreAdj"
        spdlog::error("\"process.oomScoreAdj\" not support yet");
    }

    if (this->selinuxLabel.has_value()) {
        // TODO: support "process.selinuxLabel"
        spdlog::error("\"process.selinuxLabel\" not support yet");
    }

    // FIXME: maybe we should take umask from "the calling this-> but not ourselves.
    // https://github.com/opencontainers/runtime-spec/blob/main/config.md#posix-platform-user
    this->user.umask = this->user.umask.value_or(umask(0777));
}

void Config::Namespace::parse(const std::filesystem::path &bundlePath)
{
    static std::set<Namespace::Type> namespaceTypeSet = {
        Namespace::PID, Namespace::Network, Namespace::Mount,  Namespace::IPC,
        Namespace::UTS, Namespace::User,    Namespace::Cgroup,
    };

    if (namespaceTypeSet.find(this->type) == namespaceTypeSet.end()) {
        throw util::RuntimeError(fmt::format("config.json is not valid: unknow namespace type \"{}\"", this->type));
    }
    if (this->path.has_value() && !this->path->is_absolute()) {
        throw util::RuntimeError(
            fmt::format("config.json is not valid: namespace path \"{}\" is not a absolute path.", this->path));
    }
}

} // namespace linglong::OCI
