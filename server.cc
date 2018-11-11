#include <stdio.h>        // basic I/O
#include <stdlib.h>       
#include <sys/types.h>    // standard system types
#include <netinet/in.h>   // Internet address structures
#include <sys/socket.h>   // socket API
#include <arpa/inet.h>    
#include <netdb.h>        // host to IP resolution
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <dirent.h>
using namespace std;

#define HOSTNAMELEN 40   // maximal host name length; can make it variable if you want
#define BUFLEN 1024 	   // maximum response size; can make it variable if you want


int main(int argc, char *argv[])
{
				  
  if (argc != 3 || atoi(argv[2]) < 1024 || atoi(argv[2]) > 65535 )		// check that there are enough parameter
  {
    fprintf(stderr, "Usage: client1  <hostname> <portnumber between 1024 and 65535>\n");
    exit(-1);
  }

	int port = atoi(argv[2]);
  cout << "port  = "<< port << endl;

  int soc;
  int newsoc = 0;
  struct sockaddr_in serverAddress;  // server address


  int opt = 1;
  int unarary = sizeof(serverAddress);
  int bytessend = 0; int bytesread =0;

  char buffer[1024] = {0};
  char welcome[100] = "Hello from the server";
  char goodbye[100] = "Alright.. see you later\r\n";

  cout << "Starting to run server at port " << port << endl;
  

  // creating socket
  cout << ".. creating local listener socket" << endl;
  if((soc = socket(AF_INET, SOCK_STREAM, 0)) <= 0)
	{
		cout<<"socket creation failed"<<endl;
		exit(-1);
	}


  if(setsockopt(soc, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,&opt, sizeof(opt)))
  {
      cout << "port already in use" <<endl;
      exit(-1);
  }

  serverAddress.sin_family = AF_INET;
  serverAddress.sin_addr.s_addr = (INADDR_ANY); //  localhost, hence we use INADDR_ANY
  serverAddress.sin_port = htons(port);


 	// bind to the port
 	cout << ".. binding socket to port: " << port << endl;
  if(bind(soc,(struct sockaddr*)&serverAddress,sizeof(serverAddress))<0)
  {
  	cout<<"socket binding failed"<<endl;
  	exit(-1);
  }


  int check;
  DIR *d;
  struct dirent *dir;
  char msg[1024] = {0};
  char temp[255] ={0};
  char filename[1024];


  while(1)
 	{
 		check = 0;

    // listening
    cout << ".. starting to listen at the port" <<endl;
    listen(soc,5);

    // connecting
    
    cout << ".. waiting for connection" << endl;

    newsoc = accept(soc, (struct sockaddr*)&serverAddress,(socklen_t*)&unarary);
    cout << "connected " << endl;
	  
    bytessend = send(newsoc,welcome,strlen(welcome),0);
     

		while(check == 0)
		{

			memset(buffer,0,strlen(buffer));
			bytesread = recv(newsoc, buffer, 1024,0);
			cout<<"c: " << buffer << endl;
			

			if(((strcmp(buffer,"exit\r\n"))==0) || (bytesread == 0))
			{
				send(newsoc,goodbye,strlen(goodbye),0);
				cout << "lets do that again sometime\r\n" << endl;
				check  = 1;
			}
      else if(strncmp(buffer, "send ", 5) == 0)
      {
        
        memset(filename,0,strlen(filename));
        strncpy (filename, buffer+5, strlen(buffer)-5);
        filename[strlen(buffer)-5] = '\0';

        d=opendir(".");

        if((fopen(filename, "r")) > 0)
        { 
            send(newsoc,"file already exists",strlen("file already exists"),0);
        } 
        else 
        {
            send(newsoc,"copying file to server",strlen("copying file to server"),0);
            FILE *f = fopen(filename, "ab");

            while(1)
            {
              memset(buffer,0,strlen(buffer));
              bytesread=read(newsoc,buffer,1024);
              fwrite(buffer,1,bytesread,f);
              if(bytesread < 1024)
              {
                break;
              }
            }

            fclose(f);
            cout << "file copied sucessfully" << endl;
          }

       closedir(d);
      }
      else if(strncmp(buffer, "recieve ", 8) == 0)
      {

          memset(filename,0,strlen(filename));
          strncpy (filename, buffer+8, strlen(buffer)-8);
          filename[strlen(buffer)-8] = '\0';

          d=opendir(".");

          if((fopen(filename, "r")) > 0)
          { 
            memset(buffer,0,strlen(buffer));
            send(newsoc,"server sending file\r\n",strlen("server sending file\r\n"),0);
            recv(newsoc, buffer, 1024,0);

            FILE *f = fopen(filename, "r");
            while(1)
            {
              memset(buffer,0,strlen(buffer));
              bytesread = fread(buffer,1,1024,f);
              if(bytesread > 0)
              {
                send(newsoc,buffer,bytesread,0);
              }
              if(bytesread < 1024)
              {
                break;
              }
            }

            cout << "sucessfully sent" << endl;
            fclose(f);
          } 
          else 
          {
            send(newsoc,"file doesnt exist",strlen("file doesnt exist"),0);
          }

          closedir(d);

      }
      else if(strncmp(buffer, "delete client ", 14) == 0)
      {
        
        send(newsoc,"file deleted at client",strlen("file deleted at client"),0);

      }
      else if(strncmp(buffer, "delete server ", 14) == 0)
      { 

        memset(filename,0,strlen(filename));
        strncpy (filename, buffer+14, strlen(buffer)-14);
        filename[strlen(buffer)-14] = '\0';

            if((fopen(filename, "r")) > 0)
            { 
                d=opendir(".");
                remove(filename);
                send(newsoc,"file deleted at server",strlen("file deleted at server"),0);
            } 
            else
            {
              
              send(newsoc,"file doesnt exist",strlen("file doesnt exist"),0);
            }
      }
			else
			{
				bytessend = send(newsoc,"send a valid command",strlen("send a valid command"),0);
			}
    }
  }
  return 0;
}
