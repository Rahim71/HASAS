/*

    Gtk+ utility class
    Copyright (C) 1999-2002 Jussi Laako

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <X11/Xlib.h>
#include <gtk/gtk.h>
#include <gdk/gdkprivate.h>

#include "GtkUtils.hh"


void GListFreeItem (gpointer gpData, gpointer gpUserData)
{
    free(gpData);
}


inline void clGtkUtils::RemoveNewLine(char *cpLineStr)
{
    int iCharCntr;
    int iCntrMax = strlen(cpLineStr);

    for (iCharCntr = 0; iCharCntr < iCntrMax; iCharCntr++)
    {
        if (cpLineStr[iCharCntr] == '\n' || cpLineStr[iCharCntr] == '\r')
        {
            cpLineStr[iCharCntr] = 0x00;
            return;
        }
    }
}


void clGtkUtils::BuildOptionMenu(const GtkWidget *gwOptionMenu,
    GtkWidget **gwpMenu, GtkWidget *gwaMenuItem[], const char *cpaMenuTxts[],
    int iNumItems)
{
    int iItemCntr;
    GtkWidget *gwLocalMenu;
    GtkWidget *gwLocalMenuItem;

    gwLocalMenu = gtk_menu_new();
    *gwpMenu = gwLocalMenu;
    gtk_widget_show(gwLocalMenu);
    for (iItemCntr = 0; iItemCntr < iNumItems; iItemCntr++)
    {
        gwLocalMenuItem = gtk_menu_item_new_with_label(cpaMenuTxts[iItemCntr]);
        gwaMenuItem[iItemCntr] = gwLocalMenuItem;
        gtk_menu_append(GTK_MENU(gwLocalMenu), gwLocalMenuItem);
        gtk_widget_show(gwLocalMenuItem);
    }
    gtk_menu_set_active(GTK_MENU(gwLocalMenu), 0);
    gtk_option_menu_set_menu(GTK_OPTION_MENU(gwOptionMenu), gwLocalMenu);
}


int clGtkUtils::OptionMenuGetActive(const GtkWidget *gwOptionMenu,
    GtkWidget *gwaMenuItems[], int iMenuItemCount)
{
    int iItemCntr;
    GtkWidget *gwMenuWidget;
    GtkWidget *gwActiveItem;

    gwMenuWidget = gtk_option_menu_get_menu(GTK_OPTION_MENU(gwOptionMenu));
    gwActiveItem = gtk_menu_get_active(GTK_MENU(gwMenuWidget));
    for (iItemCntr = 0; iItemCntr < iMenuItemCount; iItemCntr++)
    {
        if (gwActiveItem == gwaMenuItems[iItemCntr])
            return iItemCntr;
    }
    return -1;
}


void clGtkUtils::ConnectMotionEvent(const GtkWidget *gwDest, 
    const GtkWidget *gwSrc)
{
    gtk_signal_connect_object(GTK_OBJECT(gwSrc),
        "motion_notify_event",
        (GtkSignalFunc) EVENT_METHOD(gwDest, motion_notify_event),
        GTK_OBJECT(gwDest));
}


void clGtkUtils::EnableBackingStore(const GtkWidget *gwWidget)
{
    #if (GTK_MAJOR_VERSION == 1)
    GdkWindowPrivate *gwpWindow;
    XSetWindowAttributes xswaAttrib;
    
    gwpWindow = (GdkWindowPrivate *) gwWidget->window;
    xswaAttrib.backing_store = Always;
    XChangeWindowAttributes(gwpWindow->xdisplay, gwpWindow->xwindow,
        CWBackingStore, &xswaAttrib);
    #endif
}


void clGtkUtils::ComboListFromFile(const GtkWidget *gwWidget,
    GList **pListPtr, const char *cpFileName)
{
    char cpLineBuf[GU_TXTBUFSIZE];
    char *cpNewItem;
    FILE *fileLoad;
    GList *glList = *pListPtr;

    if (glList != NULL) 
    {
        g_list_foreach(glList, GListFreeItem, NULL);
        g_list_free(glList);
        glList = NULL;
    }
    fileLoad = fopen(cpFileName, "rt");
    if (fileLoad != NULL)
    {
        while (fgets(cpLineBuf, GU_TXTBUFSIZE, fileLoad) != NULL)
        {
            RemoveNewLine(cpLineBuf);
            if (strlen(cpLineBuf) > 0)
            {
                cpNewItem = (char *) g_malloc(strlen(cpLineBuf) + 1);
                if (cpNewItem == NULL) return;
                strcpy(cpNewItem, cpLineBuf);
                glList = g_list_append(glList, cpNewItem);
            }
        }
        gtk_combo_set_popdown_strings(GTK_COMBO(gwWidget), glList);
        fclose(fileLoad);
    }
    *pListPtr = glList;
}

