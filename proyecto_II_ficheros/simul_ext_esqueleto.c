#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cabeceras.h" // Incluimos el archivo de cabeceras proporcionado

// Implementación de ComprobarComando
int ComprobarComando(char *strcomando, char *orden, char *argumento1, char *argumento2) {
    // Extrae el comando, argumento1 y argumento2 de la cadena `strcomando`
    sscanf(strcomando, "%s %s %s", orden, argumento1, argumento2);
    return strcmp(orden, "salir") != 0; // Retorna 0 si el comando es "salir"
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
void imprimirFichero(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *blq_inodos, unsigned char *particion, char *nombreFichero) {
    int i, encontrado = 0;
    unsigned short int num_inodo = 0xFFFF;

    // 1. Buscar el fichero en el directorio
    for (i = 1; i < MAX_FICHEROS; i++) {
        if (strcmp(directorio[i].dir_nfich, nombreFichero) == 0) {
            num_inodo = directorio[i].dir_inodo;
            encontrado = 1;
            break;
        }
    }

    // Si no se encuentra el fichero, mostrar error
    if (!encontrado) {
        printf("Error: Fichero no encontrado\n");
        return;
    }

    // 2. Obtener inodo
    EXT_SIMPLE_INODE *inodo = &(blq_inodos->blq_inodos[num_inodo]);

    // 3. Verificar que el fichero tiene contenido
    if (inodo->size_fichero == 0) {
        printf("El fichero está vacío\n");
        return;
    }
    // Recorrer los bloques del inodo
    for (i = 0; i < MAX_NUMS_BLOQUE_INODO; i++) {
        // Parar si no hay más bloques (marcador FFFF)
        if (inodo->i_nbloque[i] == 0xFFFF) {
            break;
        }

        // Calcular dirección del bloque en la partición
        unsigned char *bloque = particion + (inodo->i_nbloque[i] * SIZE_BLOQUE);

        // Imprimir contenido del bloque
        // Si es el último bloque, imprimir solo hasta size_fichero
        if (i == (MAX_NUMS_BLOQUE_INODO - 1) || inodo->i_nbloque[i+1] == 0xFFFF) {
            // Último bloque, imprimir solo bytes necesarios
            int bytes_a_imprimir = inodo->size_fichero % SIZE_BLOQUE;
            if (bytes_a_imprimir == 0) bytes_a_imprimir = SIZE_BLOQUE;

            for (int j = 0; j < bytes_a_imprimir; j++) {
                printf("%c", bloque[j]);
            }
        } else {
            // Bloque completo
            for (int j = 0; j < SIZE_BLOQUE; j++) {
                printf("%c", bloque[j]);
            }
        }
    }

    printf("\n");
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
void removeFile(EXT_ENTRADA_DIR *directorio, char *nombre)
{
    // Buscar el fichero
    for (int i = 0; i < MAX_FICHEROS; i++)
    {
        //Compruebo que existe el fichero con el nombre
        if (strcmp(directorio[i].dir_nfich, nombre) == 0)
        {
            printf("Fichero enncontrado\n");
            return;
        }
    }
    printf("El fichero %s no existe\n", nombre);
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

            // Leer los datos del archivo
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
            printf("Nombre: %s\n", nombre);
            printf("Nuevo nombre: %s\n", nuevo_nombre);
            renameFile(directorio, nombre, nuevo_nombre);
        }else if (strcmp(orden, "remove") == 0)
        {
            char nombreFichero[LEN_NFICH];

            // Eliminar el salto de línea de fgets
            comando[strcspn(comando, "\n")] = 0;

            // Extraer el nombre del fichero directamente del comando
            sscanf(comando + strlen("remove"), "%s", nombreFichero);
            printf("Remover\n");
            removeFile(directorio, nombreFichero);
        }else if (strcmp(orden, "copy") == 0){
            printf("Copiar\n");
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
