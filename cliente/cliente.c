/*******************************************************
Protocolos de Transporte
Grado en Ingeniería Telemática
Dpto. Ingeníería de Telecomunicación
Univerisdad de Jaén

Fichero: cliente.c
Versión: 1.0
Fecha: 23/09/2012
Descripción:
	Cliente de eco sencillo TCP.

Autor: Juan Carlos Cuevas Martínez

*******************************************************/
#include <stdio.h>
#include <winsock.h>
#include <time.h>
#include <conio.h>

#include "protocol.h"





int main(int *argc, char *argv[])
{
	SOCKET sockfd; //Crea el socket
	struct sockaddr_in server_in; //crea la estructura del socket y la añade al servidor
	char buffer_in[1024], buffer_out[1024],input[1024]; // Es el tamaño maximo de 1024 del buffer de los datos recibidos, enviados y los que entran.
	int recibidos=0,enviados=0; //Inicializacion de variables en 0
	int estado=S_HELO; //Primer valor en la variable estado ya que será una maquina de estados
	char option; //declaramos option

	WORD wVersionRequested;
	WSADATA wsaData;
	int err;

    char ipdest[16]; //tamaño maximo para la ip de destino que se almacena en forma vectores
	char default_ip[16]="127.0.0.1";//tamaño maximo para la ip por defecto que la predefinimos.
	
	//Inicialización Windows sockets
	wVersionRequested=MAKEWORD(1,1);
	err=WSAStartup(wVersionRequested,&wsaData);
	if(err!=0)
		return(0);

	if(LOBYTE(wsaData.wVersion)!=1||HIBYTE(wsaData.wVersion)!=1)
	{
		WSACleanup();
		return(0);
	}
	//Fin: Inicialización Windows sockets

	do{
		/* DECLARACION DE LA PRIMITIVA DE LOS SOCKET
		AF_INET: definido para ipv4
		SOCK_STREAM: para conexiones TCP
		0: se pone porque es por defecto
		*/
		sockfd=socket(AF_INET,SOCK_STREAM,0);

		if(sockfd==INVALID_SOCKET) // es un bucle para comprobar si se ha creado bien el socket si no lo hace bien imprime por pantalla error al crear socket y sale.
		{
			printf("CLIENTE> ERROR AL CREAR SOCKET\r\n");
			exit(-1);
		}
		else //Si crea bien el socket pide la ip de destino por pantalla(donde queremos acceder).
		{
			printf("CLIENTE> SOCKET CREADO CORRECTAMENTE\r\n");

		
			printf("CLIENTE> Introduzca la IP destino (pulsar enter para IP por defecto): ");
			gets(ipdest);//obtiene la ip de destino en la variable ipdest y almacena en la variable
			
			if(strcmp(ipdest,"")==0) // Compara con una cadena de caracteres vacia y al ser enter una cadena de caracteres vacia se cumple y copia la de default en la ipdest
				strcpy(ipdest,default_ip);


			server_in.sin_family=AF_INET;
			server_in.sin_port=htons(TCP_SERVICE_PORT);
			server_in.sin_addr.s_addr=inet_addr(ipdest);// es para si metes una ip de manera numerica unicamente
			
			estado=S_HELO; // Asigana estado a S_HELO
		
			//PRIMITIVA CONNECT
			// establece la conexion de transporte
			if(connect(sockfd,(struct sockaddr*)&server_in,sizeof(server_in))==0)
			{
				printf("CLIENTE> CONEXION ESTABLECIDA CON %s:%d\r\n",ipdest,TCP_SERVICE_PORT);
			
		
				//Inicio de la máquina de estados
				do{
					switch(estado)
					{
					case S_HELO:
						// Se recibe el mensaje de bienvenida
						break;
					case S_USER://estado usuario para escribir el usuario
						// establece la conexion de aplicacion 
						printf("CLIENTE> Introduzca el usuario (enter para salir): ");
						gets(input);
						if(strlen(input)==0) // si no se introduce ningun caracter se sale porque pasa al estado QUIT
						{
							sprintf_s (buffer_out, sizeof(buffer_out), "%s%s",SD,CRLF);
							estado=S_QUIT;
						}
						else

						sprintf_s (buffer_out, sizeof(buffer_out), "%s %s%s",SC,input,CRLF);
						break;
					case S_PASS:// estado contraseña para escribir la contraseña
						printf("CLIENTE> Introduzca la clave (enter para salir): ");
						gets(input);
						if(strlen(input)==0)// si no se introduce ningun caracter se sale porque pasa al estado QUIT
						{
							sprintf_s (buffer_out, sizeof(buffer_out), "%s%s",SD,CRLF);
							estado=S_QUIT;
						}
						else
							sprintf_s (buffer_out, sizeof(buffer_out), "%s %s%s",PW,input,CRLF);
						break;
					case S_DATA://estado data que es para enviar datos
						printf("CLIENTE> Introduzca datos (enter o QUIT para salir): ");
						gets(input);
						if(strlen(input)==0)
						{
							sprintf_s (buffer_out, sizeof(buffer_out), "%s%s",SD,CRLF);
							estado=S_QUIT;
						}
						else //todo lo que se imprima por pantalla se envia al servidor
							sprintf_s (buffer_out, sizeof(buffer_out), "%s%s",input,CRLF);
						break;
				 
				
					}
					//Envio
					if(estado!=S_HELO)
					// Ejercicio: Comprobar el estado de envio
						// La primitiva SEND se utiliza para el envio de datos y en este caso esta compuesta por:
							//sockfd: Socket para recibir y enviar
							// buffer_out: puntero que apunta al mensaje que se quiere enviar.
							//(int)strlen(buffer_out): Tamaño en bytes del mensaje que se va a enviar.
							//0: es como va la llamada.

						enviados=send(sockfd,buffer_out,(int)strlen(buffer_out),0);
					//-*-*-*-*-*-*-*-*-*-*-*-*--*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
					/* vamos ha realizar una comprobacion de los datos enviados muy parecida a lo que se hace en recibidos*/
					if(enviados<=0){// si los datos enviados tienen un valor de menor o igual que 0
						DWORD error=GetLastError();
						if(enviados<0){ // si los datos enviados tienen un valor de -1 hay error
							printf("SERVIDOR> Error %d en el envio de datos\r\n", error);
							estado=S_QUIT;
							continue;
						} 
						else{ // si no, es decir, si su valor es 0 se cierra la conexion
							printf("SERVIDOR> Conexión con el cliente cerrada\r\n");
							estado=S_QUIT;
							continue;
						}
					}
					else{ // el valor es mayor que 0 y por tanto los datos se envian correctamente
						printf("SERVIDOR> Datos enviados correctamente\r\n");}
				}
					
				//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-	
					
					
					
					
					//Recibo
				
					//La primitiva RECV se utiliza para recibir datos:
						//sockfd : Socket que se usa para enviar o recibir.
						//buffer_in: puntero que apunta al mensaje que se quiere recibir.
						//512: Tamaño en bytes del mensaje que recibimos
						//0: es como va la llamada.
						
					recibidos=recv(sockfd,buffer_in,512,0);

					if(recibidos<=0)
					{
						DWORD error=GetLastError();
						if(recibidos<0)
						{
							printf("CLIENTE> Error %d en la recepción de datos\r\n",error);
							estado=S_QUIT;
							continue;
						}
						else
						{
							printf("CLIENTE> Conexión con el servidor cerrada\r\n");
							estado=S_QUIT;
							continue;
						
					
						}
					}else
					{
						buffer_in[recibidos]=0x00;
						printf(buffer_in);
						if(estado!=S_DATA && strncmp(buffer_in,OK,2)==0) // si el estado es diferente de DATA y comparamos los dos primeros caracteres del mensaje recibido y OK y si coinciden pasa al siguiente estado.
							estado++;  
					}
					
				}while(estado!=S_QUIT);
				
	
		
			}
			else
			{
				printf("CLIENTE> ERROR AL CONECTAR CON %s:%d\r\n",ipdest,TCP_SERVICE_PORT);
			}		
			// fin de la conexion de transporte
			closesocket(sockfd);//cerramos el socket
			
		}	
		printf("-----------------------\r\n\r\nCLIENTE> Volver a conectar (S/N)\r\n");
		option=_getche();

	}while(option!='n' && option!='N');

	
	
	return(0);

}
