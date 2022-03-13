//
// AED, 2020/2021
//
// Decoding a non-instantaneous binary code
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//
// Compile time parameters
//

#ifndef MAX_N_SYMBOLS
# define MAX_N_SYMBOLS         1000 // maximum number of alphabet symbols in a code
#endif
#ifndef MAX_CODEWORD_SIZE
# define MAX_CODEWORD_SIZE      23 // maximum number of bits of a codeword
#endif
#ifndef MAX_MESSAGE_SIZE
# define MAX_MESSAGE_SIZE   100000 // maximum number of symbols in a message
#endif

#ifndef N_OUTLIERS
# define N_OUTLIERS             20 // discard this number of mesurements (outliers) on each side of the median
#endif
#ifndef N_VALID
# define N_VALID                80 // use this number of measurements on each side of the median
#endif
#define N_MEASUREMENTS  (2 * N_OUTLIERS + 2 * N_VALID + 1)  // total number of measurements


//
// Random number generator interface
//
// In order to ensure reproducible results on Windows and GNU/Linux, we use a good random number generator, available at
//   https://www-cs-faculty.stanford.edu/~knuth/programs/rng.c
// This file has to be used without any modifications, so we take care of the main function that is there by applying
// some C preprocessor tricks
//
// DO NOT CHANGE THIS CODE
//

#define main  rng_main                        // main gets replaced by rng_main
#ifdef __GNUC__
int rng_main() __attribute__((__unused__));   // gcc will not complain if rnd_main() is not used
#endif
#include "rng.c"
#undef main                                   // main becomes main again

#define srandom(seed)  ran_start((long)seed)  // start the pseudo-random number generator
#define random()       ran_arr_next()         // get the next pseudo-random number (0 to 2^30-1)


//
// Generation of a random non-instantaneous uniquely decodable code with n symbols (inverted Hufffman code)
//
// DO NOT CHANGE THIS CODE
//

typedef struct
{
  int scaled_prob;                      // proportional to the probability of occurrence of this symbol
  int cum_scaled_prob;                  // proportional to the probability of occurrence of this or of all previous symbols
  int parent;                           // -1 means no parent, >= 0 gives the index of the parent
  int bit;                              // -1 means no information, 0 or 1 means append this bit to the parent's code
  char codeword[MAX_CODEWORD_SIZE + 1]; // the complete (inverted) Huffman code
}
symbol_t;

typedef struct
{
  int n_symbols;    // the number of symbols
  int max_bits;     // maximum number of bits of a codeword
  symbol_t *data;   // the symbols and their codes (with extra data at the end --- used to construct the entire Huffman tree)
}
code_t;

void free_code(code_t *c)
{
  if(c != NULL)
  {
    if(c->data != NULL)
      free(c->data);
    c->data = NULL;
    free(c);
  }
}

code_t *new_code(int n_symbols)
{
  int i,i0,i1,n;
  code_t *c;

  //
  // Refuse to handle too few or too many symbols
  //
  if(n_symbols < 2 || n_symbols > MAX_N_SYMBOLS)
  {
    fprintf(stderr,"new_code: n_symbols (%d) is either too small or too large\n",n_symbols);
    exit(1);
  }
  //
  // Allocate memory for the n_symbols symbols plus n_symbols-1 tree nodes for the Huffman tree
  //
  c = (code_t *)malloc(sizeof(code_t));
  if(c == NULL)
  {
    fprintf(stderr,"new_code: out of memory\n");
    exit(1);
  }
  c->data = (symbol_t *)malloc((size_t)(2 * n_symbols - 1) * sizeof(symbol_t));
  if(c->data == NULL)
  {
    free(c);
    fprintf(stderr,"new_code: out of memory\n");
    exit(1);
  }
  //
  // Initialize the symbols --- at the beginning, the symbols (leaves of the Huffman tree) are disconnected
  //
  c->n_symbols = n_symbols;
  for(i = 0;i < n_symbols;i++)
  {
    c->data[i].scaled_prob = 10 + (int)random() % 991;              // a pseudo-random integer belonging to the interval [10,1000]
    c->data[i].cum_scaled_prob = c->data[i].scaled_prob;            // used only to generate
    if(i > 0)                                                       //   symbols with the
      c->data[i].cum_scaled_prob += c->data[i - 1].cum_scaled_prob; //   correct probability
    c->data[i].parent = -1;                                         // currently, no parent node
    c->data[i].bit = -1;                                            // currently, no bit numbier
    c->data[i].codeword[0] = '\0';                                  // currently, no codeword string
  }
  //
  // Construct the Huffman code
  //
  // We are going to do it in a O(n^2) way --- speed is not important here
  // Using min-heaps would reduce that to O(n log n), but the code would be longer and more difficult to understand
  //
  n = n_symbols;
  for(;;)
  {
    //
    // Find the two "open" nodes (those with a parent equal to -1) with the smallest scaled_prob
    //
    i0 = i1 = -1;
    for(i = 0;i < n;i++)
      if(c->data[i].parent == -1)
      { // ok, we have an open node
        if(i0 < 0 || c->data[i].scaled_prob < c->data[i0].scaled_prob)
        { // the smallest scaled_prob so far
          i1 = i0;
          i0 = i;
        }
        else if(i1 < 0 || c->data[i].scaled_prob < c->data[i1].scaled_prob)
        { // the second smallest scaled_prob so far
          i1 = i;
        }
      }
    //
    // Are we done? Yes when we cannot find two open nodes (this will happen when n == 2 * n_symbols - 1)
    //
    if(i1 < 0)
      break;
    //
    // Merge the two open nodes (close them and create a new open node)
    //
    c->data[n].scaled_prob = c->data[i0].scaled_prob + c->data[i1].scaled_prob;
    c->data[n].cum_scaled_prob = -1; // not used but we initialize it anyway
    c->data[n].parent = -1;
    c->data[n].bit = -1;
    c->data[n].codeword[0] = '\0';   // not used but we initialize it anyway
    c->data[i0].parent = n;          // the parent of node i0 becomes node n --- the left descendant of node n is node i0 (we do not record this information)
    c->data[i0].bit = 0;             // give this branch a bit of 0
    c->data[i1].parent = n;          // the parent of node i1 becomes node n --- the right descendant of node n is node i1 (we do not record this information)
    c->data[i1].bit = 1;             // give this branch a bit of 1
    n++;
  }
  if(n != 2 * n_symbols - 1)
  {
    fprintf(stderr,"new_code: unexpected value of n [expected %d, we got %d]\n",2 * n_symbols - 1,n);
    exit(1);
  }
  //
  // For each symbol, initialize its (inverted) Huffman code
  //
  c->max_bits = 0;
  for(n = 0;n < n_symbols;n++)
  {
    i = 0;  // the current code size
    i0 = n; // the initial tree node index
    while(c->data[i0].parent >= 0)
    {
      if(i >= MAX_CODEWORD_SIZE)
      {
        fprintf(stderr,"ne_code: MAX_CODEWORD_SIZE is too small\n");
        exit(1);
      }
      c->data[n].codeword[i] = '0' + c->data[i0].bit;
      i++;
      i0 = c->data[i0].parent;
    }
    c->data[n].codeword[i] = '\0'; // terminate the codeword string
    if(i > c->max_bits)
      c->max_bits = i;
  }
  //
  // Done!
  //
  return c;
}


//
// Random code symbol
//
// DO NOT CHANGE THIS CODE
//

int random_symbol(code_t *c)
{
  int i,r;

  //
  // Generate an (approximately) uniformly distributed integer in the appropriate range
  //
  r = random() % c->data[c->n_symbols - 1].cum_scaled_prob;
  //
  // Find the index i for which c->data[i - 1].cum_scaled_prob <= r < c->data[i].cum_scaled_prob (with c->data[-1].cum_scaled_prob implicitly 0)
  // We are going to do it in a O(n) way --- speed is not important here
  // We could have used a special version of binary search here, but the code would be longer and more difficult to understand
  //
  for(i = 0;i < c->n_symbols;i++)
    if(r < c->data[i].cum_scaled_prob)
      break;
  if(i == c->n_symbols)
  {
    fprintf(stderr,"ramdom_symbol: i is too large! Impossible!!! [r=%d]\n",r);
    exit(1);
  }
  return i;
}


//
// Random message
//
// DO NOT CHANGE THIS CODE
//

void random_message(code_t *c,int message_size,int message[message_size])
{
  int i;

  if(message_size < 1 || message_size > MAX_MESSAGE_SIZE)
  {
    fprintf(stderr,"random_message: bad message size (%d)\n",message_size);
    exit(1);
  }
  for(i = 0;i < message_size;i++)
    message[i] = random_symbol(c);
}


//
// Encode a message
//
// DO NOT CHANGE THIS CODE
//

void encode_message(code_t *c,int message_size,int message[message_size],int max_encoded_message_size,char encoded_message[max_encoded_message_size + 1])
{
  int i,j,n;
  char *s;

  if(message_size < 1 || message_size > MAX_MESSAGE_SIZE)
  {
    fprintf(stderr,"encode_message: bad message size (%d)\n",message_size);
    exit(1);
  }
  n = 0; // encoded message size
  for(i = 0;i < message_size;i++)
  {
    if(message[i] < 0 || message[i] >= c->n_symbols)
    {
      fprintf(stderr,"encoded_message: unexpected symbol (%d)\n",message[i]);
      exit(1);
    }
    s = c->data[message[i]].codeword;
    for(j = 0;s[j] != 0;j++)
    {
      if(n > max_encoded_message_size)
      {
        fprintf(stderr,"encode_message: the encoded message is too big\n");
        exit(1);
      }
      encoded_message[n++] = s[j]; // concatenate the code word
    }
  }
  encoded_message[n] = '\0'; // terminate the string
}


//
// Global data used for decoding (to avoid passing all this information in function arguments, thus making the program more efficient)
//

struct
{
  code_t * c;                        // the code being used
  int    * original_message;         // the original message
  int      original_message_size;    // the original message length
  int      max_encoded_message_size; // the largest possible encoded message size
  char   * encoded_message;          // the encoded message
  int      max_decoded_message_size; // the largest possible decoded message length
  int    * decoded_message;          // the decoded message (should be equal to the original message)
  long     number_of_calls;          // the number of recursive function calls
  long     number_of_solutions;      // the number of solutions (at the end, is all is well, must be equal to 1)
  int      max_extra_symbols;        // the largest difference between the partially decoded message and the good part of the partially decoded message)
  //int    * sorted;                   // array of the indexes sorted by their value length  
}
decoder_global_data;

#define _c_                         decoder_global_data.c
#define _original_message_          decoder_global_data.original_message
#define _original_message_size_     decoder_global_data.original_message_size
#define _max_encoded_message_size_  decoder_global_data.max_encoded_message_size
#define _encoded_message_           decoder_global_data.encoded_message
#define _max_decoded_message_size_  decoder_global_data.max_decoded_message_size
#define _decoded_message_           decoder_global_data.decoded_message
#define _number_of_calls_           decoder_global_data.number_of_calls
#define _number_of_solutions_       decoder_global_data.number_of_solutions
#define _max_extra_symbols_         decoder_global_data.max_extra_symbols
//#define sorted                      decoder_global_data.sorted

//
// Recursive decoder
//
// encoded_idx ......... index into the _encoded_message_ array of the next bit to be considered
// decoded_idx ......... index into the _decoded_message_ array where the next decoded symbol will be placed
// good_decoded_size ... number of correct decoded symbols
//
// Decoding large messages require a large amount of stack space (one recursion level per message symbol)
// If you get a segmentation fault in our program you may need to increase the stack size (under GNU/linux, you can do it using the command "ulimit -s 16384")
//

// Code 1.0
#if 0
static void recursive_decoder(int encoded_idx,int decoded_idx,int good_decoded_size)
{                                                                                                                             //
  _number_of_calls_ ++;                                                                                                       // incremento do numero de calls da recurse 
  int retVal = 0;                                                                                                             // bool da existencia de uma codificação para o que se tem  

  // Caso Terminal
  if((decoded_idx == _original_message_size_)){                                                                               //
    if(encoded_idx == strlen(_encoded_message_))                                                                              // 
      _number_of_solutions_ ++;                                                                                               // encremento do numero de soluções
    return;                                                                                                                   //
  }                                                                                                                           //
  char val[_c_->max_bits + 1];                                                                                                // iniciação da variável k irá ter a codificação a se avaliar
  for(int j = 1 ; j <= _c_->max_bits && (j+encoded_idx) <= strlen(_encoded_message_); j++){                                   // itera sobre a codificação da mensagem desde onde estamos até ao máximo k uma codificação pode ter de bits 
    strncpy(val, _encoded_message_ + encoded_idx, j);                                                                         // copia substring a avaliar se é uma codificação
    val[j] = '\0';                                                                                                            // por terminador nulo no fim para se ter uma string                                               
    for(int i = 0 ; i < _c_->n_symbols ; i++){                                                                                // itera sobre as codificações
      if(strcmp(_c_->data[i].codeword, val)==0){                                                                              // encontra-se uma codificação possivel
        retVal = 1;                                                                                                           // altera-se o bool k representa a existencia de uma codificação possivel
        encoded_idx += j;                                                                                                     // incrementar o numero de bits a codificação teve
        _decoded_message_[decoded_idx] = i;                                                                                   // assumir o simbolo encontrado como certo
        if(_original_message_[decoded_idx] == i)                                                                              // verificar se o decoded simbolo é o certp
          good_decoded_size ++;                                                                                               // encrementar numero certo de decoded simbols se for o certo
        decoded_idx ++;                                                                                                       // incrementar o decoded_idx
        if((decoded_idx - good_decoded_size) > _max_extra_symbols_)                                                           // caso o numero de simbolos extras nesta iteração seja maior ao max até agr
          _max_extra_symbols_ = decoded_idx - good_decoded_size;                                                              // define-se o max até agr de simbolos extras como o obtido agr
        recursive_decoder(encoded_idx,decoded_idx,good_decoded_size);                                                         // aceitar como a codificação encontrada nesta iteração como certa e ir para o próximo
        encoded_idx -= j;                                                                                                     // decrementar o numero de bits k a codificação encontrada teve
        decoded_idx --;                                                                                                       // decrementar o decoded_idx
      }                                                                                                                       //
    }                                                                                                                         //
  }                                                                                                                           //

  //Dead End
  if(retVal==0)                                                                                                               // atingiu um dead end
   return;                                                                                                                    //
}                                                                                                                             //
  
#endif

// Code 2.0
#if 0
// Função de decodificação
static void recursive_decoder(int encoded_idx,int decoded_idx,int good_decoded_size)
{                                                                                                                             //
  _number_of_calls_ ++;                                                                                                       // incremento do numero de calls da recurse 
  int retVal = 0;                                                                                                             // bool da existencia de uma codificação para o que se tem  
  char val[_c_->max_bits + 1];                                                                                                // iniciação da variável k irá ter a codificação a se avaliar
  static int min_size;                                                                                                        // iniciação da variável k representa o tamanho minimo das codificações

  // Definir minimo size de codeword                    
  if(encoded_idx == 0 && decoded_idx == 0)                                                                                    // caso seja o inicio da das recursões averigua-se o tamanho
  {                                                                                                                           //
    min_size = _c_->max_bits;                                                                                                 // definir minimo como igual ao máximo size possivel dos codes                                             
    for(int i=0 ; i < _c_->n_symbols ; i++)                                                                                   // iteração pelas codificações
    {                                                                                                                         //
      if(min_size > strlen(_c_->data[i].codeword))                                                                            // verificação se existe codificação menor k a encotrada
        min_size = strlen(_c_->data[i].codeword);                                                                             //
    }                                                                                                                         //
  }                                                                                                                           //

  // Caso Terminal
  if((decoded_idx == _original_message_size_)){                                                                               // verificação se a mensagem decoded tem o mesmo tamanho k a original
    if(encoded_idx == strlen(_encoded_message_))                                                                              // verificação se foi percorrido todos os encoded bits
      _number_of_solutions_ ++;                                                                                               // encremento do numero de soluções
    return;                                                                                                                   //
  }                                                                                                                           //  

  // Tentativa de descodificação min_size by min_size
  for(int j = min_size ; j <= _c_->max_bits && (j+encoded_idx) <= strlen(_encoded_message_); j++){                            // itera sobre a codificação da mensagem desde onde estamos até ao máximo k uma codificação pode ter de bits 
    strncpy(val, _encoded_message_ + encoded_idx, j);                                                                         // copia substring a avaliar se é uma codificação
    val[j] = '\0';                                                                                                            // por terminador nulo no fim para se ter uma string                                               
    for(int i = 0 ; i <= _c_->n_symbols ; i++){                                                                               // itera sobre as codificações
      if(strcmp(_c_->data[i].codeword, val)==0){                                                                              // encontra-se uma codificação possivel
        retVal = 1;                                                                                                           // altera-se o bool k representa a existencia de uma codificação possivel
        encoded_idx += j;                                                                                                     // incrementar o numero de bits a codificação teve
        _decoded_message_[decoded_idx] = i;                                                                                   // assumir o simbolo encontrado como certo
        if(_original_message_[decoded_idx] == i)                                                                              // verificar se o decoded simbolo é o certp
          good_decoded_size ++;                                                                                               // encrementar numero certo de decoded simbols se for o certo
        decoded_idx ++;                                                                                                       // incrementar o decoded_idx
        if((decoded_idx - good_decoded_size) > _max_extra_symbols_)                                                           // caso o numero de simbolos extras nesta iteração seja maior ao max até agr
          _max_extra_symbols_ = decoded_idx - good_decoded_size;                                                              // define-se o max até agr de simbolos extras como o obtido agr
        recursive_decoder(encoded_idx,decoded_idx,good_decoded_size);                                                         // aceitar como a codificação encontrada nesta iteração como certa e ir para o próximo
        encoded_idx -= j;                                                                                                     // decrementar o numero de bits k a codificação encontrada teve
        decoded_idx --;                                                                                                       // decrementar o decoded_idx
      }                                                                                                                       //
    }                                                                                                                         //
  }                                                                                                                           //

  //Dead End
  if(retVal==0)                                                                                                               // atingiu um dead end
   return;                                                                                                                    //
}                                                                                                                             //

#endif

// Code 3.0
#if 0
// Função de decodificação
static void recursive_decoder(int encoded_idx,int decoded_idx,int good_decoded_size)
{                                                                                                                             //
  _number_of_calls_ ++;                                                                                                       // incremento do numero de calls da recurse 
  int retVal = 0;                                                                                                             // bool da existencia de uma codificação para o que se tem  
  char val[_c_->max_bits + 1];                                                                                                // iniciação da variável k irá ter a codificação a se avaliar
  static int min_size;                                                                                                        // iniciação da variável k representa o tamanho minimo das codificações
  static int *sorted;                                                                                                         // iniciação do array k repreenta os indices da matriz das codificações sorted pelo length

  // Definir minimo size de codeword e preparar sorted array por length                    
  if(encoded_idx == 0 && decoded_idx == 0)                                                                                    // caso seja o inicio da das recursões averigua-se o tamanho
  {                                                                                                                           //
    sorted =(int *) malloc(_c_->n_symbols *sizeof(int));                                                                      // alocação de memória
    char codes[_c_->n_symbols][_c_->max_bits + 1];                                                                            // criação de uma matrix de codificações para dar sort e saber os sorted indexes
    min_size = _c_->max_bits;                                                                                                 // definir minimo como igual ao máximo size possivel dos codes                                               
    for(int i=0 ; i < _c_->n_symbols ; i++)                                                                                   // iteração pelas codificações
    {                                                                                                                         //
      // Encontrar codificação mais pequena
      if(min_size > strlen(_c_->data[i].codeword))                                                                            // verificação se existe codificação menor k a encotrada
        min_size = strlen(_c_->data[i].codeword);                                                                             //
      
      // Por os indices no sorted 
      sorted[i] = i;                                                                                                          // inserção dos indices 
  
      // Copiar as codificações para dar sort
      strcpy(codes[i],_c_->data[i].codeword);                                                                                 // cópia das codificações para o array onde se irá fazer sort    
    }                                                                                                                         //    
                                                                                                              
    // Sort do array cópia em função dos sizes de codificação (Insertion sort)
    for (int i=1 ;i<_c_->n_symbols; i++)                                                                                      //
    {                                                                                                                         //
        char temp[strlen(codes[i])+1];                                                                                        //
        strcpy(temp,codes[i]);                                                                                                //
        int tempi = sorted[i];                                                                                                //  
        int j = i - 1;                                                                                                        //
        while (j >= 0 && strlen(temp) < strlen(codes[j]))                                                                     //
        {                                                                                                                     //
            strcpy(codes[j+1],codes[j]);                                                                                      //
            sorted[j+1]= sorted[j];                                                                                           //
            j--;                                                                                                              //
        }                                                                                                                     //
        strcpy(codes[j+1],temp);                                                                                              //
        sorted[j+1] = tempi;                                                                                                  //
    }                                                                                                                         //
  }                                                                                                                           //

  // Caso Terminal
  if((decoded_idx == _original_message_size_)){                                                                               // verificação se a mensagem decoded tem o mesmo tamanho k a original
    if(encoded_idx == strlen(_encoded_message_))                                                                              // verificação se foi percorrido todos os encoded bits
      _number_of_solutions_ ++;                                                                                               // encremento do numero de soluções
    return;                                                                                                                   //
  }                                                                                                                           //  

  // Tentativa de descodificação min_size by min_size
  for(int j = min_size ; j <= _c_->max_bits && (j+encoded_idx) <= strlen(_encoded_message_); j++){                            // itera sobre a codificação da mensagem desde onde estamos até ao máximo k uma codificação pode ter de bits 
    strncpy(val, _encoded_message_ + encoded_idx, j);                                                                         // copia substring a avaliar se é uma codificação
    val[j] = '\0';                                                                                                            // por terminador nulo no fim para se ter uma string                                               
    for(int i = 0 ;  strlen(_c_->data[sorted[i]].codeword) <= j && i < _c_->n_symbols ; i++){                                 // itera sobre as codificações desde a de tamanho j até parar de ser tamanho j
      if(strcmp(_c_->data[sorted[i]].codeword, val)==0){                                                                      // encontra-se uma codificação possivel
        retVal = 1;                                                                                                           // altera-se o bool k representa a existencia de uma codificação possivel
        encoded_idx += j;                                                                                                     // incrementar o numero de bits a codificação teve
        _decoded_message_[decoded_idx] = sorted[i];                                                                           // assumir o simbolo encontrado como certo
        if(_original_message_[decoded_idx] == sorted[i])                                                                      // verificar se o decoded simbolo é o certp
          good_decoded_size ++;                                                                                               // encrementar numero certo de decoded simbols se for o certo
        decoded_idx ++;                                                                                                       // incrementar o decoded_idx
        if((decoded_idx - good_decoded_size) > _max_extra_symbols_)                                                           // caso o numero de simbolos extras nesta iteração seja maior ao max até agr
          _max_extra_symbols_ = decoded_idx - good_decoded_size;                                                              // define-se o max até agr de simbolos extras como o obtido agr
        recursive_decoder(encoded_idx,decoded_idx,good_decoded_size);                                                         // aceitar como a codificação encontrada nesta iteração como certa e ir para o próximo
        encoded_idx -= j;                                                                                                     // decrementar o numero de bits k a codificação encontrada teve
        decoded_idx --;                                                                                                       // decrementar o decoded_idx
      }                                                                                                                       //
    }                                                                                                                         //
  }                                                                                                                           //

  //Dead End
  if(retVal==0)                                                                                                               // atingiu um dead end
   return;                                                                                                                    //
}                                                                                                                             //

#endif

// Code 4.0
#if 0
static void recursive_decoder(int encoded_idx,int decoded_idx,int good_decoded_size)
{                                                                                                                             //
  _number_of_calls_ ++;                                                                                                       // incremento do numero de calls da recurse 
  int retVal = 0;                                                                                                             // bool da existencia de uma codificação para o que se tem  

  // Caso Terminal
  if((decoded_idx == _original_message_size_)){                                                                               //
    if(encoded_idx == strlen(_encoded_message_))                                                                              // 
      _number_of_solutions_ ++;                                                                                               // encremento do numero de soluções
    return;
  }
  for(int j = 1 ; j <= _c_->max_bits && (j+encoded_idx) <= strlen(_encoded_message_); j++){                                   // itera sobre a codificação da mensagem desde onde estamos até ao máximo k uma codificação pode ter de bits 
    for(int i = 0 ; i < _c_->n_symbols ; i++){                                                                                // itera sobre as codificações
      char* code = _c_->data[i].codeword;                                                                                     // guarda-se a codificação a se testar
      for(int k = 0 ; k<=(j-1) && code[k]!='\0' ; k++){                                                                       // para cada bit da codificação a se comparar
        if(strlen(code) != j || code[k] != _encoded_message_[encoded_idx + k])                                                // verifica-se se o tamanho da codificação é igual à pretendida e se possui os bits iguais
          break;                                                                                                              // dá-se break para testar outra codificação caso n seja
        if(k+1==j)                                                                                                            // caso se esteja no final
          retVal = 1;                                                                                                         // altera-se o bool k representa a existencia de uma codificação possivel
      }                                                                                                                       //
      if(retVal){                                                                                                             // encontra-se uma codificação possivel
        retVal = 0;                                                                                                           // altera-se o bool k representa a existencia de uma codificação possivel
        encoded_idx += j;                                                                                                     // incrementar o numero de bits a codificação teve
        _decoded_message_[decoded_idx] = i;                                                                                   // assumir o simbolo encontrado como certo
        if(_original_message_[decoded_idx] == i)                                                                              // verificar se o decoded simbolo é o certp
          good_decoded_size ++;                                                                                               // encrementar numero certo de decoded simbols se for o certo
        decoded_idx ++;                                                                                                       // incrementar o decoded_idx
        if((decoded_idx - good_decoded_size) > _max_extra_symbols_)                                                           // caso o numero de simbolos extras nesta iteração seja maior ao max até agr
          _max_extra_symbols_ = decoded_idx - good_decoded_size;                                                              // define-se o max até agr de simbolos extras como o obtido agr
        recursive_decoder(encoded_idx,decoded_idx,good_decoded_size);                                                         // aceitar como a codificação encontrada nesta iteração como certa e ir para o próximo
        encoded_idx -= j;                                                                                                     // decrementar o numero de bits k a codificação encontrada teve
        decoded_idx --;                                                                                                       // decrementar o decoded_idx
      }                                                                                                                       //
    }                                                                                                                         //
  }                                                                                                                           //

  //Dead End
  if(retVal==0)                                                                                                               // atingiu um dead end
   return;                                                                                                                    //
}                                                                                                                             //
  
#endif

// Code 5.0
#if 0
// Função de decodificação
static void recursive_decoder(int encoded_idx,int decoded_idx,int good_decoded_size)
{                                                                                                                             //
  _number_of_calls_ ++;                                                                                                       // incremento do numero de calls da recurse 
  int retVal = 0;                                                                                                             // bool da existencia de uma codificação para o que se tem  
  static int min_size;                                                                                                        // iniciação da variável k representa o tamanho minimo das codificações

  // Definir minimo size de codeword                    
  if(encoded_idx == 0 && decoded_idx == 0)                                                                                    // caso seja o inicio da das recursões averigua-se o tamanho
  {                                                                                                                           //
    min_size = _c_->max_bits;                                                                                                 // definir minimo como igual ao máximo size possivel dos codes                                               
    for(int i=0 ; i < _c_->n_symbols ; i++)                                                                                   // iteração pelas codificações
    {                                                                                                                         //
      if(min_size > strlen(_c_->data[i].codeword))                                                                            // verificação se existe codificação menor k a encotrada
        min_size = strlen(_c_->data[i].codeword);                                                                             //
    }                                                                                                                         //
  }                                                                                                                           //

  // Caso Terminal
  if((decoded_idx == _original_message_size_)){                                                                               // verificação se a mensagem decoded tem o mesmo tamanho k a original
    if(encoded_idx == strlen(_encoded_message_))                                                                              // verificação se foi percorrido todos os encoded bits
      _number_of_solutions_ ++;                                                                                               // encremento do numero de soluções
    return;                                                                                                                   //
  }                                                                                                                           //  

  // Tentativa de descodificação min_size by min_size
  for(int j = min_size ; j <= _c_->max_bits && (j+encoded_idx) <= strlen(_encoded_message_); j++){                            // itera sobre a codificação da mensagem desde onde estamos até ao máximo k uma codificação pode ter de bits 
    for(int i = 0 ; i < _c_->n_symbols ; i++){                                                                                // itera sobre as codificações
      char* code = _c_->data[i].codeword;                                                                                     // guarda-se a codificação a se testar
      for(int k = 0 ; k<=(j-1) && code[k]!='\0' ; k++){                                                                       // para cada bit da codificação a se comparar
        if(strlen(code) != j || code[k] != _encoded_message_[encoded_idx + k])                                                // verifica-se se o tamanho da codificação é igual à pretendida e se possui os bits iguais
          break;                                                                                                              // dá-se break para testar outra codificação caso n seja
        if(k+1==j)                                                                                                            // caso se esteja no final
          retVal = 1;                                                                                                         // altera-se o bool k representa a existencia de uma codificação possivel
      }                                                                                                                       //
      if(retVal){                                                                                                             // encontra-se uma codificação possivel
        retVal = 0;                                                                                                           // altera-se o bool k representa a existencia de uma codificação possivel
        encoded_idx += j;                                                                                                     // incrementar o numero de bits a codificação teve
        _decoded_message_[decoded_idx] = i;                                                                                   // assumir o simbolo encontrado como certo
        if(_original_message_[decoded_idx] == i)                                                                              // verificar se o decoded simbolo é o certp
          good_decoded_size ++;                                                                                               // encrementar numero certo de decoded simbols se for o certo
        decoded_idx ++;                                                                                                       // incrementar o decoded_idx
        if((decoded_idx - good_decoded_size) > _max_extra_symbols_)                                                           // caso o numero de simbolos extras nesta iteração seja maior ao max até agr
          _max_extra_symbols_ = decoded_idx - good_decoded_size;                                                              // define-se o max até agr de simbolos extras como o obtido agr
        recursive_decoder(encoded_idx,decoded_idx,good_decoded_size);                                                         // aceitar como a codificação encontrada nesta iteração como certa e ir para o próximo
        encoded_idx -= j;                                                                                                     // decrementar o numero de bits k a codificação encontrada teve
        decoded_idx --;                                                                                                       // decrementar o decoded_idx
      }                                                                                                                       //
    }                                                                                                                         //
  }                                                                                                                           //

  //Dead End
  if(retVal==0)                                                                                                               // atingiu um dead end
   return;                                                                                                                    //
}                                                                                                                             //

#endif  

// Code 6.0
#if 0
// Função de decodificação
static void recursive_decoder(int encoded_idx,int decoded_idx,int good_decoded_size)
{                                                                                                                             //
  _number_of_calls_ ++;                                                                                                       // incremento do numero de calls da recurse 
  int retVal = 0;                                                                                                             // bool da existencia de uma codificação para o que se tem  
  static int min_size;                                                                                                        // iniciação da variável k representa o tamanho minimo das codificações
  static int *sorted;                                                                                                         // iniciação do array k repreenta os indices da matriz das codificações sorted pelo length

  // Definir minimo size de codeword e preparar sorted array por length                    
  if(encoded_idx == 0 && decoded_idx == 0)                                                                                    // caso seja o inicio da das recursões averigua-se o tamanho
  {                                                                                                                           //
    sorted =(int *) malloc(_c_->n_symbols *sizeof(int));                                                                      // alocação de memória
    char codes[_c_->n_symbols][_c_->max_bits + 1];                                                                            // criação de uma matrix de codificações para dar sort e saber os sorted indexes
    min_size = _c_->max_bits;                                                                                                 // definir minimo como igual ao máximo size possivel dos codes                        
    for(int i=0 ; i < _c_->n_symbols ; i++)                                                                                   // iteração pelas codificações
    {                                                                                                                         //
      // Encontrar codificação mais pequena
      if(min_size > strlen(_c_->data[i].codeword))                                                                            // verificação se existe codificação menor k a encotrada
        min_size = strlen(_c_->data[i].codeword);                                                                             //
      
      // Por os indices no sorted 
      sorted[i] = i;                                                                                                          // inserção dos indices 
  
      // Copiar as codificações para dar sort
      strcpy(codes[i],_c_->data[i].codeword);                                                                                 // cópia das codificações para o array onde se irá fazer sort    
    }                                                                                                                         //    
                                                                                                              
    // Sort do array cópia em função dos sizes de codificação (Insertion sort)
    for (int i=1 ;i<_c_->n_symbols; i++)                                                                                      //
    {                                                                                                                         //
        char temp[strlen(codes[i])+1];                                                                                        //
        strcpy(temp,codes[i]);                                                                                                //
        int tempi = sorted[i];                                                                                                //  
        int j = i - 1;                                                                                                        //
        while (j >= 0 && strlen(temp) < strlen(codes[j]))                                                                     //
        {                                                                                                                     //
            strcpy(codes[j+1],codes[j]);                                                                                      //
            sorted[j+1]= sorted[j];                                                                                           //
            j--;                                                                                                              //
        }                                                                                                                     //
        strcpy(codes[j+1],temp);                                                                                              //
        sorted[j+1] = tempi;                                                                                                  //
    }                                                                                                                         //
  }                                                                                                                           //

  // Caso Terminal
  if((decoded_idx == _original_message_size_)){                                                                               // verificação se a mensagem decoded tem o mesmo tamanho k a original
    if(encoded_idx == strlen(_encoded_message_))                                                                              // verificação se foi percorrido todos os encoded bits
      _number_of_solutions_ ++;                                                                                               // encremento do numero de soluções
    return;                                                                                                                   //
  }                                                                                                                           //  

  // Tentativa de descodificação min_size by min_size
  for(int j = min_size ; j <= _c_->max_bits && (j+encoded_idx) <= strlen(_encoded_message_); j++){                            // itera sobre a codificação da mensagem desde onde estamos até ao máximo k uma codificação pode ter de bits 
    for(int i = 0 ;  strlen(_c_->data[sorted[i]].codeword) <= j && i < _c_->n_symbols ; i++){                                 // itera sobre as codificações desde a de tamanho j até parar de ser tamanho j
      char* code = _c_->data[sorted[i]].codeword;                                                                             // guarda-se a codificação a se testar
      for(int k = 0 ; k<=(j-1) && code[k]!='\0' ; k++){                                                                       // para cada bit da codificação a se comparar
        if(strlen(code) != j || code[k] != _encoded_message_[encoded_idx + k])                                                // verifica-se se o tamanho da codificação é igual à pretendida e se possui os bits iguais
          break;                                                                                                              // dá-se break para testar outra codificação caso n seja
        if(k+1==j)                                                                                                            // caso se esteja no final
          retVal = 1;                                                                                                         // altera-se o bool k representa a existencia de uma codificação possivel
      }                                                                                                                       //
      if(retVal){                                                                                                             // encontra-se uma codificação possivel
        retVal = 0;                                                                                                           // altera-se o bool k representa a existencia de uma codificação possivel
        encoded_idx += j;                                                                                                     // incrementar o numero de bits a codificação teve
        _decoded_message_[decoded_idx] = sorted[i];                                                                           // assumir o simbolo encontrado como certo
        if(_original_message_[decoded_idx] == sorted[i])                                                                      // verificar se o decoded simbolo é o certp
          good_decoded_size ++;                                                                                               // encrementar numero certo de decoded simbols se for o certo
        decoded_idx ++;                                                                                                       // incrementar o decoded_idx
        if((decoded_idx - good_decoded_size) > _max_extra_symbols_)                                                           // caso o numero de simbolos extras nesta iteração seja maior ao max até agr
          _max_extra_symbols_ = decoded_idx - good_decoded_size;                                                              // define-se o max até agr de simbolos extras como o obtido agr
        recursive_decoder(encoded_idx,decoded_idx,good_decoded_size);                                                         // aceitar como a codificação encontrada nesta iteração como certa e ir para o próximo
        encoded_idx -= j;                                                                                                     // decrementar o numero de bits k a codificação encontrada teve
        decoded_idx --;                                                                                                       // decrementar o decoded_idx
      }                                                                                                                       //
    }                                                                                                                         //
  }                                                                                                                           //

  //Dead End
  if(retVal==0)                                                                                                               // atingiu um dead end
   return;                                                                                                                    //
} 
# endif


// Code 7.0
#if 0
static void recursive_decoder(int encoded_idx,int decoded_idx,int good_decoded_size)
{                                                                                                                             //
  _number_of_calls_ ++;                                                                                                       // incremento do numero de calls da recurse 

  // Preparar sorted array por length                    
  if(encoded_idx == 0 && decoded_idx == 0)                                                                                    // caso seja o inicio da das recursões averigua-se o tamanho
  {                                                                                                                           //
    sorted =(int *) malloc(_c_->n_symbols *sizeof(int));                                                                      // alocação de memória
    char codes[_c_->n_symbols][_c_->max_bits + 1];                                                                            // criação de uma matrix de codificações para dar sort e saber os sorted indexes
    for(int i=0 ; i < _c_->n_symbols ; i++)                                                                                   // iteração pelas codificações
    {                                                                                                                         //
      // Por os indices no sorted 
      sorted[i] = i;                                                                                                          // inserção dos indices 
  
      // Copiar as codificações para dar sort
      strcpy(codes[i],_c_->data[i].codeword);                                                                                 // cópia das codificações para o array onde se irá fazer sort    
    }                                                                                                                         //    
                                                                                                              
    // Sort do array cópia em função dos sizes de codificação (Insertion sort)
    for (int i=1 ;i<_c_->n_symbols; i++)                                                                                      //
    {                                                                                                                         //
        char temp[strlen(codes[i])+1];                                                                                        //
        strcpy(temp,codes[i]);                                                                                                //
        int tempi = sorted[i];                                                                                                //  
        int j = i - 1;                                                                                                        //
        while (j >= 0 && strlen(temp) < strlen(codes[j]))                                                                     //
        {                                                                                                                     //
            strcpy(codes[j+1],codes[j]);                                                                                      //
            sorted[j+1]= sorted[j];                                                                                           //
            j--;                                                                                                              //
        }                                                                                                                     //
        strcpy(codes[j+1],temp);                                                                                              //
        sorted[j+1] = tempi;                                                                                                  //
    }                                                                                                                         //
  }                                                                                                                           // 

  // Caso Terminal
  if(_encoded_message_[encoded_idx] == '\0'){                                                                                 //
    _number_of_solutions_ ++;                                                                                                 // incremento do numero de soluções
    return;                                                                                                                   //  
  }                                                                                                                           //

  // Procura de simbolos
  for(int i = 0 ; i < _c_->n_symbols ; i++){                                                                                  // itera sobre as codificações
    int j;                                                                                                                    //
    for(j = 0 ; j < _c_->max_bits && _c_->data[sorted[i]].codeword[j] != '\0' ; j++){                                         // itera sobre a codificação da mensagem desde onde estamos até ao máximo k uma codificação pode ter de bits 
      if(_c_->data[sorted[i]].codeword[j] != _encoded_message_[encoded_idx + j] || _encoded_message_[j+encoded_idx] == '\0')  // verifica-se se os chars são iguais
        break;                                                                                                                // dá-se break para testar outra codificação caso n seja
    }                                                                                                                         //
    if(_c_->data[sorted[i]].codeword[j] == '\0' && _number_of_solutions_ != 1L){                                              // chegasse ao final da codificação com tds os bit iguais e ainda n se encontrou uma codificação certa

      if(_original_message_[decoded_idx] == sorted[i])                                                                        // verificar se o decoded simbolo é o certo
        good_decoded_size ++;                                                                                                 // encrementar numero certo de decoded simbols se for o certo
      if((decoded_idx - good_decoded_size) > _max_extra_symbols_)                                                             // caso o numero de simbolos extras nesta iteração seja maior ao max até agr
        _max_extra_symbols_ = ((decoded_idx+1) - good_decoded_size);                                                          // define-se o max até agr de simbolos extras como o obtido agr
      recursive_decoder(encoded_idx + j,decoded_idx + 1, good_decoded_size);                                                  // aceitar como a codificação encontrada nesta iteração como certa e ir para o próximo
    }                                                                                                                         //
  }                                                                                                                           //
}                                                                                                                             //
  
#endif

// Code 8.0
#if 1
static void recursive_decoder(int encoded_idx,int decoded_idx,int good_decoded_size)
{                                                                                                                             //
  _number_of_calls_ ++;                                                                                                       // incremento do numero de calls da recurse 

  // Caso Terminal
  if(_encoded_message_[encoded_idx] == '\0'){                                                                                 //
    _number_of_solutions_ ++;                                                                                                 // incremento do numero de soluções
    return;                                                                                                                   //  
  }                                                                                                                           //

  // Procura de simbolos
  for(int i = 0 ; i < _c_->n_symbols ; i++){                                                                                  // itera sobre as codificações
    int j;                                                                                                                    //
    for(j = 0 ; j < _c_->max_bits && _c_->data[i].codeword[j] != '\0' ; j++){                                                 // itera sobre a codificação da mensagem desde onde estamos até ao máximo k uma codificação pode ter de bits 
      if(_c_->data[i].codeword[j] != _encoded_message_[encoded_idx + j] || _encoded_message_[j+encoded_idx] == '\0')          // verifica-se se os chars são iguais
        break;                                                                                                                // dá-se break para testar outra codificação caso n seja
    }                                                                                                                         //
    if(_c_->data[i].codeword[j] == '\0' && _number_of_solutions_ != 1L){                                                      // chegasse ao final da codificação com tds os bit iguais e ainda n se tem soluções
      _decoded_message_[decoded_idx] = i;                                                                                     // assumir o simbolo encontrado como certo
      if(_original_message_[decoded_idx] == i)                                                                                // verificar se o decoded simbolo é o certo
        good_decoded_size ++;                                                                                                 // encrementar numero certo de decoded simbols se for o certo
      if((decoded_idx + 1- good_decoded_size) > _max_extra_symbols_)                                                         // caso o numero de simbolos extras nesta iteração seja maior ao max até agr
        _max_extra_symbols_ = ((decoded_idx+1) - good_decoded_size);                                                          // define-se o max até agr de simbolos extras como o obtido agr
      recursive_decoder(encoded_idx + j,decoded_idx + 1, good_decoded_size);                                                  // aceitar como a codificação encontrada nesta iteração como certa e ir para o próximo
    }                                                                                                                         //
  }                                                                                                                           //
}                                                                                                                             //
  
#endif

// Code 9.0
#if 0
static void recursive_decoder(int encoded_idx,int decoded_idx,int good_decoded_size)
{                                                                                                                             //
  _number_of_calls_ ++;                                                                                                       // incremento do numero de calls da recurse 
  static int *sorted;                                                                                                         //
  
  // Caso inicio do programa -> sort array pelo length da codificação 
  if(encoded_idx == 0 && decoded_idx == 0){
    sorted =(int *) malloc(_c_->n_symbols *sizeof(int));                                                                      //
    int codes[_c_->n_symbols];                                                                                                // cópia das codificações para o array onde se irá fazer sort    
    codes[0] = strlen(_c_->data[0].codeword);
    for (int i=1 ;i<_c_->n_symbols; i++)                                                                                      //
    {                                                                                                                         //
      int temp = strlen(_c_->data[i].codeword);                                                                               //
      int tempi = i;                                                                                                          //  
      int j = i - 1;                                                                                                          //
      while (j >= 0 && temp < codes[j])                                                                                       //
      {                                                                                                                       //
        codes[j+1] = codes[j];                                                                                                //
        sorted[j+1] = sorted[j];                                                                                              //
        j--;                                                                                                                  //
      }                                                                                                                       //
      codes[j+1] = temp;                                                                                                      //
      sorted[j+1] = tempi;                                                                                                    //
    }                                                                                                                         //
  }                                                                                                                           // 

  // Caso Terminal
  if(_encoded_message_[encoded_idx] == '\0'){                                                                                 //
    _number_of_solutions_ ++;                                                                                                 // incremento do numero de soluções
    return;                                                                                                                   //  
  }                                                                                                                           //

  // Procura de simbolos
  for(int i = 0 ; i < _c_->n_symbols ; i++){                                                                                  // itera sobre as codificações
    int j;                                                                                                                    //
    for(j = 0 ; j < _c_->max_bits && _c_->data[sorted[i]].codeword[j] != '\0' ; j++){                                         // itera sobre a codificação da mensagem desde onde estamos até ao máximo k uma codificação pode ter de bits 
      if(_c_->data[sorted[i]].codeword[j] != _encoded_message_[encoded_idx + j] || _encoded_message_[j+encoded_idx] == '\0')  // verifica-se se os chars são iguais
        break;                                                                                                                // dá-se break para testar outra codificação caso n seja
    }                                                                                                                         //
    if(_c_->data[sorted[i]].codeword[j] == '\0' && _number_of_solutions_ != 1L){                                              // chegasse ao final da codificação com tds os bit iguais e ainda n se encontrou uma codificação certa
      _decoded_message_[decoded_idx] = sorted[i];                                                                             // assumir o simbolo encontrado como certo
      if(_original_message_[decoded_idx] == sorted[i])                                                                        // verificar se o decoded simbolo é o certo
        good_decoded_size ++;                                                                                                 // encrementar numero certo de decoded simbols se for o certo
      if((decoded_idx + 1 - good_decoded_size) > _max_extra_symbols_)                                                         // caso o numero de simbolos extras nesta iteração seja maior ao max até agr
        _max_extra_symbols_ = ((decoded_idx+1) - good_decoded_size);                                                          // define-se o max até agr de simbolos extras como o obtido agr
      recursive_decoder(encoded_idx + j,decoded_idx + 1, good_decoded_size);                                                  // aceitar como a codificação encontrada nesta iteração como certa e ir para o próximo
    }                                                                                                                         //
  }                                                                                                                           //
}                                                                                                                             //
  
#endif

// Opcional 1.0
 
#if 0
static void recursive_decoder(int encoded_idx,int decoded_idx,int good_decoded_size)
{                                                                                                                             //
  _number_of_calls_ ++;                                                                                                       // incremento do numero de calls da recurse 

  // Calculo do numero de bits que faltam de ser descodificados até agora da codificação atual
  int code_decoded = 0;                                                                                                       //
  for(int d = 1 ; d <= decoded_idx ; d ++){                                                                                   // percorre todas as descodificações até agora aceites neste ramo
    for(int b = 0 ; _c_->data[_decoded_message_[d - 1]].codeword[b] != '\0' ; b++)                                            // percorre o numero de bits de cada codificação encontrada até agora
      code_decoded++;                                                                                                         // encrementa-se o numero de bits previamente analisados
  }                                                                                                                           //
  code_decoded = code_decoded - encoded_idx;                                                                                  // número de bits que nos falta codificar

  // Caso Terminal
  if(_encoded_message_[encoded_idx] == '\0' && code_decoded == 0){                                                                                 // caso se chegue ao final da mensagem
    _number_of_solutions_ ++;                                                                                                 // incremento do numero de soluções
    
    // Print what we got and the right solution
    printf("\nOriginal:\n");
    for(int i = 0 ; i < _original_message_size_; i++)
      printf("%d",_original_message_[i]);
    printf("\nDecoded:\n");
    for(int i = 0 ; i < _original_message_size_; i++)
      printf("%d",_decoded_message_[i]);
    printf("\n");
    return;                                                                                                                   //  
  } 
 
  // Procura de possiveis codificações para futura verificações
  if(code_decoded == 0){                                                                                                      // caso n nos falte codificar nenhum
    for(int i = 0 ; i < _c_->n_symbols ; i++){                                                                                // iteração por todas as codificações
      if(_encoded_message_[encoded_idx] == _c_->data[i].codeword[0]){                                                         // procura-se uma codificação k tenha o inicio semelhante ao bit atualmente recebido
        _decoded_message_[decoded_idx] = i;                                                                                   // simbolo k se inicia com o bit recebido
        if(_original_message_[decoded_idx] == i)                                                                              // verificar se o decoded simbolo é o certo
          good_decoded_size ++;                                                                                               // encrementar numero certo de decoded simbols se for o certo
        if((decoded_idx + 1 - good_decoded_size) > _max_extra_symbols_)                                                       // caso o numero de simbolos extras nesta iteração seja maior ao max até agr
          _max_extra_symbols_ = ((decoded_idx+1) - good_decoded_size);                                                        // define-se o max até agr de simbolos extras como o obtido agr
        recursive_decoder(encoded_idx + 1, decoded_idx + 1, good_decoded_size);                                               // chamamento da função para se confirmar o próximo bit
      }                                                                                                                       //
    }                                                                                                                         //

  // Verificação dos bits dentro da codificaçao a se avaliar
  }else{                                                                                                                      // falta verificar ainda alguns bits
    code_decoded = strlen(_c_->data[_decoded_message_[decoded_idx - 1]].codeword ) - code_decoded;                            // index do bit onde estamos em relação à codificação atualmente a ser avaliada
    if(_c_->data[_decoded_message_[decoded_idx - 1]].codeword[code_decoded] == _encoded_message_[encoded_idx]){               // caso o bit a ser analisado atualmente seja igual ao bit da posição respetiva dentro  da codificação
      recursive_decoder(encoded_idx + 1, decoded_idx, good_decoded_size);                                                     // faz-se chamamento da função para o bit seguinte
    }else{                                                                                                                    // caso os bits sejam errados
      return;                                                                                                                 // a codificação testada está errada
    }                                                                                                                         //
  }                                                                                                                           //
}                                                                                                                             //
  
#endif

// Opcional 2.0
 
#if 0
static void recursive_decoder(int encoded_idx,int decoded_idx,int good_decoded_size)
{                                                                                                                             //
  _number_of_calls_ ++;                                                                                                       // incremento do numero de calls da recurse 

  // Calculo do numero de bits que faltam de ser descodificados até agora da codificação atual
  int code_decoded = 0;                                                                                                       //
  for(int d = 1 ; d <= decoded_idx ; d ++){                                                                                   // percorre todas as descodificações até agora aceites neste ramo
    for(int b = 0 ; _c_->data[_decoded_message_[d - 1]].codeword[b] != '\0' ; b++)                                            // percorre o numero de bits de cada codificação encontrada até agora
      code_decoded++;                                                                                                         // encrementa-se o numero de bits previamente analisados
  }                                                                                                                           //
  code_decoded = code_decoded - encoded_idx;                                                                                  // número de bits que nos falta codificar

  // Caso Terminal
  if(_encoded_message_[encoded_idx] == '\0' && code_decoded == 0){                                                            // caso se chegue ao final da mensagem
    _number_of_solutions_ ++;                                                                                                 // incremento do numero de soluções
    
    // Print what we got and the right solution
    printf("\nOriginal:\n");
    for(int i = 0 ; i < _original_message_size_; i++)
      printf("%d",_original_message_[i]);
    printf("\nDecoded:\n");
    for(int i = 0 ; i < _original_message_size_; i++)
      printf("%d",_decoded_message_[i]);
    printf("\n");
    return;                                                                                                                   //  
  } 
 
  // Procura de possiveis codificações para futura verificações
  if(code_decoded == 0){                                                                                                      // caso n nos falte codificar nenhum
    for(int i = 0 ; i < _c_->n_symbols ; i++){                                                                                // iteração por todas as codificações
      if(_encoded_message_[encoded_idx] == _c_->data[i].codeword[0] && _number_of_solutions_ != 1L){                          // procura-se uma codificação k tenha o inicio semelhante ao bit atualmente recebido
        _decoded_message_[decoded_idx] = i;                                                                                   // simbolo k se inicia com o bit recebido
        if(_original_message_[decoded_idx] == i)                                                                              // verificar se o decoded simbolo é o certo
          good_decoded_size ++;                                                                                               // encrementar numero certo de decoded simbols se for o certo
        if((decoded_idx + 1 - good_decoded_size) > _max_extra_symbols_)                                                       // caso o numero de simbolos extras nesta iteração seja maior ao max até agr
          _max_extra_symbols_ = ((decoded_idx+1) - good_decoded_size);                                                        // define-se o max até agr de simbolos extras como o obtido agr
        recursive_decoder(encoded_idx + 1, decoded_idx + 1, good_decoded_size);                                               // chamamento da função para se confirmar o próximo bit
      }                                                                                                                       //
    }                                                                                                                         //

  // Verificação dos bits dentro da codificaçao a se avaliar
  }else{                                                                                                                      // falta verificar ainda alguns bits
    code_decoded = strlen(_c_->data[_decoded_message_[decoded_idx - 1]].codeword ) - code_decoded;                            // index do bit onde estamos em relação à codificação atualmente a ser avaliada
    if(_c_->data[_decoded_message_[decoded_idx - 1]].codeword[code_decoded] == _encoded_message_[encoded_idx] && _number_of_solutions_ != 1L){//bit a ser analisado atualmente seja igual ao bit da posição respetiva dentro  da codificação
      recursive_decoder(encoded_idx + 1, decoded_idx, good_decoded_size);                                                     // faz-se chamamento da função para o bit seguinte
    }else{                                                                                                                    // caso os bits sejam errados
      return;                                                                                                                 // a codificação testada está errada
    }                                                                                                                         //
  }                                                                                                                           //
}                                                                                                                             //
  
#endif

//
// Encode and decode driver
//
// DO NOT CHANGE THIS CODE
//

void try_it(code_t *c,int message_size,int show_results)
{
  if(message_size < 1 || message_size > MAX_MESSAGE_SIZE)
  {
    fprintf(stderr,"try_it: bad message size (%d)\n",message_size);
    exit(1);
  }
  _c_ = c;
  _original_message_size_ = message_size;
  _max_encoded_message_size_ = message_size * c->max_bits;
  _max_decoded_message_size_ = message_size + 2000;
  _original_message_ = (int *)malloc((size_t)_original_message_size_ * sizeof(int));
  _encoded_message_ = (char *)malloc((size_t)(_max_encoded_message_size_ + 1) * sizeof(char));
  _decoded_message_ = (int *)malloc((size_t)_max_decoded_message_size_ * sizeof(int));
  _number_of_calls_ = 0L;
  _number_of_solutions_ = 0L;
  _max_extra_symbols_ = -1;
  if(_original_message_ == NULL || _encoded_message_ == NULL || _decoded_message_ == NULL)
  {
    fprintf(stderr,"try it: out of memory!\n");
    exit(1);
  }
  random_message(_c_,_original_message_size_,_original_message_);
  encode_message(_c_,_original_message_size_,_original_message_,_max_encoded_message_size_,_encoded_message_);
  recursive_decoder(0,0,0);
  if(_number_of_solutions_ != 1L)
  {
    fprintf(stderr,"number of solutions: %ld\n",_number_of_solutions_);
    fprintf(stderr,"number of function calls: %ld (%.3f per message symbol)\n",_number_of_calls_,(double)_number_of_calls_ / (double)_original_message_size_);
    fprintf(stderr,"number of extra symbols: %d\n",_max_extra_symbols_);
  }
  if(show_results != 0)
  {
    //
    // print some data about this particular case (average number of calls per symbol, worst probe lookahead)
    //
    printf("%4d %9.3f %3d\n",_c_->n_symbols,(double)_number_of_calls_ / (double)_original_message_size_,_max_extra_symbols_);
    fflush(stdout);
  }
  free(_original_message_); _original_message_ = NULL;
  free(_encoded_message_);  _encoded_message_  = NULL;
  free(_decoded_message_);  _decoded_message_  = NULL;
}


//
// Main program
//
// DO NOT CHANGE THIS CODE
//

int main(int argc,char **argv)
{
  //
  // Show code words (called with arguments -s n_symbols seed)
  //
  if(argc == 4 && argv[1][0] == '-' && argv[1][1] == 's')
  {
    int seed,n_symbols,i;
    code_t *c;

    n_symbols = atoi(argv[2]);
    seed = atoi(argv[3]);
    srandom(seed);
    c = new_code(n_symbols);
    printf("seed: %d\n",seed);
    printf("number of symbols: %d\n",c->n_symbols);
    printf("maximum bits of a code word: %d\n\n",c->max_bits);
    printf("symb freq  cfreq codeword\n");
    printf("---- ---- ------ --------------------\n");
    for(i = 0;i < c->n_symbols;i++)
      printf("%4d %4d %6d %s\n",i,c->data[i].scaled_prob,c->data[i].cum_scaled_prob,c->data[i].codeword);
    printf("---- ---- ------ --------------------\n\n");
    free_code(c);
    return 0;
  }
  //
  // Encode and decode a message (called with arguments -t [n_symbols [message_size [seed]]])
  //
  if(argc >= 2 && argc <= 5 && argv[1][0] == '-' && argv[1][1] == 't')
  {
    int n_symbols,message_size,seed;
    code_t *c;

    n_symbols = (argc < 3) ? 3 : atoi(argv[2]);
    message_size = (argc < 4) ? 10 : atoi(argv[3]);
    seed = (argc < 5) ? 1 : atoi(argv[4]);
    srandom(seed);
    c = new_code(n_symbols);
    try_it(c,message_size,1);
    free_code(c);
    return 0;
  }
  //
  // Try the first N_MEASUREMENTS seeds (called with arguments -x n_symbols)
  //
  if(argc == 3 && argv[1][0] == '-' && argv[1][1] == 'x')
  {
    double t,t_min,t_max,t_avg,t_data[N_MEASUREMENTS],u_avg;
    int u,u_min,u_max,u_data[N_MEASUREMENTS];
    int seed,n_symbols,i;
    code_t *c;

    n_symbols = atoi(argv[2]);
    if(n_symbols == 2)
    {
      printf("# data for MAX_MESSAGE_SIZE equal to %d\n",MAX_MESSAGE_SIZE);
      printf("# data for N_OUTLIERS equal to %d\n",N_OUTLIERS);
      printf("# data for N_VALID equal to %d\n",N_VALID);
      printf("#\n");
      printf("#      number of calls per message symbol      lookahead symbols\n");
      printf("#     -----------------------------------  ---------------------\n");
      printf("# ns       min      avg      med      max   min    avg  med  max\n");
      printf("#---  -------- -------- -------- --------  ---- ------ ---- ----\n");
    }
    if(n_symbols < 3 || n_symbols > MAX_N_SYMBOLS)
    {
      fprintf(stderr,"main: bad number of symbols for the -x command line option\n");
      exit(1);
    }
    t_min = t_max = 0.0;
    u_min = u_max = 0;
    for(seed = 1;seed <= N_MEASUREMENTS;seed++)
    {
      srandom(seed);
      c = new_code(n_symbols);
      try_it(c,MAX_MESSAGE_SIZE,0);
      free_code(c);
      t = (double)_number_of_calls_ / (double)MAX_MESSAGE_SIZE;
      u = _max_extra_symbols_;
      if(seed == 1 || t < t_min)
        t_min  = t;
      if(seed == 1 || t > t_max)
        t_max  = t;
      if(seed == 1 || u < u_min)
        u_min  = u;
      if(seed == 1 || u > u_max)
        u_max  = u;
      for(i = seed - 1;i > 0 && t_data[i - 1] > t;i--) // inner loop of insertion sort!
        t_data[i] = t_data[i - 1];
      t_data[i] = t;
      for(i = seed - 1;i > 0 && u_data[i - 1] > u;i--) // inner loop of insertion sort!
        u_data[i] = u_data[i - 1];
      u_data[i] = u;
    }
    t_avg = u_avg = 0.0;
    for(i = N_OUTLIERS;i < N_MEASUREMENTS - N_OUTLIERS;i++)
    {
      t_avg += t_data[i];
      u_avg += (double)u_data[i];
    }
    t_avg /= (double)(2 * N_VALID + 1);
    u_avg /= (double)(2 * N_VALID + 1);
    printf("%4d  %8.3f %8.3f %8.3f %8.3f  %4d %6.1f %4d %4d\n",n_symbols,t_min,t_avg,t_data[N_OUTLIERS + N_VALID],t_max,u_min,u_avg,u_data[N_OUTLIERS + N_VALID],u_max);
    return 0;
  }
  //
  // Help message
  //
  fprintf(stderr,"usage: %s -s n_symbols seed                     # show the code words of random code\n",argv[0]);
  fprintf(stderr,"       %s -t [n_symbols [message_size [seed]]]  # encode and decode a message\n",argv[0]);
  fprintf(stderr,"       %s -x n_symbols                          # try the first %d seeds\n",argv[0],N_MEASUREMENTS);
  return 1;
}
