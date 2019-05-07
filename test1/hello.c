#include<stdio.h>

int main(int argc,char *argv[])
{
  char buf[] = "hello world";
  char array[] = "i love china,how about you";
  char buffer[] = "yes,i love china too.";
  printf_s("Jack say : %s\n",array);
  printf_s("Bob say : %s\n",buffer);
  return 0;
}
