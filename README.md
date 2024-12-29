# Simulador de Sistema de Ficheros

## Integrantes
 - Pablo Jiménez
 - Gonzalo Herrera


## Video explicacion ejecucion
 [Link_video](https://liveutad-my.sharepoint.com/:v:/g/personal/pablo_jimenez2_live_u-tad_com/EbD46Hv8HdBNqDxLXRWcARMBPEH2kB78m2wvoNiNRkmVrg?e=L4QtNL)

# DOCS
 # Simulador de Sistema de Archivos EXT

Una implementación simplificada de un simulador de sistema de archivos tipo Linux que opera sobre un archivo binario (`particion.bin`) en lugar de una partición real del disco.

## Descripción General

Este proyecto implementa un simulador básico de sistema de archivos EXT con las siguientes características principales:

* Tamaño de bloque fijo de 512 bytes
* Partición de 100 bloques
* Estructura de inodos simplificada
* Soporte para hasta 20 archivos
* Operaciones básicas de archivos (crear, leer, eliminar, copiar)

## Estructura del Sistema de Archivos

* **Bloque 0**: Superbloque
* **Bloque 1**: Mapas de bytes (para inodos y bloques)
* **Bloque 2**: Lista de inodos
* **Bloque 3**: Directorio
* **Bloques 4-99**: Bloques de datos

## Comandos Disponibles

* `info`: Muestra información del superbloque
* `bytemaps`: Muestra el estado de asignación de inodos y bloques
* `dir`: Lista todos los archivos y sus propiedades
* `rename <viejo> <nuevo>`: Renombra un archivo
* `imprimir <archivo>`: Muestra el contenido del archivo
* `remove <archivo>`: Elimina un archivo
* `copy <origen> <destino>`: Copia un archivo
* `salir`: Sale del simulador

## Detalles Técnicos

### Estructuras Principales

* **Superbloque**: Contiene metadatos del sistema de archivos
* **Mapas de bytes**: Controla la asignación de inodos y bloques
* **Inodos**: Almacena metadatos de archivos y punteros a bloques
* **Entradas de Directorio**: Mapea nombres de archivo a inodos

### Limitaciones

* Máximo 24 inodos
* Máximo 20 archivos
* 7 punteros directos a bloques por inodo
* Tamaño de bloque fijo de 512 bytes

## Compilación y Ejecución

1. Compilar el simulador:
```bash
gcc -o simul_ext simul_ext.c
```

2. Asegurarse de que `particion.bin` existe en el mismo directorio

3. Ejecutar el simulador:
```bash
./simul_ext
```

## Manejo de Errores

El simulador incluye verificación de errores para:

* Existencia de archivos
* Disponibilidad de recursos (inodos/bloques libres)
* Validez de comandos
* Consistencia del sistema de archivos

## Notas de Implementación

* Utiliza mapas de bytes en lugar de mapas de bits por simplicidad
* No hay soporte para subdirectorios
* Nombres de archivo sensibles a mayúsculas y minúsculas
* Todas las operaciones mantienen la consistencia del sistema de archivos
* Guardado automático de cambios en el archivo de partición

## Uso del Proyecto

Este proyecto fue desarrollado como parte de la asignatura de Sistemas Operativos. Implementa un sistema de archivos simplificado que permite entender los conceptos básicos de:

* Gestión de archivos
* Asignación de espacio
* Estructuras de datos del sistema de archivos
* Operaciones básicas sobre archivos

## Autores

* [Pablo Jimenez]
* [Gonzalo Herrera]
