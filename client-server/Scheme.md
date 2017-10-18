
# Servidor

- Crear un socket. Para eso usamos la función socket().
- Enlazar el socket a una dirección usando la llamada al sistema *bind*.
  - Nº de puerto + dirección del host
- Escuchar las peticiones. Función listen.
- Aceptamos las peticiones con Accept(). (Bloquea la conexión mientras se conecta con el servidor, hay que determinar el nº máximo de conexiones a la vez)
- Recibir y enviar datos. read() y write().


# Cliente

- Crear un socket.
- Conectamos con el servidor. connect().
- Enviamos y recibimos los datos.


# Tipos de socket

## Protocolos de Internet (TCP/IP)
- TCP, UDP --> Nº de puerto + dirección e destino

## UNIX
- UDP --> Nº de puerto (normalmente el 0).


TCP --> Stream sockets, flujo continuo de caracteres.
UDP --> Datagram sockets, paquetes o tramas.

