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
#include <curl/curl.h>
#include <sqlite3.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char curlErrorBuffer[CURL_ERROR_SIZE];

/*
 * This function is the callbackfunction for rss-receive
*/
size_t receive(char* data, size_t size, size_t nmemb, void* stream)
{
  // Testcode
  char* buffer = (char*)malloc(size * nmemb + 1);
  memset(buffer, 0, size * nmemb + 1);
  memcpy(buffer, data, size * nmemb);
  printf(buffer);
  free(buffer);
  return size*nmemb;
}

/*
 * This function will retrieve the RSS-Feed from the URL and update
 * list of books in the database
*/
int updateList(char* url)
{
  CURL* curl;
  CURLcode curlResult;

  // Initialize curl and prepare operation
  curl = curl_easy_init();

  if (!curl)
    return -1;

  curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, curlErrorBuffer);
  curl_easy_setopt(curl, CURLOPT_URL, url);
  curl_easy_setopt(curl, CURLOPT_HEADER, 0);
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, receive);

  curlResult = curl_easy_perform(curl);

  if (curlResult != CURLE_OK)
    return -1;
}
