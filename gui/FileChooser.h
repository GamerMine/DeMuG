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

#ifndef EMU_GAMEBOY_FILEDIALOG_H
#define EMU_GAMEBOY_FILEDIALOG_H

#include <nfd.h>

class FileChooser {
public:
    static FileChooser *getInstance() {
        if (instance == nullptr) {
            instance = new FileChooser();
        }

        return instance;
    }

    const char *chooseROM() {
        nfdu8char_t *outputPath;
        nfdfilteritem_t fileFilters[1] = {{"GameBoy ROM", "gb"}};

        SetMasterVolume(0);
        nfdresult_t result = NFD_OpenDialogU8(&outputPath, fileFilters, 1, nullptr);

        switch (result) {
            case NFD_OKAY: return outputPath;
            default: return "";
        }
    }

private:
    FileChooser() {
        NFD_Init();
    }

    ~FileChooser() {
        NFD_Quit();
    }

    inline static FileChooser *instance;
};

#endif //EMU_GAMEBOY_FILEDIALOG_H
