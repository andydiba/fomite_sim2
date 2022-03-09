#ifndef UTILITIES_H
#define UTILITIES_H

#include <stdint.h>
#include <stdbool.h>


#define RED "\x1b[0;31m"
#define RED_B "\x1b[1;31m"
#define GREEN "\x1b[0;32m"
#define GREEN_B "\x1b[1;32m"
#define BLUE "\x1b[0;34m"
#define BLUE_B "\x1b[1;34m"
#define CYAN "\x1b[0;36m"
#define CYAN_B "\x1b[1;36m"
#define MAGENTA "\x1b[0;35m"
#define MAGENTA_B "\x1b[1;35m"
#define YELLOW "\x1b[0;33m"
#define YELLOW_B "\x1b[1;33m"

#define RESET "\033[0m"

//Prints an error message
bool errorMsg(char* msg);
void warningMsg(char* msg);

void updateMsg(char* msg);


#endif
