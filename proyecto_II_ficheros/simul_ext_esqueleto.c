#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cabeceras.h"

// Implementación de ComprobarComando
int ComprobarComando(char *strcomando, char *orden, char *argumento1, char *argumento2) {
    // Extrae el comando, argumento1 y argumento2 de la cadena `strcomando`
    sscanf(strcomando, "%s %s %s", orden, argumento1, argumento2);
    return strcmp(orden, "salir") != 0; // Retorna 0 si el comando es "salir"
}

void ActualizarSuperbloque(EXT_SIMPLE_SUPERBLOCK *superbloque, EXT_BYTE_MAPS *bytemaps)
{
    unsigned int numBloques = 0, numInodos = 0;
    for (int i = 0; i<MAX_INODOS; i++)
    {
        if (bytemaps->bmap_inodos[i] == 1)
        {
            numInodos++;
        }
    }
    //Actualizo el numero de inodos libres del superbloque
    superbloque->s_free_inodes_count = MAX_INODOS - numInodos;

    for (int i = 0; i < MAX_BLOQUES_PARTICION; i++)
    {
        if (bytemaps->bmap_bloques[i] == 1)
        {
            numBloques++;
        }
    }
    // Actualizo el numero de bloques libres del superbloque
    superbloque->s_free_blocks_count = MAX_BLOQUES_PARTICION - numBloques;

}
unsigned int* NumeroBloques(EXT_SIMPLE_INODE *inodo, unsigned int *num_bloques){
    unsigned int *i_nbloques = NULL;
    unsigned int capacidad = 0;
    unsigned int cantidad = 0;
    for (int j = 0; j < MAX_NUMS_BLOQUE_INODO; j++) {
        if (inodo->i_nbloque[j] != NULL_BLOQUE) { // Bloque válido
            unsigned int *temp = realloc(i_nbloques, ++capacidad * sizeof(unsigned int));
            if (temp == NULL) {
                // Si realloc falla, liberar memoria previa y manejar el error
                free(i_nbloques);
                return 0;
            }
            // Actualizar puntero y datos
            i_nbloques = temp;
            i_nbloques[cantidad] = inodo->i_nbloque[j];
            capacidad++;
            cantidad++;
        }
    }
    *num_bloques = cantidad;
    return i_nbloques;
}

// Implementación de Directorio
void Directorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos) {
    for (int i = 1; i < MAX_FICHEROS; i++) {
        if (directorio[i].dir_inodo != NULL_INODO) { // Verificar si el inodo está ocupado
            unsigned int num_bloques;
            unsigned short inodo_index = directorio[i].dir_inodo;
            EXT_SIMPLE_INODE *inodo = &inodos->blq_inodos[inodo_index]; // Obtener inodo correspondiente
            unsigned int *numeroBloques = NumeroBloques(inodo, &num_bloques);
            // Mostrar información del archivo
            printf("%-15s  tamaño:%-10u   inodo:%-2d",
                   directorio[i].dir_nfich,
                   inodo->size_fichero,
                   inodo_index);
            if (numeroBloques != NULL) {
                printf("Bloque: ");
                // Usar los bloque
                for (unsigned int i = 0; i < num_bloques; i++) {
                    printf("%u ", numeroBloques[i]);
                }

                // Liberar memoria cuando ya no la necesites
                free(numeroBloques);
            }

            printf("\n"); // Nueva línea para el siguiente archivo
        }
    }
}


// Implementación de GrabarDatos
void GrabarDatos(EXT_DATOS *memdatos, FILE *fich) {
    printf("Grabando datos en el archivo...\n");
    fwrite(memdatos, sizeof(EXT_DATOS), 1, fich);
    printf("Datos grabados correctamente.\n");
}

//Implementacion de ByteMaps
void Bytemaps(EXT_BYTE_MAPS *bytemaps) {

    printf("Inodos :");
    for (int i = 0; i < MAX_INODOS; i++) {
        printf(" %d ", bytemaps->bmap_inodos[i]);
    }
    printf("\n");

    printf("Bloques [0-25] : ");
    for (int i = 0; i < 26; i++) {
        printf(" %d ", bytemaps->bmap_bloques[i]);
    }
    printf("\n");

}

//Implementación de info
void Info(EXT_SIMPLE_SUPERBLOCK *superbloque) {
    printf("Bloque %u bytes\n", superbloque->s_block_size);
    printf("Inodos particion = %u\n", superbloque->s_inodes_count);
    printf("Inodos libres = %u\n", superbloque->s_free_inodes_count);
    printf("Bloques particion = %u\n", superbloque->s_blocks_count);
    printf("Bloques libres = %u\n", superbloque->s_free_blocks_count);
    printf("Primer bloque de datos = %u\n", superbloque->s_first_data_block);
}

void imprimirFichero(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos,
                     EXT_DATOS *memdatos, char *nombreFichero) {
    // Buscar el fichero
    int encontrado = 0;
    unsigned short int numInodo;

    for(int i = 0; i < MAX_FICHEROS; i++) {
        if(strcmp(directorio[i].dir_nfich, nombreFichero) == 0) {
            encontrado = 1;
            numInodo = directorio[i].dir_inodo;
            break;
        }
    }

    if(!encontrado) {
        printf("ERROR: Fichero %s no encontrado\n", nombreFichero);
        return;
    }

    // Obtener el tamaño del fichero
    unsigned int tamano = inodos->blq_inodos[numInodo].size_fichero;

    // Crear buffer para almacenar todo el contenido
    char *contenido = (char *)malloc(tamano + 1);
    if(!contenido) {
        printf("ERROR: No se pudo asignar memoria\n");
        return;
    }

    // Posición actual en el buffer
    unsigned int pos = 0;

    // Copiar contenido de cada bloque
    for(int i = 0; i < MAX_NUMS_BLOQUE_INODO; i++) {
        unsigned short int numBloque = inodos->blq_inodos[numInodo].i_nbloque[i];
        if(numBloque == NULL_BLOQUE) break;

        // Calcular cuántos bytes copiar de este bloque
        unsigned int bytesACopiar = (tamano - pos < SIZE_BLOQUE) ?
                                   tamano - pos : SIZE_BLOQUE;

        // Copiar datos del bloque al buffer
        memcpy(contenido + pos, memdatos[numBloque - PRIM_BLOQUE_DATOS].dato,
               bytesACopiar);
        pos += bytesACopiar;
    }

    // Añadir terminador de cadena
    contenido[tamano] = '\0';

    // Imprimir contenido y liberar memoria
    puts(contenido);
    free(contenido);
}

void renameFile(EXT_ENTRADA_DIR *directorio, char *nombre, char *nuevo_nombre)
{
    // Buscar el fichero
    for (int i = 0; i < MAX_FICHEROS; i++)
    {
        //Compruebo que existe el fichero con el nombre
        if (strcmp(directorio[i].dir_nfich, nombre) == 0)
        {
            //Comprobar que no exte un fichero con el nombre nuevo
            for (int j = 0; j < MAX_FICHEROS; j++)
            {
                if (strcmp(directorio[j].dir_nfich, nuevo_nombre) == 0)
                {
                    printf("El fichero %s ya existe\n", nuevo_nombre);
                    return;
                }
            }
            // Cambio el nombre a ese fichero
            strcpy(directorio[i].dir_nfich, nuevo_nombre);
            printf("rename %s %s\n", nombre, nuevo_nombre);
            return;
        }
    }

    printf("El fichero %s no existe\n", nombre);

}

void removeFile(EXT_ENTRADA_DIR *directorio, char *nombre, EXT_BYTE_MAPS *bytemaps, EXT_BLQ_INODOS *ext_blq_inodos)
{
    // Buscar el fichero
    for (int i = 0; i < MAX_FICHEROS; i++)
    {
        //Compruebo que existe el fichero con el nombre
        if (strcmp(directorio[i].dir_nfich, nombre) == 0)
        {
            //Marcar como libre el inodo
            bytemaps->bmap_inodos[directorio[i].dir_inodo] = 0;

            // Liberar los bloques del inodo
            for (int j = 0; j < MAX_NUMS_BLOQUE_INODO; j++) {
                if (ext_blq_inodos->blq_inodos[directorio[i].dir_inodo].i_nbloque[j] != NULL_BLOQUE) {
                    bytemaps->bmap_bloques[ext_blq_inodos->blq_inodos[directorio[i].dir_inodo].i_nbloque[j]] = 0;
                }
            }

            //Liberar la entrada del directorio
            directorio[i].dir_inodo= NULL_INODO;
            directorio[i].dir_nfich[0] = '\0';

            printf("Fichero %s eliminado\n", nombre);
            return;
        }
    }
    printf("ERROR: fichero %s no encontrado\n", nombre);
}

void copyFile(EXT_ENTRADA_DIR *directorio, EXT_BYTE_MAPS *bytemaps, EXT_BLQ_INODOS *inodos,
    EXT_DATOS *memdatos,EXT_SIMPLE_SUPERBLOCK *superbloque , char *nombre, char *nombre2) {

    /// Find source file with exact case matching
    unsigned short int inodo_origen = NULL_INODO;
    for (int i = 0; i < MAX_FICHEROS; i++) {
        if (directorio[i].dir_inodo != NULL_INODO && strcmp(directorio[i].dir_nfich, nombre) == 0) {
            inodo_origen = directorio[i].dir_inodo;
            break;
        }
    }
    if (inodo_origen == NULL_INODO) {
        printf("No existe el fichero: %s\n", nombre);
        return;
    }

    // Find free directory entry
    int dir_entry = -1;
    for (int i = 0; i < MAX_FICHEROS; i++) {
        if (directorio[i].dir_inodo == NULL_INODO) {
            dir_entry = i;
            break;
        }
    }
    if (dir_entry == -1) {
        printf("Directorio lleno\n");
        return;
    }

    // Find free inode
    unsigned short int inodo_destino = NULL_INODO;
    for (int i = 3; i < MAX_INODOS; i++) {
        if (bytemaps->bmap_inodos[i] == 0) {
            inodo_destino = i;
            break;
        }
    }
    if (inodo_destino == NULL_INODO) {
        printf("No hay inodos libres\n");
        return;
    }

    // Initialize destination inode
    bytemaps->bmap_inodos[inodo_destino] = 1;
    inodos->blq_inodos[inodo_destino].size_fichero = inodos->blq_inodos[inodo_origen].size_fichero;
    for (int i = 0; i < MAX_NUMS_BLOQUE_INODO; i++) {
        inodos->blq_inodos[inodo_destino].i_nbloque[i] = NULL_BLOQUE;
    }

    // Copy blocks
    for (int i = 0; i < MAX_NUMS_BLOQUE_INODO; i++) {
        unsigned short bloque_origen = inodos->blq_inodos[inodo_origen].i_nbloque[i];
        if (bloque_origen == NULL_BLOQUE) break;

        // Find free block
        unsigned short int bloque_destino = NULL_BLOQUE;
        for (int j = PRIM_BLOQUE_DATOS; j < MAX_BLOQUES_PARTICION; j++) {
            if (bytemaps->bmap_bloques[j] == 0) {
                bloque_destino = j;
                break;
            }
        }
        if (bloque_destino == NULL_BLOQUE) {
            printf("No hay bloques libres\n");
            // Cleanup
            bytemaps->bmap_inodos[inodo_destino] = 0;
            for (int k = 0; k < i; k++) {
                if (inodos->blq_inodos[inodo_destino].i_nbloque[k] != NULL_BLOQUE) {
                    bytemaps->bmap_bloques[inodos->blq_inodos[inodo_destino].i_nbloque[k]] = 0;
                }
            }
            return;
        }

        // Copy block data and update structures
        bytemaps->bmap_bloques[bloque_destino] = 1;
        memcpy(&memdatos[bloque_destino - PRIM_BLOQUE_DATOS],
               &memdatos[bloque_origen - PRIM_BLOQUE_DATOS],
               SIZE_BLOQUE);
        inodos->blq_inodos[inodo_destino].i_nbloque[i] = bloque_destino;
    }

    // Update directory
    strncpy(directorio[dir_entry].dir_nfich, nombre2, LEN_NFICH-1);
    directorio[dir_entry].dir_nfich[LEN_NFICH-1] = '\0';
    directorio[dir_entry].dir_inodo = inodo_destino;

    ActualizarSuperbloque(superbloque, bytemaps);

}

// Función principal
int main() {
    // Variables necesarias
    EXT_SIMPLE_SUPERBLOCK superbloque;
    EXT_BYTE_MAPS bytemaps;
    EXT_BLQ_INODOS ext_blq_inodos;
    EXT_ENTRADA_DIR directorio[MAX_FICHEROS];
    EXT_DATOS memdatos[MAX_BLOQUES_DATOS];
    char comando[SIZE_BLOQUE], orden[SIZE_BLOQUE], argumento1[SIZE_BLOQUE], argumento2[SIZE_BLOQUE];
    FILE *fent;

    // Inicialización del sistema
    fent = fopen("particion.bin", "rb+"); // Abre el archivo binario
    if (fent == NULL) {
        perror("Errorr al abrir el archivo particion.bin");
        return 1;
    }

    fread(&superbloque, sizeof(EXT_SIMPLE_SUPERBLOCK), 1, fent);
    fread(&bytemaps, sizeof(EXT_BYTE_MAPS), 1, fent);
    fread(&ext_blq_inodos, sizeof(EXT_BLQ_INODOS), 1, fent);
    fread(&directorio, sizeof(directorio), 1, fent);
    fread(&memdatos, sizeof(memdatos), 1, fent);

    // Bucle de comandos
    do {
        printf(">>");
        fgets(comando, SIZE_BLOQUE, stdin);

        if (ComprobarComando(comando, orden, argumento1, argumento2) == 0) {
            break; // Salir del bucle si el comando es "salir"
        }

        if (strcmp(orden, "dir") == 0) {
            Directorio(directorio, &ext_blq_inodos);
        }else if (strcmp(orden, "bytemaps") == 0) {
            Bytemaps(&bytemaps);
        }else if (strcmp(orden, "info") == 0){
            Info(&superbloque);
        }else if (strcmp(orden, "imprimir") == 0){
            char nombreFichero[LEN_NFICH];

            // Eliminar el salto de línea de fgets
            comando[strcspn(comando, "\n")] = 0;

            // Extraer el nombre del fichero directamente del comando
            sscanf(comando + strlen("imprimir"), "%s", nombreFichero);

            // Leer la partición completa
            unsigned char particion[SIZE_BLOQUE * MAX_BLOQUES_PARTICION];
            fseek(fent, 0, SEEK_SET);
            fread(particion, 1, sizeof(particion), fent);

            imprimirFichero(directorio, &ext_blq_inodos, particion, nombreFichero);

        }else if (strcmp(orden, "rename") == 0){
            char nombre[LEN_NFICH];
            char nuevo_nombre[LEN_NFICH];
            // Eliminar el salto de línea de fgets
            comando[strcspn(comando, "\n")] = 0;

            // Extraer el nombre del fichero directamente del comando
            sscanf(comando + strlen("rename"), "%s %s", nombre, nuevo_nombre);
            renameFile(directorio, nombre, nuevo_nombre);
        }else if (strcmp(orden, "remove") == 0)
        {
            char nombreFichero[LEN_NFICH];

            // Eliminar el salto de línea de fgets
            comando[strcspn(comando, "\n")] = 0;

            // Extraer el nombre del fichero directamente del comando
            sscanf(comando + strlen("remove"), "%s", nombreFichero);
            removeFile(directorio, nombreFichero, &bytemaps, &ext_blq_inodos);

        }else if (strcmp(orden, "copy") == 0){
            printf("Copiar\n");
            char nombreExistente[LEN_NFICH];
            char nuevo_nombre[LEN_NFICH];

            // Eliminar el salto de línea de fgets
            comando[strcspn(comando, "\n")] = 0;

            // Extraer el nombre del fichero directamente del comando
            sscanf(comando + strlen("copy"), "%s %s", nombreExistente, nuevo_nombre);


            copyFile(directorio, &bytemaps, &ext_blq_inodos, memdatos, &superbloque, nombreExistente, nuevo_nombre);

        }else {
            printf("ERROR: Comando ilegal [bytemaps,copy,info,dir,imprimir,rename,salir].\n");
        }


    } while (1);

    // Guardar cambios en el archivo binario
    fseek(fent, 0, SEEK_SET);
    fwrite(&superbloque, sizeof(EXT_SIMPLE_SUPERBLOCK), 1, fent);
    fwrite(&bytemaps, sizeof(EXT_BYTE_MAPS), 1, fent);
    fwrite(&ext_blq_inodos, sizeof(EXT_BLQ_INODOS), 1, fent);
    fwrite(&directorio, sizeof(directorio), 1, fent);
    fwrite(&memdatos, sizeof(memdatos), 1, fent);

    fclose(fent);
    printf("Sistema de archivos guardado. ¡Hasta luego!\n");
    return 0;
}
