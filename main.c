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
#include "database.h"
#include "dockapp.h"
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

char url[1024];
char db[1024];

gboolean update(gpointer userdata)
{
  // Update booklist if that is needed
  if (needUpdate(10) == 1)
  {
    updateList(url);
    updateDone();
  }

  bookdata* bd = (bookdata*)userdata;
  bd->ok = getOk();
  bd->soon = getSoon();
  bd->crit = getCritical();
  bd->late = getLate();
  return TRUE;
}

void printUsage()
{
  printf("Usage: wmslub -u <RSS-URL> [-d <DB-File>]\n");
}

int main(int argc, char* argv[])
{
  // Preinitialization of dockapp
  preInit(&argc, &argv);

  // Parse commandline options
  int hasURL = 0;
  int hasDB = 0;
  memset(url, 0, 1024);
  memset(db, 0, 1024);

  int opt;
  while ((opt = getopt(argc, argv, "u:d:")) != -1)
  {
    switch (opt)
    {
    case 'u':
      strncpy(url, optarg, 1023);
      hasURL = 1;
      break;
    case 'd':
      strncpy(db, optarg, 1023);
      hasDB = 1;
      break;
    default:
      printUsage();
      exit(1);
    }
  }

  if (!hasURL)
  {
    printUsage();
    exit(1);
  }

  if (!hasDB)
  {
    strcat(db, getenv("HOME"));
    strcat(db, "/.wmslub.db");
  }

  // Try to open the database
  if (openDatabase(db))
    return 1;

  // Init dockapp
  initDockapp(update);

  launchDockapp();

  return 0;
}
