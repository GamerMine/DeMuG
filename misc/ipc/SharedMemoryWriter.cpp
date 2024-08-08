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

#include "SharedMemoryWriter.h"

SharedMemoryWriter::SharedMemoryWriter(char *segmentName, size_t size) {
    Logger *logger = Logger::getInstance("SharedMemoryWriter");
    filePointer = shm_open(segmentName, O_RDWR | O_CREAT);
    if (filePointer == -1) logger->log(Logger::CRITICAL, "Memory segment %s, could not be created", segmentName);

    if (ftruncate(filePointer, size) == -1) logger->log(Logger::CRITICAL, "Memory segment %s, cannot set size", segmentName);

    m_size = size;
    shm_ptr = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, filePointer, 0);
    if (shm_ptr == MAP_FAILED) logger->log(Logger::CRITICAL, "Memory segment %s, cannot map to memory", segmentName);
}

SharedMemoryWriter::~SharedMemoryWriter() {
    munmap(shm_ptr, m_size);
    close(filePointer);
}
