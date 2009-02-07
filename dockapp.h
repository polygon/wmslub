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

#ifndef _DOCKAPP_H
#define _DOCKAPP_H

#include <gai/gai.h>

typedef struct 
{
  int ok;
  int soon;
  int crit;
  int late;
} bookdata;

void preInit(int* argc, char** argv[]);
int initDockapp(GaiCallback0 func);
void launchDockapp();

#endif // _DOCKAPP_H
