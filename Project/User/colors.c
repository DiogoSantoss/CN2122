#include <stdio.h>

// Pretty colors
void red() {
  printf("\033[1;31m");
}

void green() {
  printf("\033[0;32m");
}

void blue() {
  printf("\033[0;34m");
}

void purple() {
  printf("\033[0;35m");
}

void cyan() {
  printf("\033[0;36m");
}

void white() {
  printf("\033[0;37m");
}

void yellow() {
  printf("\033[1;33m");
}

void reset() {
  printf("\033[0m");
}