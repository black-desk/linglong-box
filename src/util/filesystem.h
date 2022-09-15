/*
 * Copyright (c) 2020-2021. Uniontech Software Ltd. All rights reserved.
 *
 * Author:     Iceyer <me@iceyer.net>
 *
 * Maintainer: Iceyer <me@iceyer.net>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef LINGLONG_BOX_SRC_UTIL_FILESYSTEM_H_
#define LINGLONG_BOX_SRC_UTIL_FILESYSTEM_H_

#include "util/fd.h"
#include <filesystem>
namespace linglong::util::fs {
void touch(std::filesystem::path path, __mode_t mode);
void mkdir(std::filesystem::path path, __mode_t mode);
void rmrf(std::filesystem::path path);

} // namespace linglong::util::fs

#endif /* LINGLONG_BOX_SRC_UTIL_FILESYSTEM_H_ */
