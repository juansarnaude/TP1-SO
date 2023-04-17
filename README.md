# TP1 Sistemas Operativos
## Integrantes
* Juan Segundo Arnaude       Legajo: 62184
* Bautista Ignacio Canevaro  Legajo: 62179
* Matias Wodtke              Legajo: 62098

## Requerimientos
En primera instancia, se debe instalar el entorno de docker, corriendo el siguiente comando en la terminal.
```
docker pull agodio/itba-so:1.0
```  
Luego, se requiere iniciar un contenedor de docker con esa imagen con el siguiente comando.
```
docker run -v "${PWD}:/root" --privileged -ti agodio/itba-so:1.0
```
Finalmente, accedemos al directorio root del docker para usar el programa.
```
cd root
```
## Instrucciones de compilación
Para comenzar con la compilación, este proyecto cuenta con un Makefile que facilita este proceso. Simplemente ejecute el siguiente comando para compilarlo y borrar archivos de anteriores ejecuciones.
```
make clean
make all
```

## Instrucciones de ejecución
### Calcular el md5 de los archivos
La primera forma de ejecutar el programa es obteniendo los md5 en un archivo llamado `resultado.txt`. Para ello, ejecutar la siguiente línea de código.
```
./app <files-to-process>
```

### Calcular el md5 de los archivos e imprimirlos en pantalla usando pipe
La segunda forma de ejecutar el programa es obteniendo los md5 en un archivo llamado `resultado.txt` y, mediante el programa view, obtener en pantalla los resultados. Para ello, ejecutar la siguiente línea de código.
```
./app <files-to-process>|./view
```

### Calcular el md5 de los archivos e imprimirlos en pantalla usando 2 terminales
La última forma de ejecutar el programa es obteniendo los md5 en un archivo llamado `resultado.txt` y, mediante el programa view, usando 2 terminales distintas, obtener en pantalla los resultados. Para ello, ejecutar la siguiente línea de código en una terminal.
```
./app <files-to-process>
```
Luego, ejecutar la siguiente línea en un terminal aparte, en un rango de 2 segundos tras haber ejecutado el comando anterior.
```
./view <shared-memory-name> <number-of-files-processed>
```