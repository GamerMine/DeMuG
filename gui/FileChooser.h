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

#ifndef EMU_GAMEBOY_FILECHOOSER_H
#define EMU_GAMEBOY_FILECHOOSER_H

#include <gtk/gtk.h>
#include "../logging/Logger.h"

class FileChooser {
public:
#ifdef __linux__
    static void chooseROM() {
        auto dialog = gtk_file_dialog_new();
        gtk_file_dialog_set_title(dialog, "Choose a GameBoy ROM");

        GListStore *filters = g_list_store_new(GTK_TYPE_FILE_FILTER);

        GtkFileFilter *filter = gtk_file_filter_new();
        gtk_file_filter_add_pattern(filter, "*.gb");
        gtk_file_filter_set_name(filter, "GameBoy ROM");
        g_list_store_append(filters, filter);

        gtk_file_dialog_set_filters(dialog, G_LIST_MODEL(filters));
        gtk_file_dialog_open(dialog, nullptr, nullptr, [](GObject *src, GAsyncResult *res, gpointer data) {
            (void)data;
            GError *error = nullptr;
            GFile *file = gtk_file_dialog_open_finish(GTK_FILE_DIALOG(src), res, &error);
            Logger::getInstance("FileChooser")->log(Logger::DEBUG, "%s", g_file_get_path(file));
            }, nullptr);
    }
#endif
};

#endif //EMU_GAMEBOY_FILECHOOSER_H
