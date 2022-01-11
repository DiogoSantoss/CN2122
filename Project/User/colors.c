#include <stdio.h>

// Used to color the stdout

void colorRed() {
  printf("\033[1;31m");
}

void colorGreen() {
  printf("\033[0;32m");
}

void colorBlue() {
  printf("\033[0;34m");
}

void colorPurple() {
  printf("\033[0;35m");
}

void colorCyan() {
  printf("\033[0;36m");
}

void colorWhite() {
  printf("\033[0;37m");
}

void colorYellow() {
  printf("\033[1;33m");
}

void colorReset() {
  printf("\033[0m");
}