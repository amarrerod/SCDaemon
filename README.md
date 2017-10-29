
# Sistemas Operativos y Sistemas Empotrados
## Máster en Ingeniería Informática - ULL 

### Demonio sobre sistema distribuido simulado con vagrant

1. Crear la carpeta /test_folder/
2. Lanzar los demonios por medio de los scripts bash:
   - ./server.d start
   - ./daemon.d start
3. Para finalizar la ejecucion:
	- ./server.d stop
	- ./daemon.d stop
4. La comunicación bidireccional se muestra en el fichero */var/log/syslog*
5. Se pueden lanzar varios demonios clientes (en diferentes máquinas)
6. Se registran los eventos de las subcarpetas creadas dinámicamente