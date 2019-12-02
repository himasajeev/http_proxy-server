
#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<string.h>
#include<stdlib.h>
#include<netdb.h>
#include<stdbool.h>
#include<sys/select.h>
#include<limits.h>



#define no_of_connections 1
int frequency[15];
int recentlyused[5] = { -1, -1, -1, -1, -1 };

int precnt = -1;
char website[15][50] = { '\0' };
char blockedSite[4][50] =
  { "http://iczn.org", "http://wallacefund.info",
"http://checklist.onlineflora.cn", "http://mosquito-taxonomic-inventory.info" };
char authSite[4][50] =
  { "http://katydidsfrombrazil.myspecies.info","http://www.google.com",
  //"http://s3p.myspecies.info",
"http://ul.myspecies.info", "http://tree.myspecies.info" };
char pass[10] = "password";

int findminfreq ()
{
  //printf ("\n finding minimum frequency");
  int min = INT_MAX;
  int location = -1;
  for (int c = 0; c < 15; c++)
    {
      int flag = 0;
      if (frequency[c] < min)
	{
	  for (int ii = 0; ii < 5; ii++)
	    {
	      if (recentlyused[ii] == c)
		{
		  flag++;
		  break;
		}
	    }
	  if (!flag)
	    {
	      min = frequency[c];
	      location = c;
	    //  printf ("\n min = %d", min);
	     // printf ("\n loc = %d", location);
	    }
	}
    }
  //printf ("\n %d", location);
  return location;
}

int
main (int arg_cnt, char *arg_val[])
{
  int proxy_sock, client_sock, clilen, n, web_sock;
  char buffer[6535], temp_buffer[10];
  struct sockaddr_in proxy_addr, accept_addr, web_addr;
  struct hostent *webserver;

  if (arg_cnt < 2)
    {
      fprintf (stderr, "\nError, No port provided ");
      exit (1);
    }

  proxy_sock = socket (AF_INET, SOCK_STREAM, 0);

  if (proxy_sock < 0)
    {
      perror ("\nError in opening proxy socket ");
      close (proxy_sock);
      exit (1);
    }

  bzero ((char *) &proxy_addr, sizeof (proxy_addr));
  bzero ((char *) &accept_addr, sizeof (accept_addr));

  proxy_addr.sin_family = AF_INET;
  proxy_addr.sin_addr.s_addr = INADDR_ANY;
  proxy_addr.sin_port = htons (atoi (arg_val[1]));

  if (bind (proxy_sock, (struct sockaddr *) &proxy_addr, sizeof (proxy_addr))
      < 0)
    {
      perror ("\nError in binding proxy socket and proxy addresses ");
      close (proxy_sock);
      exit (1);
    }

  listen (proxy_sock, no_of_connections);
  clilen = sizeof (accept_addr);

  do
    {
      char host[1024], request[512], site[2048], *trim_host =
	NULL, *find_conn = NULL, sitecpy[2048], *trim_port = NULL;
      bool portprovided = false, proceed = false;
      char URL[50];
      int port = 80, i;
      client_sock =
	accept (proxy_sock, (struct sockaddr *) &accept_addr, &clilen);

      if (client_sock < 0)
	{
	  perror ("\nError in accepting socket on proxy ");
	  close (proxy_sock);
	  close (client_sock);
	  exit (1);
	}
      bzero (buffer, sizeof (buffer));

      do
	{
	  bzero (temp_buffer, sizeof (temp_buffer));
	  n = read (client_sock, temp_buffer, sizeof (temp_buffer) - 1);
	  if (n != 0)
	    strcat (buffer, temp_buffer);
	}
      while (buffer[strlen (buffer) - 1] != '\n'
	     || buffer[strlen (buffer) - 2] != '\r'
	     || buffer[strlen (buffer) - 3] != '\n'
	     || buffer[strlen (buffer) - 4] != '\r');
      sscanf (buffer, "%s %s ", request, site);
      int Bflag = 0;
      int authflag = 0;
     // printf ("\n \n site %s blag %d authflag %d", site, Bflag, authflag);

      for (int k = 0; k < 4; k++)
	{
	  if (strcmp (site, blockedSite[k]) == 0)
	    {
	    //  printf ("\n inside blockedif");
	      Bflag++;
	      break;
	    }
	  if (strcmp (site, authSite[k]) == 0)
	    {
	     // printf ("\n inside autgedif");
	      authflag++;
	      break;
	    }
	}
      char password[10];
      int AAflag = 1;
      //printf ("\n authflag %d\n", authflag);
      if (authflag)
	{
	  char messag[] =
	    "Requesting access to restricted website!! Enter password";
	  write (client_sock, messag, strlen (messag));
	  password[0] = '\0';
	  int n1 = read (client_sock, password, sizeof (password));
	  //scanf ("%s", password);
	  password[8] = '\0';
	 // printf ("password %s ", password);
	 // printf ("after password");
	 // printf ("\n pass%s\n", pass);
	  if (strcmp (pass, password) != 0)
	    {
	     // printf ("inside strcmp");
	      AAflag = 0;
	    }
	}
      if ((strncmp (request, "GET", 3) != 0))
	{
	  printf ("\nNot a GET Request!");
	  proceed = false;
	}
      else if ((strncmp (site, "http://", 7) != 0))
	{
	  printf ("\nNot a HTTP Request!");
	  proceed = false;
	}
      else
	proceed = true;
      //printf (" bflag : %d", Bflag);
      if (proceed == true)
	{
	  //printf ("inside proceed\n");
	  if (Bflag == 1)
	    {
	      char messag[] = "Access denied!! Requested website is blocked";
	      write (client_sock, messag, strlen (messag));
	      printf ("\n Access denied!! Requested website is blocked");
	      close (client_sock);
	    }
	  else
	    {
	     // printf ("inside else\n");

	      //printf ("aaflag %d\n", AAflag);
	      if (!AAflag)
		{
		  //printf ("inside else if\n");

		  char messag[] = "Authentication failed!! Invalid password";
		  write (client_sock, messag, strlen (messag));
		  printf ("\n Authentication failed!! Invalid password");

		  close (client_sock);

		}
	      else
		{
		  //printf ("inside elseelse\n");
		  strcpy (URL, site);
		  int flag = 0;
		  for (int i = 0; i < 15; i++)
		    {
		      if (strlen (website[i]))
			{
			 // printf ("\n strlen %d", strlen (website[i]));
			  if (!strcmp (URL, website[i]))
			    {
			      flag++;
			      break;
			    }
			}
		    }
		  if (flag)
		    {
		      FILE *obj;

		      switch (i)
			{
			case 0:
			  obj = fopen ("file0.txt", "r");
			  fscanf (obj, "%s", buffer);
			  fclose (obj);
			  frequency[i]++;
			  precnt++;
			  recentlyused[precnt % 5] = i;
			  break;
			case 1:
			  obj = fopen ("file1.txt", "r");
			  fscanf (obj, "%s", buffer);
			  fclose (obj);
			  frequency[i]++;
			  precnt++;
			  recentlyused[precnt % 5] = i;
			  break;
			case 2:
			  obj = fopen ("file2.txt", "r");
			  fscanf (obj, "%s", buffer);
			  fclose (obj);
			  frequency[i]++;
			  precnt++;
			  recentlyused[precnt % 5] = i;
			  break;
			case 3:
			  obj = fopen ("file3.txt", "r");
			  fscanf (obj, "%s", buffer);
			  fclose (obj);
			  frequency[i]++;
			  precnt++;
			  recentlyused[precnt % 5] = i;
			  break;
			case 4:
			  obj = fopen ("file4.txt", "r");
			  fscanf (obj, "%s", buffer);
			  fclose (obj);
			  frequency[i]++;
			  precnt++;
			  recentlyused[precnt % 5] = i;
			  break;
			case 5:
			  obj = fopen ("file5.txt", "r");
			  fscanf (obj, "%s", buffer);
			  fclose (obj);
			  frequency[i]++;
			  precnt++;
			  recentlyused[precnt % 5] = i;
			  break;
			case 6:
			  obj = fopen ("file6.txt", "r");
			  fscanf (obj, "%s", buffer);
			  fclose (obj);
			  frequency[i]++;
			  precnt++;
			  recentlyused[precnt % 5] = i;
			  break;
			case 7:
			  obj = fopen ("file7.txt", "r");
			  fscanf (obj, "%s", buffer);
			  fclose (obj);
			  frequency[i]++;
			  precnt++;
			  recentlyused[precnt % 5] = i;
			  break;
			case 8:
			  obj = fopen ("file8.txt", "r");
			  fscanf (obj, "%s", buffer);
			  fclose (obj);
			  frequency[i]++;
			  precnt++;
			  recentlyused[precnt % 5] = i;
			  break;
			case 9:
			  obj = fopen ("file9.txt", "r");
			  fscanf (obj, "%s", buffer);
			  fclose (obj);
			  frequency[i]++;
			  precnt++;
			  recentlyused[precnt % 5] = i;
			  break;
			case 10:
			  obj = fopen ("file10.txt", "r");
			  fscanf (obj, "%s", buffer);
			  fclose (obj);
			  frequency[i]++;
			  precnt++;
			  recentlyused[precnt % 5] = i;
			  break;
			case 11:
			  obj = fopen ("file11.txt", "r");
			  fscanf (obj, "%s", buffer);
			  fclose (obj);
			  frequency[i]++;
			  precnt++;
			  recentlyused[precnt % 5] = i;
			  break;
			case 12:
			  obj = fopen ("file12.txt", "r");
			  fscanf (obj, "%s", buffer);
			  fclose (obj);
			  frequency[i]++;
			  precnt++;
			  recentlyused[precnt % 5] = i;
			  break;
			case 13:
			  obj = fopen ("file13.txt", "r");
			  fscanf (obj, "%s", buffer);
			  fclose (obj);
			  frequency[i]++;
			  precnt++;
			  recentlyused[precnt % 5] = i;
			  break;
			case 14:
			  obj = fopen ("file14.txt", "r");
			  fscanf (obj, "%s", buffer);
			  fclose (obj);
			  frequency[i]++;
			  precnt++;
			  recentlyused[precnt % 5] = i;
			  break;
			}


		    }


		  else
		    {

		      for (i = 7; i < strlen (site); i++)
			{
			  if (site[i] == ':')
			    {
			      memcpy (sitecpy, &site[i], strlen (site) - i);
			      sitecpy[strlen (site) - i] = '\0';
			      trim_port = strtok (sitecpy, ":/");

			      for (i = 0; i < strlen (trim_port); i++)
				{
				  if (trim_port[i] < '0'
				      || trim_port[i] > '9')
				    {
				      portprovided = false;
				      break;
				    }
				  else
				    portprovided = true;
				}
			      break;
			    }
			}
		      if (portprovided == true)
			port = atoi (trim_port);
		      find_conn = strstr (buffer, "keep-alive");
		      if (find_conn != NULL)
			strncpy (find_conn, "close\r\n\r\n\r", 10);
		      trim_host = strtok (site, "//");
		      trim_host = strtok (NULL, "/:");

		      sprintf (host, "%s", trim_host);

		      web_sock = socket (AF_INET, SOCK_STREAM, 0);

		      if (web_sock < 0)
			{
			  perror ("\nError in opening web socket ");
			  close (proxy_sock);
			  close (client_sock);
			  close (web_sock);
			  exit (1);
			}
		      webserver = gethostbyname (host);

		      if (webserver == NULL)
			{
			  printf ("\nBad host or error in host");
			  char bad_req[] =
			    "400 : BAD REQUEST\n\nNo Such host!";
			  write (client_sock, bad_req, strlen (bad_req));
			  close (client_sock);
			  continue;
			}

		      bzero ((char *) &web_addr, sizeof (web_addr));
		      web_addr.sin_port = htons (port);
		      web_addr.sin_family = AF_INET;
		      bcopy ((char *) webserver->h_addr,
			     (char *) &web_addr.sin_addr.s_addr,
			     webserver->h_length);

		      if (connect
			  (web_sock, (struct sockaddr *) &web_addr,
			   sizeof (web_addr)) < 0)
			{
			  perror ("\nError connecting to webserver ");
			  char bad_req[] =
			    "400 : BAD REQUEST\n\nConnection timed out";
			  write (client_sock, bad_req, strlen (bad_req));
			  close (client_sock);
			  continue;
			}
		      char bff[1000];

		      write (web_sock, buffer, strlen (buffer));
		      //printf ("b4 do while\n");
		      int loc = findminfreq ();
		      bzero (buffer, sizeof (buffer));

		      do
			{ bzero (buffer, sizeof (buffer));

			  n = read (web_sock, buffer, sizeof (buffer) - 1);
			//  printf ("\n\nafter readong\n");
               // printf("\n n %d",n);
			  if (n > 0)
			    write (client_sock, buffer, strlen (buffer));
			//  printf("hopefully in next line");

                 //printf("\n\n\n %s",buffer);
			 // printf ("\n \n minimum frequency %d\n\n", loc);
			  FILE *obj;
			  switch (loc)
			    {
			     

			    case 0:
			      obj = fopen ("file0.txt", "a");
			      int n;
			     // printf ("after fopen\n");
			     // printf ("\n\n%s\n\n", buffer);
			      strcpy (website[loc], "");
			      strcpy (website[loc], URL);
			      fprintf (obj, "%s", buffer);
			     // printf ("\n\n%s\n\n", buffer);
			      fclose (obj);

			      break;
			    case 1:
			      obj = fopen ("file1.txt", "a");
			      strcpy (website[loc], "");
			      strcpy (website[loc], URL);
			      fprintf (obj, "%s", buffer);
			      fclose (obj);
			      break;
			    case 2:
			      obj = fopen ("file2.txt", "a");
			      strcpy (website[loc], "");
			      strcpy (website[loc], URL);
			      fprintf (obj, "%s", buffer);
			      fclose (obj);
			      break;
			    case 3:
			      obj = fopen ("file3.txt", "a");
			      strcpy (website[loc], "");
			      strcpy (website[loc], URL);
			      fprintf (obj, "%s", buffer);
			      fclose (obj);
			      break;
			    case 4:
			      obj = fopen ("file4.txt", "w+");
			      strcpy (website[loc], "");
			      strcpy (website[loc], URL);
			      fprintf (obj, "%s", buffer);
			      fclose (obj);
			      break;
			    case 5:
			      obj = fopen ("file5.txt", "a");
			      strcpy (website[loc], "");
			      strcpy (website[loc], URL);
			      fprintf (obj, "%s", buffer);
			      fclose (obj);
			      break;
			    case 6:
			      obj = fopen ("file6.txt", "a");
			      strcpy (website[loc], "");
			      strcpy (website[loc], URL);
			      fprintf (obj, "%s", buffer);
			      fclose (obj);
			      break;
			    case 7:
			      obj = fopen ("file7.txt", "a");
			      strcpy (website[loc], "");
			      strcpy (website[loc], URL);
			      fprintf (obj, "%s", buffer);
			      fclose (obj);
			      break;
			    case 8:
			      obj = fopen ("file8.txt", "a");
			      strcpy (website[loc], "");
			      strcpy (website[loc], URL);
			      fprintf (obj, "%s", buffer);
			      fclose (obj);
			      break;
			    case 9:
			      obj = fopen ("file9.txt", "a");
			      strcpy (website[loc], "");
			      strcpy (website[loc], URL);
			      fprintf (obj, "%s", buffer);
			      fclose (obj);
			      break;
			    case 10:
			      obj = fopen ("file10.txt", "a");
			      strcpy (website[loc], "");
			      strcpy (website[loc], URL);
			      fprintf (obj, "%s", buffer);
			      fclose (obj);
			      break;
			    case 11:
			      obj = fopen ("file11.txt", "a");
			      strcpy (website[loc], "");
			      strcpy (website[loc], URL);
			      fprintf (obj, "%s", buffer);
			      fclose (obj);
			      break;
			    case 12:
			      obj = fopen ("file12.txt", "a");
			      strcpy (website[loc], "");
			      strcpy (website[loc], URL);
			      fprintf (obj, "%s", buffer);
			      fclose (obj);
			      break;
			    case 13:
			      obj = fopen ("file13.txt", "a");

			      strcpy (website[loc], "");
			      strcpy (website[loc], URL);
			      fprintf (obj, "%s", buffer);
			      fclose (obj);
			      break;
			    case 14:
			      obj = fopen ("file14.txt", "a");

			      strcpy (website[loc], "");
			      strcpy (website[loc], URL);
			      fprintf (obj, "%s", buffer);
			      fclose (obj);
			      break;


			    }
			}
		      while (n > 0);
		      frequency[loc] = 1;
		      precnt++;
		      recentlyused[precnt % 5] = loc;
		     // printf ("\n\nwritten to socket\n\n");
		      //printf ("\n\n done writing\n\n");
		      close (client_sock);

		    }

		}

	    }

	}
      else
	{
	  char bad_req[] =
	    "400 : BAD REQUEST\n\nOnly HTTP Protocols and GET requests accepted";
	  write (client_sock, bad_req, strlen (bad_req));
	  close (client_sock);
	}

    }
  while (1);

  close (proxy_sock);
  close (web_sock);
  return 0;

}
