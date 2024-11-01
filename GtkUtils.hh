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


#include <gtk/gtk.h>


#ifndef GTKUTILS_HH
    #define GTKUTILS_HH


    #define GU_TXTBUFSIZE           1024
    #if (GTK_MAJOR_VERSION == 1)
    #define EVENT_METHOD(i,x) GTK_WIDGET_CLASS(GTK_OBJECT(i)->klass)->x
    #else
    #define EVENT_METHOD(i,x) GTK_WIDGET_GET_CLASS(GTK_OBJECT(i))->x
    #endif


    /**
        Gtk+ utility class
    */
    class clGtkUtils
    {
            void RemoveNewLine (char *);
        public:
            clGtkUtils () {}
            ~clGtkUtils () {}
            /**
                Builds and attaches entries for specified OptionMenu,
                sets default entry to 0.

                \param gwOptionMenu Option menu widget (in)
                \param gwpMenu Menu (out)
                \param gwaMenuItem Menu items (out)
                \param cpaMenuTxts Menu item texts (in)
                \param iNumItems Itemcount (in)
            */
            void BuildOptionMenu (const GtkWidget *, GtkWidget **, 
                GtkWidget **, const char **, int);
            /**
                Get active item for option menu.

                \param gwOptionMenu Option menu
                \param gwaMenuItems Menu items
                \param iMenuItemCount Number of menuitems
                \return Index of active menuitem, -1 on error
            */
            int OptionMenuGetActive (const GtkWidget *, GtkWidget **, int);
            /**
                Connects widgets motion_notify_event to specified widget.

                This is very useful for connecting drawing area's 
                motion_notify_event to rulers.

                \param gwDest Destination widget
                \param gwSrc Source widget
            */
            void ConnectMotionEvent (const GtkWidget *, const GtkWidget *);
            /**
                Enables X backing store for specified widget.

                \param gwWidget Widget
            */
            void EnableBackingStore (const GtkWidget *);
            /**
                Creates Combo widget's popdown menu from specified file.

                \note Free's list if already allocated

                \param gwWidget Widget
                \param pListPtr GList object pointer (out)
                \param cpFileName Name of file
            */
            void ComboListFromFile (const GtkWidget *, GList **, const char *);
    };

#endif
