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

#include "booklist.h"
#include <dockapp.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>

char* url = NULL;
char* db = NULL;

void destroy()
{
  closeDatabase();
}

void update()
{

}

int main(int argc, char* argv[])
{
  // Parameters
  DAProgramOption options[2];
  memset(options, 0, 2*sizeof(DAProgramOption));
  options[0].shortForm = "-u";
  options[0].longForm = "--url";
  options[0].description = "The URL of your personal SLUB-RSS-Feed";
  options[0].type = DOString;
  options[0].value.string = &url;
 
  options[1].shortForm = "-d";
  options[1].longForm = "--database";
  options[1].description = "The sqlite-database to use, will be created if not existing (optional)";
  options[1].type = DOString;
  options[1].value.string = &db;

  // Callbacks
  DACallbacks callbacks;
  callbacks.destroy = destroy;
  callbacks.buttonPress = NULL;
  callbacks.buttonRelease = NULL;
  callbacks.motion = NULL;
  callbacks.enter = NULL;
  callbacks.leave = NULL;
  callbacks.timeout = update;

  // Obtain options
  DAParseArguments(argc, argv, options, 2, "WMSlub - A reminder for everyone who likes to forget books from the library", "WMSlub 1.0");

  // If the URL is not given, we abort
  if (!options[0].used)
  {
    fprintf(stderr, "The URL of the RSS-Feed is missing, please use -u\n");
    return 0;
  }

  // If the database is given, use it, otherwise use standard db location
  char dbfile[1024];
  memset(dbfile, 0, 1024);

  if (!options[1].used)
  {
    // The database-file defaults to "$HOME/.wmslub.db"
    strcat(dbfile, getenv("HOME"));
    strcat(dbfile, "/.wmslub.db");
    db = dbfile;
  } 

  // Try to open the database
  if (openDatabase(db))
    return 1;

  // Initialize dockapp
  DASetExpectedVersion(20030126);

  DAOpenDisplay(NULL, argc, argv);

  DACreateIcon("WMSlub", 48, 48, argc, argv);

  DASetCallbacks(&callbacks);

  DASetTimeout(1000);

  DAShow();

  DAEventLoop();

  return 0;
}
