/*******************************************************
Protocolos de Transporte
Grado en Ingeniería Telemática
Dpto. Ingeníería de Telecomunicación
Univerisdad de Jaén

Fichero: servidor.c
Versión: 1.0
Fecha: 23/09/2012
Descripción:
	Servidor de eco sencillo TCP.

Autor: Juan Carlos Cuevas Martínez

*******************************************************/
#include <stdio.h>
#include <string.h>
#include <conio.h>
#include <Winsock2.h>

#include "protocol.h"




main()
{
	//variables del servidor
	WORD wVersionRequested;
	WSADATA wsaData;
	//creacion sockets
	SOCKET sockfd,nuevosockfd;
	//crea estructura de los socket
	struct sockaddr_in  local_addr,remote_addr;
	
	/*
	buffer_out: tamaño maximo del mesanje enviado
	buffer_in: tamaño maximo del mensaje recibido
	cmd:tamaña para
	usr:tamaño para introducir el usuario
	pas:tamaño para introducir la contraseña
	*/
	char buffer_out[1024],buffer_in[1024], cmd[10], usr[10], pas[10];
	//definimos los parametros para el caso de S_SUM
	char sm[20];
	int suma=0,Num1,Num2;

	int err,tamanio;
	int fin=0, fin_conexion=0;
	int recibidos=0,enviados=0;
	int estado=0;

	/** INICIALIZACION DE BIBLIOTECA WINSOCK2 **
	 ** OJO!: SOLO WINDOWS                    **/
	wVersionRequested=MAKEWORD(1,1);
	err=WSAStartup(wVersionRequested,&wsaData);
	if(err!=0){
		return(-1); //si falla nos devuelve -1 y sabemos que es fallo del api de la libreria de los sockets
	}
	if(LOBYTE(wsaData.wVersion)!=1||HIBYTE(wsaData.wVersion)!=1){
		WSACleanup() ;
		return(-2); //si falla nos devuelve -2 y sabemos que es fallo del api de la libreria de los sockets
	}
	/** FIN INICIALIZACION DE BIBLIOTECA WINSOCK2 **/

	//creacion del nuevo socket llamado sockfd y la conexion con el servicio
	//AF_INET: indica que es para ipv4
	//SOCK_STREAM: usa conexión TCP
	//0 :protocolo por defecto

	sockfd=socket(AF_INET,SOCK_STREAM,0);//Creación del socket


	if(sockfd==INVALID_SOCKET)	{ //si el socket no se crea bien INVALID_SOCKET devuelve -3 y sabemos que es fallo de la creacion del nuevo socket
		return(-3);
	}
	//si el socket se crea  correctamente realizamos las estructura de la direccion IPV4
	else {
		local_addr.sin_family		=AF_INET;			// Familia de protocolos de Internet
		local_addr.sin_port			=htons(TCP_SERVICE_PORT);	// Puerto del servidor
		local_addr.sin_addr.s_addr	=htonl(INADDR_ANY);	// Direccion IP del servidor Any cualquier disponible
													// Cambiar para que conincida con la del host
	}
	//PRIMITIVA BIND
	//relaciona una direccion a un socket
	
	// Enlace el socket a la direccion local (IP y puerto)
	if(bind(sockfd,(struct sockaddr*)&local_addr,sizeof(local_addr))<0)
		return(-4);
	//PRIMITIVA LISTEN
	//las solicitudes de conexion son almacenadas en una cola
	
	//Se prepara el socket para recibir conexiones y se establece el tamaño de cola de espera
	if(listen(sockfd,5)!=0) //Longitud máxima de la cola de conexiones pendientes es de 5
		return (-6);
	
	tamanio=sizeof(remote_addr);

	//CAPA DE TRANSPORTE
	do
	{
		//nos muestra por pantalla que esta esperando una conexion
		printf ("SERVIDOR> ESPERANDO NUEVA CONEXION DE TRANSPORTE\r\n");
		
		//se crea un nuevo socket para la realizacion de la conexion
		nuevosockfd=accept(sockfd,(struct sockaddr*)&remote_addr,&tamanio);

		//si el socket no se crea bien INVALID_SOCKET devuelve -5 y sabemos que es fallo de la creacion del nuevo socket
		if(nuevosockfd==INVALID_SOCKET) {
			
			return(-5);
		}
		//si el socket se ha creado correctamente se muestra por pantalla la direccion IP y el puerto
		//ip--> inet_ntoa
		//puerto-->ntohs
		printf ("SERVIDOR> CLIENTE CONECTADO\r\nSERVIDOR [IP CLIENTE]> %s\r\nSERVIDOR [CLIENTE PUERTO TCP]>%d\r\n",
					inet_ntoa(remote_addr.sin_addr),ntohs(remote_addr.sin_port));

		//Mensaje de Bienvenida
		sprintf_s (buffer_out, sizeof(buffer_out), "%s Bienvenindo al servidor de ECO%s",OK,CRLF);
		
		//PRIMITIVA SEND PARA EL ENVIO DE MENSAJES
		//nuevosockfd--> sockets en uso
		//buffer_out--> mensaje que se va a enviar
		//(int)strlen(buffer_out)--> nos indica la longuitud del mensaje que se tiene en la salida sin incluir el nulo del final
		enviados=send(nuevosockfd,buffer_out,(int)strlen(buffer_out),0);
		//TODO Comprobar error de envío
		
		//Realizamos la comprobacion del envio del mensaje de la misma forma que se hace en el cliente
		//---------------------------------------------------------------------------------------------
		if(enviados<=0) {
			DWORD error=GetLastError();
					//si el envio de datos falla nos sale SOCKET_EROOR=-1
					if(enviados<0){
						printf("SERVIDOR> Error %d en el envio de datos\r\n",error);
						fin_conexion=1;
						continue;
					}
					else{ // si no, es decir, si su valor es 0 se cierra la conexion
						printf("SERVIDOR> Conexión con el servidor cerrada\r\n");
						fin_conexion=1;
						continue;
					}
		}
		else{ // el valor es mayor que 0 y por tanto los datos se envian correctamente
						printf("SERVIDOR> Datos enviados correctamente\r\n");
		}

		//--------------------------------------------------------------------------------------------
		
		//Se reestablece el estado inicial cambiando al estado usuario que el volver al principio
		estado = S_USER;
		fin_conexion = 0; // cuando esta a 1 se cierra la conexion y se se qeuda en 0 la mantiene abierta

		printf ("SERVIDOR> Esperando conexion de aplicacion\r\n");
		do
		{
			//PRIMITIVA RECV la cual se usa para el recibimiento de los mensajes
			//Se espera un comando del cliente
			//nuevosockfd--> sockets en uso
		    //buffer_in--> mensaje que se va a recibir
			//el tamaño maximo es de 1023 bytes
			//0:como va la llamada
			recibidos = recv(nuevosockfd,buffer_in,1023,0);
			//TODO Comprobar posible error de recepción

			//Realizamos la comprobacion de la recepcion del mensaje de la misma forma que se hace en el cliente
		//---------------------------------------------------------------------------------------------
		if(recibidos<=0) {
			DWORD error=GetLastError();
					//si el envio de datos falla nos sale SOCKET_EROOR=-1
					if(recibidos<0){
						printf("SERVIDOR> Error %d en la recepcion de datos\r\n",error);
						fin_conexion=1;
						continue;
					}
					else{ // si no, es decir, si su valor es 0 se cierra la conexion
						printf("SERVIDOR> Conexión con el servidor cerrada\r\n");
						fin_conexion=1;
						continue;
					}
		}
		else{ // el valor es mayor que 0 y por tanto los datos se envian correctamente
						printf("SERVIDOR> Datos recibidos correctamente\r\n");
		}

		//-------------------------------------------------------------------------------------------
			
			
			buffer_in[recibidos] = 0x00;
			//nos reafirma que los datos has sido recibidos correctamente
			//recibidos--> tamaño del mensaje recibido
			//buffer:in--> guarda el mensaje recibido
			printf ("SERVIDOR [bytes recibidos]> %d\r\nSERVIDOR [datos recibidos]>%s", recibidos, buffer_in);
			
			//MAQUINA DE ESTADOS
			switch (estado)
			{
				//PRIMER ESTADO, USER
				case S_USER:    /*****************************************/
					
					//strncpy:Copia caracteres de fuente a destino en caso de que la fuente sea mayor que el destino se rellena con ceros
					//cmd:Puntero que nos marca donde se va a copiar lo seleccionado
					//sizeof(cmd):dimension de la matriz cmd
					//buffer_in: mensaje que se va a copiar
					strncpy_s ( cmd, sizeof(cmd),buffer_in, 4);
					cmd[4]=0x00; // en C los arrays finalizan con el byte 0000 0000

					//coteja el cliente que ha pedido la conexion con la matriz cmd
					if ( strcmp(cmd,SC)==0 ) // si recibido es solicitud de conexion de aplicacion
					{
						//se lee el usuario introducido por el cliente y lo guarda en la variable urs de la que calcula su dimension
						sscanf_s (buffer_in,"USER %s\r\n",usr,sizeof(usr));
						
						// envia OK acepta todos los usuarios hasta que tenga la clave
						sprintf_s (buffer_out, sizeof(buffer_out), "%s%s", OK,CRLF);
						
						estado = S_PASS;
						printf ("SERVIDOR> Esperando clave\r\n");
					} else
						//coteja el cliente que ha pedido la conexion con la matriz cmd
					if ( strcmp(cmd,SD)==0 )
					{
						sprintf_s (buffer_out, sizeof(buffer_out), "%s Fin de la conexión%s", OK,CRLF);
						fin_conexion=1;
					}
					else //si se introduce un usuario erroneo nos lo muestra por pantalla
					{
						sprintf_s (buffer_out, sizeof(buffer_out), "%s Comando incorrecto%s",ER,CRLF);
					}
				break;

				case S_PASS: /******************************************************/

					//se copia la contraseña introducida por el usuario que esta guardada en buffer_in a cmd
					strncpy_s ( cmd, sizeof(cmd), buffer_in, 4);
					cmd[4]=0x00; // en C los arrays finalizan con el byte 0000 0000

					if ( strcmp(cmd,PW)==0 ) // si comando recibido es password
					{
						//guarda la contraseña en la variable pas
						sscanf_s (buffer_in,"PASS %s\r\n",pas,sizeof(usr));

						if ( (strcmp(usr,USER)==0) && (strcmp(pas,PASSWORD)==0) ) // si password y usuario recibido es correcto
						{
							// envia aceptacion de la conexion de aplicacion, nombre de usuario y
							// la direccion IP desde donde se ha conectado
							sprintf_s (buffer_out, sizeof(buffer_out), "%s %s IP(%s)%s", OK, usr, inet_ntoa(remote_addr.sin_addr),CRLF);
							//nos vamos de estado al S_DATA
							estado = S_DATA;
							printf ("SERVIDOR> Esperando comando\r\n");
						}
						else
						{
							//si la contraseña o el usuario son incorrectos nos avisa
							sprintf_s (buffer_out, sizeof(buffer_out), "%s Autenticación errónea%s",ER,CRLF);
						}
					} else
						//coteja el cliente que ha pedido la conexion con la matriz cmd
					if ( strcmp(cmd,SD)==0 )
					{
						sprintf_s (buffer_out, sizeof(buffer_out), "%s Fin de la conexión%s", OK,CRLF);
						fin_conexion=1; //cerramos conexion
					}
					else
					{
						//para los comandos que no se correspondan con PASS o QUIT indica que no es correcto
						sprintf_s (buffer_out, sizeof(buffer_out), "%s Comando incorrecto%s",ER,CRLF);
					}
				break;

				//---------------------------------------------------------------------------------------------------
		

				//aqui se aguarda la recepcion de los mensajes por el usuario
				case S_DATA: /***********************************************************/
					
					buffer_in[recibidos] = 0x00;

					//se copia el comando introducida por el usuario que esta guardado en buffer_in a cmd y se nuestra
					strncpy_s(cmd,sizeof(cmd), buffer_in, 4);

					printf ("SERVIDOR [Comando]>%s\r\n",cmd);
					
					//si recibe un SD-->QUIT se acaba la conexion y fin_conexion=1
					if ( strcmp(cmd,SD)==0 )
					{
						sprintf_s (buffer_out, sizeof(buffer_out), "%s Fin de la conexión%s", OK,CRLF);
						fin_conexion=1;
					}
					//si recibe un SD2-->EXIT se acaba la conexion y fin_conexion=1 y fin=1
					else if (strcmp(cmd,SD2)==0)
					{
						sprintf_s (buffer_out, sizeof(buffer_out), "%s Finalizando servidor%s", OK,CRLF);
						fin_conexion=1;
						fin=1;
					}

						else if(strcmp(cmd,SUM)==0)
					{
						Num1=0;
						Num2=0;
						sscanf_s(buffer_in, "SUM %d %d\r\n",&Num1,&Num2);
						if((Num1>0 && Num1<10000) && (Num2>0 && Num2<10000))
						{
							suma=Num1+Num2;
							printf("%d",suma);
							sprintf_s(buffer_out, sizeof(buffer_out), "%s %d%s",OK, suma, CRLF);
						}else
							sprintf_s(buffer_out, sizeof(buffer_out), "ERROR--->INTRODUZCA OTRO VALOR%s",CRLF);

					}
					else
					{
						sprintf_s (buffer_out, sizeof(buffer_out), "%s Comando incorrecto%s",ER,CRLF);
					}
					break;
					
				default:
					break;
					
			} // switch

			enviados=send(nuevosockfd,buffer_out,(int)strlen(buffer_out),0);
			//TODO 


		} while (!fin_conexion);
		printf ("SERVIDOR> CERRANDO CONEXION DE TRANSPORTE\r\n");
		shutdown(nuevosockfd,SD_SEND);
		closesocket(nuevosockfd);

	}while(!fin);

	printf ("SERVIDOR> CERRANDO SERVIDOR\r\n");

	return(0);
} 
