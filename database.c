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

#include <sqlite3.h>
#include <stdio.h>
#include <string.h>

sqlite3* database = NULL;

/*
 * This function will create/open the database given in the parameter db.
 * It will create all the neccessary tables if they don't yet exist. This
 * function must be called and it's success verified before using any other
 * functions of this library.
*/
int openDatabase(char* db)
{
  // Create/Open the database
  if (sqlite3_open_v2(db, &database, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL) != SQLITE_OK)
  {
    fprintf(stderr, "Failed to open database %s, reason: %s\n", db, sqlite3_errmsg(database));
    sqlite3_close(database);

    return -1;
  }

  // Create the neccessary tables if they don't exist
  sqlite3_stmt* command;
  if (sqlite3_prepare_v2(database, "CREATE TABLE IF NOT EXISTS books (name string not null, url string, date string not null);", -1, &command, NULL))
  {
    fprintf(stderr, "Failed to create books-table, reason: %s\n", sqlite3_errmsg(database));
    sqlite3_close(database);

    return -1;
  }

  if (sqlite3_step(command) != SQLITE_DONE)
  {
    fprintf(stderr, "Failed to create books-table, reason: %s\n", sqlite3_errmsg(database));
    sqlite3_finalize(command);
    sqlite3_close(database);

    return -1;
  }
  sqlite3_finalize(command);

  if (sqlite3_prepare_v2(database, "CREATE TABLE IF NOT EXISTS config (key string unique, value string);", -1, &command, NULL))
  {
    fprintf(stderr, "Failed to create config-table, reason: %s\n", sqlite3_errmsg(database));
    sqlite3_close(database);

    return -1;
  }

  if (sqlite3_step(command) != SQLITE_DONE)
  {
    fprintf(stderr, "Failed to create config-table, reason: %s\n", sqlite3_errmsg(database));
    sqlite3_finalize(command);
    sqlite3_close(database);

    return -1;
  }
  sqlite3_finalize(command);

  // Database successfully initialized

  return 0;
}

/*
 * This function starts a transaction on the database. Make sure that you finish
 * this transaction by calling endTransaction of abortTransaction. If the return
 * value is not 0, the transaction has to be assumed as not started.
*/
int beginTransaction()
{
  // Create and execute begin transaction command
  sqlite3_stmt* command;
  if (sqlite3_prepare_v2(database, "BEGIN EXCLUSIVE;", -1, &command, NULL))
  {
    fprintf(stderr, "Failed to start transaction, reason: %s\n", sqlite3_errmsg(database));

    return -1;
  }

  if (sqlite3_step(command) != SQLITE_DONE)
  {
    fprintf(stderr, "Failed to start transaction, reason: %s\n", sqlite3_errmsg(database));
    sqlite3_finalize(command);

    return -1;
  }
  sqlite3_finalize(command);
  
  return 0;
}

/*
 * This function will close the database, call at the end of the program before exiting.
*/
int closeDatabase()
{
  sqlite3_close(database); 
  return 0;
}

/*
 * This function will clear the contents of the table books. It should be called when
 * updating the list of books from the datasource. You should start a transaction
 * before calling this function to ensure you can rollback to the previous state in case
 * something goes wrong during data retrieval.
*/
int clearBooklist()
{
  // Create and execute delete command
  sqlite3_stmt* command;
  if (sqlite3_prepare_v2(database, "DELETE FROM books;", -1, &command, NULL))
  {
    fprintf(stderr, "Failed to clear booklist, reason: %s\n", sqlite3_errmsg(database));

    return -1;
  }

  if (sqlite3_step(command) != SQLITE_DONE)
  {
    fprintf(stderr, "Failed to clear booklist, reason: %s\n", sqlite3_errmsg(database));
    sqlite3_finalize(command);

    return -1;
  }
  sqlite3_finalize(command);
  
  return 0;
}

/*
 * This function will add a book to the books-table.
*/
int addBook(char* title, char* url, char* date)
{
  // Create, bind and execute the insert command
  sqlite3_stmt* command;
  if (sqlite3_prepare_v2(database, "INSERT INTO books (name, url, date) VALUES(?, ?, date(?));", -1, &command, NULL))
  {
    fprintf(stderr, "Failed to insert book, reason: %s\n", sqlite3_errmsg(database));

    return -1;
  }

  if (sqlite3_bind_text(command, 1, title, -1, SQLITE_STATIC))
  {
    fprintf(stderr, "Failed to insert book, reason: %s\n", sqlite3_errmsg(database));
    sqlite3_finalize(command);

    return -1;
  }

  if (sqlite3_bind_text(command, 2, url, -1, SQLITE_STATIC))
  {
    fprintf(stderr, "Failed to insert book, reason: %s\n", sqlite3_errmsg(database));
    sqlite3_finalize(command);

    return -1;
  }

  if (sqlite3_bind_text(command, 3, date, -1, SQLITE_STATIC))
  {
    fprintf(stderr, "Failed to insert book, reason: %s\n", sqlite3_errmsg(database));
    sqlite3_finalize(command);

    return -1;
  }

  if (sqlite3_step(command) != SQLITE_DONE)
  {
    fprintf(stderr, "Failed to insert book, reason: %s\n", sqlite3_errmsg(database));
    sqlite3_finalize(command);

    return -1;
  }
  sqlite3_finalize(command);
  
  return 0;
}

/*
 * This function ends a transaction on the database. The changes done in the transaction
 * will be commited.
*/
int endTransaction()
{
  // Create and execute end transaction command
  sqlite3_stmt* command;
  if (sqlite3_prepare_v2(database, "COMMIT;", -1, &command, NULL))
  {
    fprintf(stderr, "Failed to commit transaction, reason: %s\n", sqlite3_errmsg(database));

    return -1;
  }

  if (sqlite3_step(command) != SQLITE_DONE)
  {
    fprintf(stderr, "Failed to commit transaction, reason: %s\n", sqlite3_errmsg(database));
    sqlite3_finalize(command);

    return -1;
  }
  sqlite3_finalize(command);
  
  return 0;
}

/*
 * This function ends a transaction on the database. The changes done in the transaction
 * will be rolled back.
*/
int abortTransaction()
{
  // Create and execute rollback transaction command
  sqlite3_stmt* command;
  if (sqlite3_prepare_v2(database, "ROLLBACK;", -1, &command, NULL))
  {
    fprintf(stderr, "Failed to roll back transaction, reason: %s\n", sqlite3_errmsg(database));

    return -1;
  }

  if (sqlite3_step(command) != SQLITE_DONE)
  {
    fprintf(stderr, "Failed to roll back transaction, reason: %s\n", sqlite3_errmsg(database));
    sqlite3_finalize(command);

    return -1;
  }
  sqlite3_finalize(command);
  
  return 0;
}

/*
 * This function returns the amound of books where the time left is "Ok" (>5 days)
*/
int getOk()
{
  // Create and execute the select-command
  sqlite3_stmt* command;
  if (sqlite3_prepare_v2(database, "SELECT COUNT(*) FROM books WHERE date > date('now', '+5 day');", -1, &command, NULL))
  {
    fprintf(stderr, "Failed to get books, reason: %s\n", sqlite3_errmsg(database));

    return -1;
  }

  if (sqlite3_step(command) != SQLITE_ROW)
  {
    fprintf(stderr, "Failed to get books, reason: %s\n", sqlite3_errmsg(database));
    sqlite3_finalize(command);

    return -1;
  }

  // Obtain the data
  int books = sqlite3_column_int(command, 0);
  sqlite3_finalize(command);
  
  return books;
}

/*
 * This function returns the amound of books where the time left is "Soon" (not today but <=5days)
*/
int getSoon()
{
  // Create and execute the select-command
  sqlite3_stmt* command;
  if (sqlite3_prepare_v2(database, "SELECT COUNT(*) FROM books WHERE date <= date('now', '+5 day') AND date > date('now');", -1, &command, NULL))
  {
    fprintf(stderr, "Failed to get books, reason: %s\n", sqlite3_errmsg(database));

    return -1;
  }

  if (sqlite3_step(command) != SQLITE_ROW)
  {
    fprintf(stderr, "Failed to get books, reason: %s\n", sqlite3_errmsg(database));
    sqlite3_finalize(command);

    return -1;
  }

  // Obtain the data
  int books = sqlite3_column_int(command, 0);
  sqlite3_finalize(command);
  
  return books;
}

/*
 * This function returns the amound of books where the time left is "critical" (today)
*/
int getCritical()
{
  // Create and execute the select-command
  sqlite3_stmt* command;
  if (sqlite3_prepare_v2(database, "SELECT COUNT(*) FROM books WHERE date = date('now');", -1, &command, NULL))
  {
    fprintf(stderr, "Failed to get books, reason: %s\n", sqlite3_errmsg(database));

    return -1;
  }

  if (sqlite3_step(command) != SQLITE_ROW)
  {
    fprintf(stderr, "Failed to get books, reason: %s\n", sqlite3_errmsg(database));
    sqlite3_finalize(command);

    return -1;
  }

  // Obtain the data
  int books = sqlite3_column_int(command, 0);
  sqlite3_finalize(command);
  
  return books;
}

/*
 * This function returns the amound of books where the time left is "late" (lies in the past)
*/
int getLate()
{
  // Create and execute the select-command
  sqlite3_stmt* command;
  if (sqlite3_prepare_v2(database, "SELECT COUNT(*) FROM books WHERE date < date('now');", -1, &command, NULL))
  {
    fprintf(stderr, "Failed to get books, reason: %s\n", sqlite3_errmsg(database));

    return -1;
  }

  if (sqlite3_step(command) != SQLITE_ROW)
  {
    fprintf(stderr, "Failed to get books, reason: %s\n", sqlite3_errmsg(database));
    sqlite3_finalize(command);

    return -1;
  }

  // Obtain the data
  int books = sqlite3_column_int(command, 0);
  sqlite3_finalize(command);
  
  return books;
}

/*
 * This function checks if an update is needed (the last update was never or
 * is older than the specified amount of minutes)
*/
int needUpdate(int minutes)
{
  // Create and execute the select-command
  sqlite3_stmt* command;
  int ret;
  char sql[1024];
  snprintf(sql, 1024, "SELECT * FROM config WHERE key='lastupdate' AND value > datetime('now', '-%i minutes');", minutes);
  if (sqlite3_prepare_v2(database, sql, -1, &command, NULL))
  {
    fprintf(stderr, "Failed to get last update, reason: %s\n", sqlite3_errmsg(database));

    return -1;
  }

  if (sqlite3_step(command) == SQLITE_ROW) // If there is a result, we don't need to update
    ret = 0;
  else
    ret = 1;

  sqlite3_finalize(command);
  
  return ret;
}

/*
 * Call this function after an update was done, it will update the lastupdate-value
*/
int updateDone()
{
  // Create and execute the insert-command
  sqlite3_stmt* command;
  if (sqlite3_prepare_v2(database, "INSERT OR REPLACE INTO config (key, value) VALUES('lastupdate', datetime('now'));", -1, &command, NULL))
  {
    fprintf(stderr, "Failed to update lastupdate, reason: %s\n", sqlite3_errmsg(database));

    return -1;
  }

  if (sqlite3_step(command) != SQLITE_DONE)
  {
    fprintf(stderr, "Failed to update lastupdate, reason: %s\n", sqlite3_errmsg(database));
    sqlite3_finalize(command);

    return -1;
  }

  sqlite3_finalize(command);
  
  return 0;
}
