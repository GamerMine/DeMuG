/*
 *  ____
 * /\  _`\                                       /'\_/`\  __
 * \ \ \L\_\     __      ___ ___      __   _ __ /\      \/\_\    ___      __
 *  \ \ \L_L   /'__`\  /' __` __`\  /'__`\/\`'__\ \ \__\ \/\ \ /' _ `\  /'__`\
 *   \ \ \/, \/\ \L\.\_/\ \/\ \/\ \/\  __/\ \ \/ \ \ \_/\ \ \ \/\ \/\ \/\  __/
 *    \ \____/\ \__/.\_\ \_\ \_\ \_\ \____\\ \_\  \ \_\\ \_\ \_\ \_\ \_\ \____\
 *     \/___/  \/__/\/_/\/_/\/_/\/_/\/____/ \/_/   \/_/ \/_/\/_/\/_/\/_/\/____/
 *
 * Copyright (c) 2024-2024 GamerMine <maxime-sav@outlook.fr>
 *
 * This Source Code From is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/ .
 */

#include "SharedMemoryReader.h"

SharedMemoryReader::SharedMemoryReader(const char *segmentName, size_t size) {
    filePointer = shm_open(segmentName, O_RDONLY, DEFFILEMODE);
    success = true;
    if (filePointer == -1) {
        success = false;
    }

    if (success) {
        m_size = size;
        shm_ptr = mmap(nullptr, size, PROT_READ, MAP_SHARED, filePointer, 0);
        if (shm_ptr == MAP_FAILED) {
            success = false;
        }
    }
}

bool SharedMemoryReader::isSuccess() const {
    return success;
}

SharedMemoryReader::~SharedMemoryReader() {
    munmap(shm_ptr, m_size);
    close(filePointer);
}
