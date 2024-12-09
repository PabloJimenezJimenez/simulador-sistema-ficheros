#include <stdio.h>
#include <string.h>
#include "cabeceras.h" // Incluimos el archivo de cabeceras proporcionado

// Implementación de ComprobarComando
int ComprobarComando(char *strcomando, char *orden, char *argumento1, char *argumento2) {
    // Extrae el comando, argumento1 y argumento2 de la cadena `strcomando`
    sscanf(strcomando, "%s %s %s", orden, argumento1, argumento2);
    return strcmp(orden, "salir") != 0; // Retorna 0 si el comando es "salir"
}

// Implementación de Directorio
void Directorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos) {
    printf("Lista de archivos en el directorio:\n");
    for (int i = 0; i < MAX_FICHEROS; i++) {
        if (directorio[i].dir_inodo != NULL_INODO) {
            printf("Archivo: %s, Inodo: %d\n", directorio[i].dir_nfich, directorio[i].dir_inodo);
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
    printf("Bloque 512 Bytes\n");
    printf("Inodos particion = %u\n", superbloque->s_inodes_count);
    printf("Inodos libres = %u\n", superbloque->s_free_inodes_count);
    printf("Bloques particion = %u\n", superbloque->s_blocks_count);
    printf("Bloques libres = %u\n", superbloque->s_free_blocks_count);
    printf("Primer bloque de datos = %u\n", superbloque->s_first_data_block);
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
        perror("Error al abrir el archivo particion.bin");
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
