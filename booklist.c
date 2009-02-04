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
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

char curlErrorBuffer[CURL_ERROR_SIZE];

int readEntries(xmlDocPtr xmlRss);

/*
 * This function is the callbackfunction for rss-receive
*/
size_t receive(char* data, size_t size, size_t nmemb, xmlParserCtxtPtr* parser)
{
  // Does the parser already exist?
  if (*parser == NULL)
  {
    // Create the parser and feed the first chunk of data
    *parser = xmlCreatePushParserCtxt(NULL, NULL, data, size*nmemb, NULL);
    if (*parser == NULL)
    {
      fprintf(stderr, "Could not create xmlPushParserCtxt.\n");
      return 0; // Failed to create the parser, no data was processed
    }
  }
  else
  {
    // Feed the next chunk of data into the parser
    if (xmlParseChunk(*parser, data, size*nmemb, 0))
    {
      fprintf(stderr, "XML parsing failed.\n");
      return 0; // Parsing failure, no data was processed
    }
  }

  return size * nmemb;  // Return amount of processed data as required by cURL
}

/*
 * This function will retrieve the RSS-Feed from the URL and update
 * list of books in the database
*/
int updateList(char* url)
{
  CURL* curl;
  CURLcode curlResult;
  xmlParserCtxtPtr xmlParser = NULL;
  xmlDocPtr xmlRss;

  // Initialize curl and prepare operation
  curl = curl_easy_init();

  if (!curl)
    return -1;

  curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, curlErrorBuffer);
  curl_easy_setopt(curl, CURLOPT_URL, url);
  curl_easy_setopt(curl, CURLOPT_HEADER, 0);
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, receive);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &xmlParser);

  curlResult = curl_easy_perform(curl);
  //curl_easy_cleanup(curl);

  if (curlResult != CURLE_OK)
  {
    fprintf(stderr, "Error reading RSS-feed: %s\n", curlErrorBuffer);
    if (xmlParser)
      xmlFreeParserCtxt(xmlParser);

    return -1;
  }

  // Finish parsing, check validity and clean up
  xmlParseChunk(xmlParser, NULL, 0, 1);
  if (!xmlParser->wellFormed)
  {
    fprintf(stderr, "Error reading RSS-feed: XML is not well-formed\n");
    xmlFreeParserCtxt(xmlParser);

    return -1;
  }

  xmlRss = xmlParser->myDoc;
  xmlFreeParserCtxt(xmlParser);

  return readEntries(xmlRss);
}

/*
 * This function takes the RSS-Feed and updates the book database
*/
int readEntries(xmlDocPtr xmlRss)
{
  // Build XPath to select all booktitles
  xmlXPathContextPtr xpathCtxt = NULL;
  xmlXPathObjectPtr xpathObj = NULL;
  int res = 0;
  int i;

  // Create XPath-query for all items of the channel and execute it
  xpathCtxt = xmlXPathNewContext(xmlRss);
  if (xpathCtxt == NULL)
  {
    fprintf(stderr, "Could not create XPath-Context\n");
    xmlFreeDoc(xmlRss);

    return -1;
  }

  xpathObj = xmlXPathEvalExpression("/rss/channel/item", xpathCtxt);
  if (xpathObj == NULL)
  {
    fprintf(stderr, "Could not execute XPath-Expression \"/rss/channel/item\"\n");
    xmlXPathFreeContext(xpathCtxt);
    xmlFreeDoc(xmlRss);

    return -1;
  }

  // Extract data from every entry
  for (i = 0; i < xpathObj->nodesetval->nodeNr; i++)
  {
    char* title = NULL;
    char* url = NULL;
    char* data = NULL;
    xmlNodePtr book = xpathObj->nodesetval->nodeTab[i];

    // Check all child nodes for their content
    xmlNodePtr entry;
    for (entry = book->children; entry != NULL; entry = entry->next)
    {
      if (xmlStrEqual(entry->name, "title"))
        title = (char*)xmlNodeGetContent(entry);
      if (xmlStrEqual(entry->name, "link"))
        url = (char*)xmlNodeGetContent(entry);
      if (xmlStrEqual(entry->name, "description"))
        data = (char*)xmlNodeGetContent(entry);
    }

    /// Testcode
    if (title)  printf("Titel: %s\n", title);
    if (url)    printf("URL: %s\n", url);
    if (data)   printf("Data: %s\n", data);
    printf("\n");
  }

  xmlXPathFreeObject(xpathObj);
  xmlXPathFreeContext(xpathCtxt);
  xmlFreeDoc(xmlRss);
  return res;
}
