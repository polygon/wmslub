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
#include <sys/types.h>
#include <regex.h>
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

  // Build regexp to extract the date from the description
  regex_t dateExtract;
  if (regcomp(&dateExtract, "([0-9][0-9]?)\\s(Jan|Feb|Mär|Apr|Mai|Jun|Jul|Aug|Sep|Okt|Nov|Dez)\\s([0-9]{4})", REG_EXTENDED))
  {
    fprintf(stderr, "Failed to compile date-extracting regular expression\n");
    xmlXPathFreeObject(xpathObj);
    xmlXPathFreeContext(xpathCtxt);
    xmlFreeDoc(xmlRss);

    return -1;
  }

  // Begin transaction on database and clear book list
  if (beginTransaction())
  {
    xmlXPathFreeObject(xpathObj);
    xmlXPathFreeContext(xpathCtxt);
    xmlFreeDoc(xmlRss);

    return -1;
  }
  clearBooklist();

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

    // Fail if any string is missing
    if ((title == NULL) || (url == NULL) || (data == NULL))
    {
      fprintf(stderr, "Invalid entry, datafield(s) missing\n");
      xmlXPathFreeObject(xpathObj);
      xmlXPathFreeContext(xpathCtxt);
      xmlFreeDoc(xmlRss);
      regfree(&dateExtract);

      return -1;
    }

    // Match description against regexp
    regmatch_t matches[4];
    int ret;
    if (ret = regexec(&dateExtract, data, 4, matches, 0))
    {
      fprintf(stderr, "Invalid entry, date missing in description\n");
      xmlXPathFreeObject(xpathObj);
      xmlXPathFreeContext(xpathCtxt);
      xmlFreeDoc(xmlRss);
      regfree(&dateExtract);

      return -1;
    }

    // Extract date-data
    data[matches[1].rm_eo] = 0;
    data[matches[2].rm_eo] = 0;
    data[matches[3].rm_eo] = 0;

    // Month conversion
    char* month;
    if (!strcmp(&data[matches[2].rm_so], "Jan"))
      month = "01";
    else if (!strcmp(&data[matches[2].rm_so], "Feb"))
      month = "02";
    else if (!strcmp(&data[matches[2].rm_so], "Mär"))
      month = "03";
    else if (!strcmp(&data[matches[2].rm_so], "Apr"))
      month = "04";
    else if (!strcmp(&data[matches[2].rm_so], "Mai"))
      month = "05";
    else if (!strcmp(&data[matches[2].rm_so], "Jun"))
      month = "06";
    else if (!strcmp(&data[matches[2].rm_so], "Jul"))
      month = "07";
    else if (!strcmp(&data[matches[2].rm_so], "Aug"))
      month = "08";
    else if (!strcmp(&data[matches[2].rm_so], "Sep"))
      month = "09";
    else if (!strcmp(&data[matches[2].rm_so], "Okt"))
      month = "10";
    else if (!strcmp(&data[matches[2].rm_so], "Nov"))
      month = "11";
    else if (!strcmp(&data[matches[2].rm_so], "Dez"))
      month = "12";
    else
    {
      fprintf(stderr, "Invalid entry, unknown month: %s\n", data[matches[2].rm_so]);
      xmlXPathFreeObject(xpathObj);
      xmlXPathFreeContext(xpathCtxt);
      xmlFreeDoc(xmlRss);
      regfree(&dateExtract);
      abortTransaction();

      return -1;
    }

    // If day is in range 0-9, add a leading zero
    if (strlen(&data[matches[1].rm_so]) == 1)
    {
      matches[1].rm_so--;
      data[matches[1].rm_so] = '0';
    }

    // Build date string
    char date[255];
    date[0] = 0;
    snprintf(date, 255, "%s-%s-%s", &data[matches[3].rm_so], month, &data[matches[1].rm_so]);

    // Insert book to database
    if (addBook(title, url, date))
    {
      xmlXPathFreeObject(xpathObj);
      xmlXPathFreeContext(xpathCtxt);
      xmlFreeDoc(xmlRss);
      regfree(&dateExtract);
      abortTransaction();

      return -1;
    }
  }

  endTransaction();

  xmlXPathFreeObject(xpathObj);
  xmlXPathFreeContext(xpathCtxt);
  xmlFreeDoc(xmlRss);
  regfree(&dateExtract);
  return res;
}
