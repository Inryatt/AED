#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
 
bool is_sorted(int *a, int n)
{
  while ( --n >= 1 ) {
    if ( a[n] < a[n-1] ) return false;
  }
  return true;
}
 
void shuffle(int *a, int n)
{
  int i, t, r;
  for(i=0; i < n; i++) {
    t = a[i];
    r = rand() % n;
    a[i] = a[r];
    a[r] = t;
  }
}
 
void bogosort(int *a, int n)
{
  while ( !is_sorted(a, n) ){


   shuffle(a, n);
   for (int i=0;i<n;i++){
       printf("%d ",a[i]);
   }
   printf("\n");
  };
}
 
 #define N_ELEMENTS 7
int main()
{

  int numbers[] = {6,5,4,3,2,1,0};
  int i;
  srand(time(0)) ;
  bogosort(numbers, N_ELEMENTS);
  for (i=0; i < N_ELEMENTS; i++) printf("%d ", numbers[i]);
  printf("\n");
}