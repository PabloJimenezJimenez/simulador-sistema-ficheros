#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "cabeceras.h"

#define LONGITUD_COMANDO 100

// Function prototypes as specified in the skeleton
void Printbytemaps(EXT_BYTE_MAPS *ext_bytemaps);
int ComprobarComando(char *strcomando, char *orden, char *argumento1, char *argumento2);
void LeeSuperBloque(EXT_SIMPLE_SUPERBLOCK *psup);
int BuscaFich(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, char *nombre);
void Directorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos);
int Renombrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, char *nombreantiguo, char *nombrenuevo);
int Imprimir(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_DATOS *memdatos, char *nombre);
int Borrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_BYTE_MAPS *ext_bytemaps,
           EXT_SIMPLE_SUPERBLOCK *ext_superblock, char *nombre, FILE *fich);
int Copiar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_BYTE_MAPS *ext_bytemaps,
           EXT_SIMPLE_SUPERBLOCK *ext_superblock, EXT_DATOS *memdatos, char *nombreorigen,
           char *nombredestino, FILE *fich);
void Grabarinodosydirectorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, FILE *fich);
void GrabarByteMaps(EXT_BYTE_MAPS *ext_bytemaps, FILE *fich);
void GrabarSuperBloque(EXT_SIMPLE_SUPERBLOCK *ext_superblock, FILE *fich);
void GrabarDatos(EXT_DATOS *memdatos, FILE *fich);

// Function implementations
void Printbytemaps(EXT_BYTE_MAPS *ext_bytemaps) {
    printf("Inodos: ");
    for (int i = 0; i < MAX_INODOS; i++) {
        printf("%d ", ext_bytemaps->bmap_inodos[i]);
    }
    printf("\n");

    printf("Bloques [0-25]: ");
    for (int i = 0; i < 26; i++) {
        printf("%d ", ext_bytemaps->bmap_bloques[i]);
    }
    printf("\n");
}

int ComprobarComando(char *strcomando, char *orden, char *argumento1, char *argumento2) {
    strcpy(orden, "");
    strcpy(argumento1, "");
    strcpy(argumento2, "");

    if (sscanf(strcomando, "%s %s %s", orden, argumento1, argumento2) < 1) {
        return -1;
    }

    return strcmp(orden, "salir") != 0;
}

void LeeSuperBloque(EXT_SIMPLE_SUPERBLOCK *psup) {
    printf("Bloque %u bytes\n", psup->s_block_size);
    printf("Inodos particion = %u\n", psup->s_inodes_count);
    printf("Inodos libres = %u\n", psup->s_free_inodes_count);
    printf("Bloques particion = %u\n", psup->s_blocks_count);
    printf("Bloques libres = %u\n", psup->s_free_blocks_count);
    printf("Primer bloque de datos = %u\n", psup->s_first_data_block);
}

int BuscaFich(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, char *nombre) {
    for (int i = 0; i < MAX_FICHEROS; i++) {
        if (strcmp(directorio[i].dir_nfich, nombre) == 0) {
            return i;
        }
    }
    return -1;
}

void Directorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos) {
    for (int i = 1; i < MAX_FICHEROS; i++) {
        if (directorio[i].dir_inodo != NULL_INODO) {
            EXT_SIMPLE_INODE *inodo = &inodos->blq_inodos[directorio[i].dir_inodo];
            printf("%s tamaño:%u  inodo:%d bloques:",
                   directorio[i].dir_nfich,
                   inodo->size_fichero,
                   directorio[i].dir_inodo);

            for (int j = 0; j < MAX_NUMS_BLOQUE_INODO; j++) {
                if (inodo->i_nbloque[j] != NULL_BLOQUE) {
                    printf(" %d", inodo->i_nbloque[j]);
                }
            }
            printf("\n");
        }
    }
}

int Renombrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, char *nombreantiguo, char *nombrenuevo) {
    int pos_origen = BuscaFich(directorio, inodos, nombreantiguo);
    if (pos_origen < 0) {
        printf("Error: Fichero %s no encontrado\n", nombreantiguo);
        return -1;
    }

    if (BuscaFich(directorio, inodos, nombrenuevo) >= 0) {
        printf("Error: El fichero %s ya existe\n", nombrenuevo);
        return -1;
    }

    strncpy(directorio[pos_origen].dir_nfich, nombrenuevo, LEN_NFICH);
    return 0;
}

int Imprimir(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_DATOS *memdatos, char *nombre) {
    int pos = BuscaFich(directorio, inodos, nombre);
    if (pos < 0) {
        printf("Error: Fichero %s no encontrado\n", nombre);
        return -1;
    }

    EXT_SIMPLE_INODE *inodo = &inodos->blq_inodos[directorio[pos].dir_inodo];
    for (int i = 0; i < MAX_NUMS_BLOQUE_INODO && inodo->i_nbloque[i] != NULL_BLOQUE; i++) {
        printf("%s", memdatos[inodo->i_nbloque[i] - PRIM_BLOQUE_DATOS].dato);
    }
    printf("\n");
    return 0;
}
int Borrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos,
           EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock,
           char *nombre, FILE *fich) {
    // Buscar el fichero en el directorio
    int pos = -1;
    for (int i = 0; i < MAX_FICHEROS; i++) {
        if (strcmp(directorio[i].dir_nfich, nombre) == 0) {
            pos = i;
            break;
        }
    }

    if (pos == -1) {
        printf("ERROR: Fichero %s no encontrado\n", nombre);
        return -1;
    }

    // Obtener el inodo del fichero
    unsigned short inodo_num = directorio[pos].dir_inodo;
    EXT_SIMPLE_INODE *inodo = &inodos->blq_inodos[inodo_num];

    // Liberar los bloques del fichero
    for (int i = 0; i < MAX_NUMS_BLOQUE_INODO; i++) {
        if (inodo->i_nbloque[i] != NULL_BLOQUE) {
            ext_bytemaps->bmap_bloques[inodo->i_nbloque[i]] = 0;
            inodo->i_nbloque[i] = NULL_BLOQUE;
            ext_superblock->s_free_blocks_count++;
        }
    }

    // Liberar el inodo
    ext_bytemaps->bmap_inodos[inodo_num] = 0;
    ext_superblock->s_free_inodes_count++;
    inodo->size_fichero = 0;

    // Liberar la entrada de directorio
    directorio[pos].dir_inodo = NULL_INODO;
    memset(directorio[pos].dir_nfich, 0, LEN_NFICH);

    return 0;
}

int Copiar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_BYTE_MAPS *ext_bytemaps,
           EXT_SIMPLE_SUPERBLOCK *ext_superblock, EXT_DATOS *memdatos,
           char *nombreorigen, char *nombredestino, FILE *fich) {
    // 1. Verificar que existe el fichero origen
    int pos_origen = -1;
    for (int i = 0; i < MAX_FICHEROS; i++) {
        if (strcmp(directorio[i].dir_nfich, nombreorigen) == 0) {
            pos_origen = i;
            break;
        }
    }

    if (pos_origen == -1) {
        printf("ERROR: Fichero origen %s no encontrado\n", nombreorigen);
        return -1;
    }

    // 2. Verificar que no existe el fichero destino
    for (int i = 0; i < MAX_FICHEROS; i++) {
        if (strcmp(directorio[i].dir_nfich, nombredestino) == 0) {
            printf("ERROR: El fichero destino %s ya existe\n", nombredestino);
            return -1;
        }
    }

    // 3. Buscar un inodo libre
    int nuevo_inodo = -1;
    for (int i = 3; i < MAX_INODOS; i++) {  // Empezamos en 3 porque 0,1,2 están reservados
        if (ext_bytemaps->bmap_inodos[i] == 0) {
            nuevo_inodo = i;
            break;
        }
    }

    if (nuevo_inodo == -1) {
        printf("ERROR: No hay inodos libres\n");
        return -1;
    }

    // 4. Buscar entrada libre en el directorio
    int pos_destino = -1;
    for (int i = 0; i < MAX_FICHEROS; i++) {
        if (directorio[i].dir_inodo == NULL_INODO) {
            pos_destino = i;
            break;
        }
    }

    if (pos_destino == -1) {
        printf("ERROR: No hay entradas libres en el directorio\n");
        return -1;
    }

    // 5. Copiar el contenido del fichero
    EXT_SIMPLE_INODE *inodo_origen = &inodos->blq_inodos[directorio[pos_origen].dir_inodo];
    EXT_SIMPLE_INODE *inodo_destino = &inodos->blq_inodos[nuevo_inodo];

    // Copiar tamaño del fichero
    inodo_destino->size_fichero = inodo_origen->size_fichero;

    // Copiar bloques
    for (int i = 0; i < MAX_NUMS_BLOQUE_INODO; i++) {
        if (inodo_origen->i_nbloque[i] == NULL_BLOQUE) {
            inodo_destino->i_nbloque[i] = NULL_BLOQUE;
            break;
        }

        // Buscar un bloque libre
        int nuevo_bloque = -1;
        for (int j = PRIM_BLOQUE_DATOS; j < MAX_BLOQUES_PARTICION; j++) {
            if (ext_bytemaps->bmap_bloques[j] == 0) {
                nuevo_bloque = j;
                break;
            }
        }

        if (nuevo_bloque == -1) {
            printf("ERROR: No hay bloques libres\n");
            // Limpieza de los bloques ya asignados
            for (int k = 0; k < i; k++) {
                ext_bytemaps->bmap_bloques[inodo_destino->i_nbloque[k]] = 0;
                ext_superblock->s_free_blocks_count++;
            }
            ext_bytemaps->bmap_inodos[nuevo_inodo] = 0;
            ext_superblock->s_free_inodes_count++;
            return -1;
        }

        // Copiar el contenido del bloque
        inodo_destino->i_nbloque[i] = nuevo_bloque;
        ext_bytemaps->bmap_bloques[nuevo_bloque] = 1;
        ext_superblock->s_free_blocks_count--;
        memcpy(&memdatos[nuevo_bloque - PRIM_BLOQUE_DATOS],
               &memdatos[inodo_origen->i_nbloque[i] - PRIM_BLOQUE_DATOS],
               SIZE_BLOQUE);
    }

    // 6. Actualizar estructuras
    ext_bytemaps->bmap_inodos[nuevo_inodo] = 1;
    ext_superblock->s_free_inodes_count--;

    // Actualizar directorio
    strcpy(directorio[pos_destino].dir_nfich, nombredestino);
    directorio[pos_destino].dir_inodo = nuevo_inodo;

    return 0;
}

// Main implementation
int main() {
    char comando[LONGITUD_COMANDO];
    char orden[LONGITUD_COMANDO];
    char argumento1[LONGITUD_COMANDO];
    char argumento2[LONGITUD_COMANDO];

    EXT_SIMPLE_SUPERBLOCK ext_superblock;
    EXT_BYTE_MAPS ext_bytemaps;
    EXT_BLQ_INODOS ext_blq_inodos;
    EXT_ENTRADA_DIR directorio[MAX_FICHEROS];
    EXT_DATOS memdatos[MAX_BLOQUES_DATOS];
    EXT_DATOS datosfich[MAX_BLOQUES_PARTICION];
    int grabardatos = 0;
    FILE *fent;

    // Inicialización del sistema
    fent = fopen("particion.bin", "r+b");
    if (fent == NULL) {
        printf("Error al abrir particion.bin\n");
        return 1;
    }

    fread(&datosfich, SIZE_BLOQUE, MAX_BLOQUES_PARTICION, fent);

    memcpy(&ext_superblock, &datosfich[0], SIZE_BLOQUE);
    memcpy(&ext_bytemaps, &datosfich[1], SIZE_BLOQUE);
    memcpy(&ext_blq_inodos, &datosfich[2], SIZE_BLOQUE);
    memcpy(&directorio, &datosfich[3], SIZE_BLOQUE);
    memcpy(&memdatos, &datosfich[4], MAX_BLOQUES_DATOS * SIZE_BLOQUE);

    // Bucle principal de comandos
    while (1) {
        printf(">> ");
        fgets(comando, LONGITUD_COMANDO, stdin);

        if (ComprobarComando(comando, orden, argumento1, argumento2) == 0) {
            GrabarDatos(memdatos, fent);
            fclose(fent);
            return 0;
        }

        if (strcmp(orden, "info") == 0) {
            LeeSuperBloque(&ext_superblock);
        } else if (strcmp(orden, "bytemaps") == 0) {
            Printbytemaps(&ext_bytemaps);
        } else if (strcmp(orden, "dir") == 0) {
            Directorio(directorio, &ext_blq_inodos);
        } else if (strcmp(orden, "rename") == 0) {
            if (Renombrar(directorio, &ext_blq_inodos, argumento1, argumento2) == 0) {
                Grabarinodosydirectorio(directorio, &ext_blq_inodos, fent);
            }
        } else if (strcmp(orden, "imprimir") == 0) {
            Imprimir(directorio, &ext_blq_inodos, memdatos, argumento1);
        }else if (strcmp(orden, "remove") == 0) {
            if (Borrar(directorio, &ext_blq_inodos, &ext_bytemaps, &ext_superblock, argumento1, fent) == 0) {
                Grabarinodosydirectorio(directorio, &ext_blq_inodos, fent);
                GrabarByteMaps(&ext_bytemaps, fent);
                GrabarSuperBloque(&ext_superblock, fent);
            }
        }else if (strcmp(orden, "copiar") == 0) {
            if (Copiar(directorio, &ext_blq_inodos, &ext_bytemaps, &ext_superblock, memdatos, argumento1, argumento2, fent) == 0) {
                Grabarinodosydirectorio(directorio, &ext_blq_inodos, fent);
                GrabarByteMaps(&ext_bytemaps, fent);
                GrabarSuperBloque(&ext_superblock, fent);
            }
        }
        else {
            printf("ERROR: Comando ilegal [info, bytemaps, dir, rename, imprimir, salir]\n");
        }
    }

    return 0;
}

// Funciones de grabación
void Grabarinodosydirectorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, FILE *fich) {
    fseek(fich, 2 * SIZE_BLOQUE, SEEK_SET);
    fwrite(inodos, SIZE_BLOQUE, 1, fich);
    fwrite(directorio, SIZE_BLOQUE, 1, fich);
}

void GrabarByteMaps(EXT_BYTE_MAPS *ext_bytemaps, FILE *fich) {
    fseek(fich, SIZE_BLOQUE, SEEK_SET);
    fwrite(ext_bytemaps, SIZE_BLOQUE, 1, fich);
}

void GrabarSuperBloque(EXT_SIMPLE_SUPERBLOCK *ext_superblock, FILE *fich) {
    fseek(fich, 0, SEEK_SET);
    fwrite(ext_superblock, SIZE_BLOQUE, 1, fich);
}

void GrabarDatos(EXT_DATOS *memdatos, FILE *fich) {
    fseek(fich, 4 * SIZE_BLOQUE, SEEK_SET);
    fwrite(memdatos, SIZE_BLOQUE, MAX_BLOQUES_DATOS, fich);
}