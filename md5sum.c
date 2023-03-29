#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#define BUFF_LEN    256
#define SLAVES_QTY  4


//Funciones usadas dentro de este archivo
void create_slave_processes(char ** paths, int paths_qty);
void print_error_msg(char* str);


int main(int argc, char * argv[]){
    //Verificamos que la cantidad de argumentos sea mayor a 1
    if(argc <= 1){
        char errmsg[] = "Invalid arguments quantity\n";
        print_error_msg(errmsg);
    }
    char ** paths;
    int file_qty;
    //Asignamos cada uno de los path al array de strings "paths" para pasárselo como argumento a la función create_slave_processes
    for(file_qty = 1 ; file_qty <= argc - 1 ; file_qty++){
        paths[file_qty - 1] = argv[file_qty];
    }
    //Llamamos a la funcion que crea los procesos esclavos
    create_slave_processes(paths,file_qty);
}

void create_slave_processes(char ** paths,int paths_qty){
    int aux;
    for(aux = 0 ; aux < paths_qty ; aux++){
        fork();
    }
}

void print_error_msg(char * str){
    perror(str);
    exit(0);
}