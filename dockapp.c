/*
 * Copyright 2009 Jan Dohl
 *
 * This file is part of wmslub.
 *
 * wmslub is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * wmslub is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with wmslub.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "dockapp.h"
#include <gai/gai.h>
#include <string.h>

GaiCallback0* updatedata;

GdkPixbuf* p_ok;
GdkPixbuf* p_soon;
GdkPixbuf* p_crit;
GdkPixbuf* p_late;

int first = 1;

/*
 * This function does preinitialization
*/
void preInit(int* argc, char** argv[])
{
  // Init GAI
  GaiApplet gapp;
  gapp.name = "wmslub";
  gapp.version = "1.0";
  gapp.nice_name = "WM Slub";
  gapp.author = "Jan Dohl";
  gapp.license = "GNU GPLv3";
  gapp.description = "Auto-updating reminder for your books from the library";
  gapp.icon = "";
  gapp.image_path = ".";
  
  gai_init2(&gapp, argc, argv);
}

/*
 * This function calls the update-callback to get the data and then draws the dockapp
*/
gboolean redraw(gpointer data)
{
  // Draw the background only once
  if (first)
  {
    GdkPixbuf * bg = gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, 8, 64, 64);
    gdk_pixbuf_fill(bg, 0x202020FF);
    gai_draw_bg(bg, 0, 0, 56, 56, 4, 4);
    gdk_pixbuf_unref(bg);
    GdkPixbuf * div = gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, 8, 1, 56);
    gdk_pixbuf_fill(div, 0x000000FF);
    gai_draw_bg(div, 0, 0, 1, 56, 28, 4);
    gdk_pixbuf_unref(div);
    gai_draw_update_bg();
    first = 0;
  }

  // TODO: Find a way to get the text on the background w/o having ugly transparency issues
  //       That way we could drastically reduce update intervals
  gai_draw(p_ok, 0, 0, gdk_pixbuf_get_width(p_ok), gdk_pixbuf_get_height(p_ok), 6, 6);
  gai_draw(p_soon, 0, 0, gdk_pixbuf_get_width(p_soon), gdk_pixbuf_get_height(p_soon), 6, 18);
  gai_draw(p_crit, 0, 0, gdk_pixbuf_get_width(p_crit), gdk_pixbuf_get_height(p_crit), 6, 30);
  gai_draw(p_late, 0, 0, gdk_pixbuf_get_width(p_late), gdk_pixbuf_get_height(p_late), 6, 42);

  // Draw the numbers
  bookdata bd;
  updatedata((gpointer)&bd);

  char buf[128];
  GdkPixbuf* pb;

  memset(buf, 0, 128);
  snprintf(buf, 128, "%i", bd.ok);
  pb = gai_text_create(buf, "Courier New", 8, GAI_TEXT_NORMAL, 64, 255, 64);
  gai_draw(pb, 0, 0, gdk_pixbuf_get_width(pb), gdk_pixbuf_get_height(pb), 32, 6);
  gdk_pixbuf_unref(pb);
  
  memset(buf, 0, 128);
  snprintf(buf, 128, "%i", bd.soon);
  pb = gai_text_create(buf, "Courier New", 8, GAI_TEXT_NORMAL, 255, 255, 64);
  gai_draw(pb, 0, 0, gdk_pixbuf_get_width(pb), gdk_pixbuf_get_height(pb), 32, 18);
  gdk_pixbuf_unref(pb);

  memset(buf, 0, 128);
  snprintf(buf, 128, "%i", bd.crit);
  pb = gai_text_create(buf, "Courier New", 8, GAI_TEXT_NORMAL, 255, 64, 64);
  gai_draw(pb, 0, 0, gdk_pixbuf_get_width(pb), gdk_pixbuf_get_height(pb), 32, 30);
  gdk_pixbuf_unref(pb);

  memset(buf, 0, 128);
  snprintf(buf, 128, "%i", bd.late);
  pb = gai_text_create(buf, "Courier New", 8, GAI_TEXT_NORMAL, 128, 128, 128);
  gai_draw(pb, 0, 0, gdk_pixbuf_get_width(pb), gdk_pixbuf_get_height(pb), 32, 42);
  gdk_pixbuf_unref(pb);

  gai_draw_update();
}

/*
 * This function prepares the window, draws the background and sets all the other stuff up
*/
int initDockapp(GaiCallback0 update)
{
  gai_background_set(64, 64, 64, TRUE);
  gai_signal_on_update(redraw, 100, 0);
  updatedata = update;

  p_ok = gai_text_create("Ok:", "Courier New", 8, GAI_TEXT_NORMAL, 64, 255, 64);
  p_soon = gai_text_create("<5:", "Courier New", 8, GAI_TEXT_NORMAL, 255, 255, 64);
  p_crit = gai_text_create("<1:", "Courier New", 8, GAI_TEXT_NORMAL, 255, 64, 64);
  p_late = gai_text_create("Lt:", "Courier New", 8, GAI_TEXT_NORMAL, 160, 160, 160);
}

/*
 * This function finally starts the dockapp
*/
void launchDockapp()
{
  gai_start();
}
