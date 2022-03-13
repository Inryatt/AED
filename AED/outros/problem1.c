#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#define ARRAY_SIZE(x) (sizeof x / sizeof x[0])
int AsRepeated( char *str, int position){ // este metodo retorna 0 caso encontre dois numeros ou duas chars iguais
  for( int i = position; i >= 0 ; i--) // dois for para correr para cada char os restantes chars
    for( int j = i - 1; j >= 0; j--) // eu fiz o j
      if(str[i] == str[j]){
        return 0;
      }
  return 1;
}
char *integerComp(char *n, int position,int nAppend,int size){
  int temp ;
  do{ // while loop para "ramificar" os casos para a posicao um gera um caso que comeca para cada numero
    n[position] = nAppend;
    if(atol(n) % (position + 1) == 0 && AsRepeated(n,position)){
      if(position == (size - 1)) printf("  %s\n",n);
      return integerComp(n,++position,48,size);
    }
    else{
      ++nAppend;
    }
  }while(nAppend <58);
  if(n[position - 1 ] + 1 == 58){ // caso o numero fique  XXXXX99 se nao tiver isto vai estar sempre a repetir XXXXXX90 XXXXX91 ..... por ai fora
    temp =(int) n[position - 2] + 1;
    if(atoi(n) == 99){
      return NULL;
      }
    n[position] = 'a';
    n[position - 1] = 'a';
    position = position -2 ;
    return integerComp(n,position,temp,size);
  }
  temp =(int) n[position - 1] + 1; // este penso que seja caso seja XXXX89 passa para XXXX9
  n[position] = 'a';
  return integerComp(n,--position,temp,size);

}
int main(int argc, char **argv){
  //conditions 10 numbers (0-9) the first one can only be 0 cause only 1 divides 1
  // no repeated integers
  // the number until the position k(k=1,2,3,4,..,10) must be divided by k    12 can be the first two integers on the number but 13 can't
  int maxChar = 10;
  char *number = (char *)malloc((size_t) (maxChar + 1) * sizeof(char));
  printf("O seguinte numero respeitas as condicoes:");
  integerComp(number,0,49,maxChar);
  free(number);
  return 0;
}
