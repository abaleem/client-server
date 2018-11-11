#include <stdio.h> // basic I/O
#include <stdlib.h>
#include <sys/types.h> // standard system types
#include <netinet/in.h> // Internet address structures
#include <sys/socket.h> // socket API
#include <arpa/inet.h>
#include <netdb.h> // host to IP resolution
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <dirent.h>
using namespace std;

#define HOSTNAMELEN 40  // maximal host name length; can make it variable if you want
#define BUFLEN 1024 	// maximum response size; can make it variable if you want


int main(int argc, char *argv[])
{

	// check that there are enough parameters
	if(argc != 3)		
    {
      fprintf(stderr, "Usage: client1  <hostname> <port>\n");
      exit(-1);
    }

	int portx = atoi(argv[2]);
	int size;
	char*  address;
	struct hostent *test;
	struct in_addr **list;


	// getting host ip from host name
	test = gethostbyname(argv[1]);
	cout << endl;
	cout  << "Running client" << endl;
	cout << "Will try to connect to " << argv[1] << " at port " << portx  << endl;
	
	if(test <= 0)
	{	cout << "unknown host " << endl; 
		exit(-1);  }

	list = (struct in_addr **)test->h_addr_list;
	address = inet_ntoa(*list[0]); 	
	cout << "hostname: " << argv[1] << " has IP: " << address << endl; 
	

	int c;
	int s; 
	char buffer[1024] = {0} ;
	struct sockaddr_in saddress;  // server address
	int check = 0;
	int bytessend=0; int bytesread =0;


	// creating a socket
	cout << ".. creating local connector socket"<<endl;
	s = socket(AF_INET, SOCK_STREAM, 0);		
	if(s<0)
	{
		cout<<"socket failure"<<endl;
		exit(-1); 
	}

	memset(&saddress, '0', sizeof(saddress));

	saddress.sin_family = AF_INET;
	saddress.sin_port = htons(portx);
	
	inet_pton(AF_INET, address, &saddress.sin_addr);   //from text to binary
	


	// connecting the socket to port
	cout <<".. connecting socket to " <<argv[1]<<":"<<portx<<endl;
	if((c = connect(s, (struct sockaddr *)&saddress, sizeof(saddress)))<0)
	{
		cout<<" connecting failed" <<endl;
		exit(-1);
	}


	// recieving initial message from server
	bytesread = recv(s, buffer, 1024,0); 
	cout << endl <<  buffer << endl << endl;

	DIR *d;
	struct dirent *dir;
	
	char input[1024];
	char filename[1024];
	char keep[10]; 
	while(true)
	{
		start:
		memset(input,0,strlen(input));
		memset(buffer,0,strlen(buffer));
		memset(filename,0,strlen(filename));
		memset(buffer,0,strlen(buffer));


		cout<<"c: ";
		cin.getline(input,sizeof(input));
		
		if(strcmp(input,"exit")==0 || bytesread == 0)
		{
			send(s,strcat(input,"\r\n"),strlen(strcat(input,"\r\n")),0);
			return 0;
		}
		else if(strncmp(input, "send ", 5) == 0)
		{	
			strncpy (filename, input+5, strlen(input)-5);
            filename[strlen(input)-5] = '\0';

        	d=opendir(".");

			if((fopen(filename, "r")) > 0)
			{			

				send(s,strcat(input,"\r\n"),strlen(strcat(input,"\r\n")),0);
				recv(s, buffer, 1024,0);
       			cout << buffer << endl; 

        	   if(strcmp(buffer,"file already exists")!=0)
 		       {
 		       		

					FILE *f = fopen(filename, "rb");
					while(1)
					{
						memset(buffer,0,strlen(buffer));
						bytesread = fread(buffer,1,1024,f);
						if(bytesread > 0)
						{
							send(s,buffer,bytesread,0);
						}
						if(bytesread < 1024)
						{
							break;
						}
					}

					cout << "sucessfully sent" << endl;
					fclose(f);
				}
			} 
			else 
			{
			    cout <<"file doesn't exist" << endl << endl;
			}

				closedir(d);
		}
		else if(strncmp(input, "recieve ", 8) == 0)
		{	

			memset(filename,0,strlen(filename));
            strncpy (filename, input+8, strlen(input)-8);
            filename[strlen(input)-8] = '\0';

            FILE *f = NULL;

            if((fopen(filename, "r")) > 0)
            { 
				cout <<"file already exists. do you want to keep(k) or replace(r)" << endl;
	        	cin >> keep;
	        	if(strcmp(keep,"r")==0) 
	        		{
	        			f = fopen(filename, "w+");
						fclose(f);
	        			cout << "replacing" << endl;
	        			goto replacefile;
	        	
		        	}
	        } 
            else 
            {	
            	replacefile:
            	cout << "here " << endl;
				send(s,strcat(input,"\r\n"),strlen(strcat(input,"\r\n")),0);
				
				memset(buffer,0,strlen(buffer));
				recv(s, buffer, 1024,0);
            	//cout << buffer << endl;
            	send(s,"ok",strlen("ok"),0);


            	if(buffer == "server sending file\r\n") 
            	{
					d=opendir(".");

					f = fopen(filename, "a");

	                while(1)
	                {
	                  memset(buffer,0,strlen(buffer));
	                  bytesread=read(s,buffer,1024);
	                  fwrite(buffer,1,bytesread,f);
	                  if(bytesread < 1024)
	                  {
	                    break;
	                  }
	                }

	                fclose(f);
	                closedir(d);
	                cout << "file copied sucessfully" << endl;
	            }
            }

		}
		else if(strncmp(input, "delete client ", 14) == 0)
		{
			send(s,strcat(input,"\r\n"),strlen(strcat(input,"\r\n")),0);	
			memset(filename,0,strlen(filename));
			strncpy (filename, input+14, strlen(input)-14);
            filename[strlen(input)-14] = '\0';
            if((fopen(filename, "r")) > 0) 
            { 
            	d=opendir(".");
                remove(filename);
            	memset(buffer,0,strlen(buffer));
            	recv(s, buffer, 1024,0);
       			cout << buffer << endl; 
	        } 
	        else
	        {
	        	cout << "file doesnt exist" << endl;
	        }



		}
		else if(strncmp(input, "delete server ", 14) == 0)
		{	
			send(s,strcat(input,"\r\n"),strlen(strcat(input,"\r\n")),0);
			memset(buffer,0,strlen(buffer));
	        recv(s, buffer, 1024,0);
       		cout << buffer << endl; 

		}
		else
		{
			cout<<"invalid command, enter again" << endl << endl;
			goto start;
		}

	}

  return 0;
}
