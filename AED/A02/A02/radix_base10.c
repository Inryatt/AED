#include "sorting_methods.h"
#include <stdlib.h>
#include <stdio.h>

int findLargestNum(T * arr, int size){
  
  int i;
  int largestNum = -1;
  
  for(i = 0; i < size; i++){
    if(arr[i] > largestNum)
      largestNum = arr[i];
  }
  
  return largestNum;
}

void order(T * array, int size){
  // Base 10 is used
  int i;
  T *semiSorted = (T *)malloc(size * sizeof(T));
  long int significantDigit = 1;
  int largestNum = findLargestNum(array, size);
  
  // Loop until we reach the largest significant digit
  while (largestNum / significantDigit > 0){
    
    int bucket[10] = { 0 };
    
    // Counts the number of "keys" or digits that will go into each bucket
    for (i = 0; i < size; i++)
      bucket[(array[i] / significantDigit) % 10]++;
    
    /**
     * Add the count of the previous buckets,
     * Acquires the indexes after the end of each bucket location in the array
		 * Works similar to the count sort algorithm
     **/
    for (i = 1; i < 10; i++)
      bucket[i] += bucket[i - 1];
    
    // Use the bucket to fill a "semiSorted" array
    for (i = size - 1; i >= 0; i--)
      semiSorted[--bucket[(array[i] / significantDigit) % 10]] = array[i];
    
    
    for (i = 0; i < size; i++)
      array[i] = semiSorted[i];
    
    // Move to next significant digit
    significantDigit *= 10;
    
  }
}


// Radix Sort
void radix_base10 (T * data, int first, int one_after_last){  
  int nposi = 0;
  int nneg = 0;
  T * posi= (T *)malloc((one_after_last - first) * sizeof(T));
  T * neg = (T *)malloc((one_after_last - first) * sizeof(T));

  //copy arrays
  for(int f = first ; f < one_after_last ; f++){
    if(data[f] >= 0){
      posi[nposi]= data[f];
      nposi++;
    }else{
      neg[nneg] = abs(data[f]);
      nneg++;
    }
  }
  
  nposi--;
  nneg--;

  if(nneg > 0){
    //Sort negatives
    order(neg, nneg + 1);
    
    //Paste negatives
    for(int k = nneg ; k >= 0 ; k--){
      data[first + nneg - k] = neg[k]*-1;
    }
  }
  
  //Free memory of negatives array
  free(neg); 

  if(nposi > 0){
    //Sort positives
    order(posi, nposi + 1);
    
    //Paste positives
    for(int k = 0; k <= nposi; k++){
      data[first + nneg + k + 1] = posi[k];
    }
  }

  //Free memory of positives array
  free(posi);
}

  
  

  
