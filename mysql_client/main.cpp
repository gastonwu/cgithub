/*
 * connect1.c - connect to and disconnect from MySQL server
 */

#include <my_global.h>
#include <my_sys.h>
#include <mysql.h>

static char *opt_host_name = "127.0.0.1";    /* server host (default=localhost) */
static char *opt_user_name = "";    /* username (default=login name) */
static char *opt_password = "";     /* password (default=none) */
static unsigned int opt_port_num = 9306; /* port number (use built-in value) */
static char *opt_socket_name = NULL;  /* socket name (use built-in value) */
static char *opt_db_name = "test";      /* database name (default=none) */
static unsigned int opt_flags = 0;    /* connection flags (none) */

static MYSQL *conn;                   /* pointer to connection handler */

int
main (int argc, char *argv[])
{
  MY_INIT (argv[0]);
  /* initialize client library */
  if (mysql_library_init (0, NULL, NULL))
  {
    fprintf (stderr, "mysql_library_init() failed\n");
    exit (1);
  }
  /* initialize connection handler */
  conn = mysql_init (NULL);
  if (conn == NULL)
  {
    fprintf (stderr, "mysql_init() failed (probably out of memory)\n");
    exit (1);
  }
  /* connect to server */
  conn = mysql_real_connect (conn, opt_host_name, opt_user_name, opt_password,
      opt_db_name, opt_port_num, opt_socket_name, opt_flags);
  printf("conn:%d\n",(int)conn);

  if (conn == NULL)
  {
    fprintf (stderr, "mysql_real_connect() failed\n");
    mysql_close (conn);
    exit (1);
  }

MYSQL_RES *result;
MYSQL_ROW row;
unsigned int num_fields;
unsigned int i;
 
  mysql_query(conn,"select * from testrt where match('aa') limit 1");
  result = mysql_store_result(conn);


   unsigned long *lengths;
num_fields = mysql_num_fields(result);
while ((row = mysql_fetch_row(result)))
{
   lengths = mysql_fetch_lengths(result);
   for(i = 0; i < num_fields; i++)
   {
       printf("[%.*s] ", (int) lengths[i], row[i] ? row[i] : "NULL");
   }
   printf("\n");
}

  /* disconnect from server, terminate client library */
  mysql_close (conn);
  mysql_library_end ();
  exit (0);
}
