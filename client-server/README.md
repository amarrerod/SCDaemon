
# Servidor

- Crear un socket. Para eso usamos la función socket().
- Enlazar el socket a una dirección usando la llamada al sistema *bind*.
  - Nº de puerto + dirección del host
- Escuchar las peticiones. Función listen.
- Aceptamos las peticiones con Accept(). (Bloquea la conexión mientras se conecta con el servidor, hay que determinar el nº máximo de conexiones a la vez)
- Recibir y enviar datos. read() y write().

```cpp
struct sockaddr_in {
	short sin_family; // (AF_INET / AF_UNIX)
	u_short sin_port; // Nº de puerto --> usamos la función htons
	struct in_addr sin_addr; // IP del host  (INADDR_ANY coge la dirección de la máquina)
	char sin_zero[0];
}

sockfd = socket(AF_INET/AF_UNIX, SOCK_STREAM/SOCK_DGRAM, 0);
bind(sockfd, (struct sockaddr*)& serv_addr, sizeof(serv_addr));
listen(sockfd, NUM_CONEXIONES);
newsockfd = accept(sockfd, (struct sockaddr*)& cli_addr, sizeof(cli_addr));
read(newsockfd, buffer, 255);
write(newsockfd, "HOLA", 4);
```


# Cliente

- Crear un socket.
- Conectamos con el servidor. connect().
- Enviamos y recibimos los datos.

```cpp
struct hostent{
	char*  hname; // Nombre del host
	char** h_aliases; // Alias del host
	int    h_addrtype; // Tipo de dirección
	int    h_length;   // Longitud de la dirección
	char** h_addr_list; // Lista de dirección del servidor
	#define h_addr h_addr_list[0]; // Compatibilidad de versiones
}
connect(sockfd, &serv_addr, sizeof(serv_addr));
```

# Tipos de socket

## Protocolos de Internet (TCP/IP)
- TCP, UDP --> Nº de puerto + dirección e destino

## UNIX
- UDP --> Nº de puerto (normalmente el 0).


TCP --> Stream sockets, flujo continuo de caracteres.
UDP --> Datagram sockets, paquetes o tramas.


