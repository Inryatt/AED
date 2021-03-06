////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// AED, 2020/2021
//
// TODO: Pedro   - 97827
// TODO: Camila  - 97880
// TODO: Rodrigo - 98475
//
// Brute-force solution of the generalized weighted job selection problem
//
// Compile with "cc -Wall -O2 job_selection.c -lm" or equivalent
//
// In the generalized weighted job selection problem we will solve here we have T programming tasks and P programmers.
// Each programming task has a starting date (an integer), an ending date (another integer), and a profit (yet another
// integer). Each programming task can be either left undone or it can be done by a single programmer. At any given
// date each programmer can be either idle or it can be working on a programming task. The goal is to select the
// programming tasks that generate the largest profit.
//


// Things to do:
//   0. (mandatory)
//      Place the student numbers and names at the top of this file. 

//   1. (highly recommended)
//      Read and understand this code. 

//   2. (mandatory)
//      Solve the problem for each student number of the group and for
//        N=1, 2, ..., as higher as you can get and
//        P=1, 2, ... min(8,N)
//      Present the best profits in a table (one table per student number).
//      Present all execution times in a graph (use a different color for the times of each student number).
//      Draw the solutions for the highest N you were able to do.


//   3. (optional) 
//      Ignore the profits (or, what is the same, make all profits equal); what is the largest number of programming
//      tasks that can be done? 

//   4. (optional)
//      Count the number of valid task assignments. Calculate and display an histogram of the number of occurrences of
//      each total profit. Does it follow approximately a normal distribution?

//   5. (optional)
//      Try to improve the execution time of the program (use the branch-and-bound technique).
//      Can you use divide and conquer to solve this problem? yes.
//      Can you use dynamic programming to solve this problem? no.

//   6. (optional)
//      For each problem size, and each student number of the group, generate one million (or more!) valid random
//      assignments and compute the best solution found in this way. Compare these solutions with the ones found in
//      item 2.

//   7. (optional)
//      Surprise us, by doing something more! DONE

//   8. (mandatory)
//      Write a report explaining what you did. Do not forget to put all your code in an appendix.
//

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "../../P02/elapsed_time.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Random number generator interface (do not change anything in this code section)
//
// In order to ensure reproducible results on Windows and GNU/Linux, we use a good random number generator, available at
//   https://www-cs-faculty.stanford.edu/~knuth/programs/rng.c
// This file has to be used without any modifications, so we take care of the main function that is there by applying
// some C preprocessor tricks
//

#define main rng_main // main gets replaced by rng_main
#ifdef __GNUC__
int rng_main() __attribute__((__unused__)); // gcc will not complain if rnd_main() is not used
#endif
#include "rng.c"
#undef main // main becomes main again

#define srandom(seed) ran_start((long)seed) // start the pseudo-random number generator
#define random() ran_arr_next()             // get the next pseudo-random number (0 to 2^30-1)

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// problem data (if necessary, add new data fields in the structures; do not change anything else in this code section)
//
// on the data structures declared below, a comment starting with
// * a I means that the corresponding field is initialized by init_problem()
// * a S means that the corresponding field should be used when trying all possible cases
// * IS means both (part initialized, part used)
//

#if 1

#define MAX_T 64 // maximum number of programming tasks
#define MAX_P 10 // maximum number of programmers

typedef struct
{
  int starting_date;    // I starting date of this task
  int ending_date;      // I ending date of this task
  int profit;           // I the profit if this task is performed
  int assigned_to;      // S current programmer number this task is assigned to (use -1 for no assignment)
  int best_assigned_to; // S current programmer number this task is assigned to (use -1 for no assignment)
} task_t;

typedef struct
{
  int NMec;           // I  student number
  int T;              // I  number of tasks
  int P;              // I  number of programmers
  int I;              // I  if 1, ignore profits
  long long casos;          // I  if 1, ignore profits
  int total_profit;   // S  current total profit
  int biggest_profit; // S  current total profit
  double cpu_time;    // S  time it took to find the solution
  task_t task[MAX_T]; // IS task data
  int busy[MAX_P];    // S  for each programmer, record until when she/he is busy (-1 means idle)
  char dir_name[16];  // I  directory name where the solution file will be created
  char file_name[64]; // I  file name where the solution data will be stored
} problem_t;

int compare_tasks_E(const void *t1, const void *t2)       // compara????o das tasks com prioridade o ending date
{
  int d1, d2;

  d1 = ((task_t *)t1)->ending_date;
  d2 = ((task_t *)t2)->ending_date;
  if (d1 != d2)
    return (d1 < d2) ? -1 : +1;
  d2 = ((task_t *)t2)->starting_date;
  d1 = ((task_t *)t1)->starting_date;
  if (d1 != d2)
    return (d1 < d2) ? -1 : +1;
  return 0;
}

int compare_tasks(const void *t1, const void *t2)
{
  int d1, d2;

  d1 = ((task_t *)t1)->starting_date;
  d2 = ((task_t *)t2)->starting_date;
  if (d1 != d2)
    return (d1 < d2) ? -1 : +1;
  d1 = ((task_t *)t1)->ending_date;
  d2 = ((task_t *)t2)->ending_date;
  if (d1 != d2)
    return (d1 < d2) ? -1 : +1;
  return 0;
}

void init_problem(int NMec, int T, int P, int ignore_profit, problem_t *problem)
{
  int i, r, scale, span, total_span;
  int *weight;
  //
  // input validation
  //
  if (NMec < 1 || NMec > 999999)
  {
    fprintf(stderr, "Bad NMec (1 <= NMex (%d) <= 999999)\n", NMec);
    exit(1);
  }
  if (T < 1 || T > MAX_T)
  {
    fprintf(stderr, "Bad T (1 <= T (%d) <= %d)\n", T, MAX_T);
    exit(1);
  }
  if (P < 1 || P > MAX_P)
  {
    fprintf(stderr, "Bad P (1 <= P (%d) <= %d)\n", P, MAX_P);
    exit(1);
  }
  //
  // the starting and ending dates of each task satisfy 0 <= starting_date <= ending_date <= total_span
  //
  total_span = (10 * T + P - 1) / P;
  if (total_span < 30)
    total_span = 30;
  //
  // probability of each possible task duration
  //
  // task span relative probabilities
  //
  // |  0  0  4  6  8 10 12 14 16 18 | 20 | 19 18 17 16 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1 | smaller than 1
  // |  0  0  2  3  4  5  6  7  8  9 | 10 | 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 | 30 31 ... span
  //
  weight = (int *)alloca((size_t)(total_span + 1) * sizeof(int)); // allocate memory (freed automatically)
  if (weight == NULL)
  {
    fprintf(stderr, "Strange! Unable to allocate memory\n");
    exit(1);
  }
#define sum1 (298.0)                     // sum of weight[i] for i=2,...,29 using the data given in the comment above
#define sum2 ((double)(total_span - 29)) // sum of weight[i] for i=30,...,data_span using a weight of 1
#define tail 100
  scale = (int)ceil((double)tail * 10.0 * sum2 / sum1); // we want that scale*sum1 >= 10*tail*sum2, so that large task
  if (scale < tail)                                     // durations occur 10% of the time
    scale = tail;
  weight[0] = 0;
  weight[1] = 0;
  for (i = 2; i <= 10; i++)
    weight[i] = scale * (2 * i);
  for (i = 11; i <= 29; i++)
    weight[i] = scale * (30 - i);
  for (i = 30; i <= total_span; i++)
    weight[i] = tail;
#undef sum1
#undef sum2
#undef tail
  //
  // accumulate the weigths (cummulative distribution)
  //
  for (i = 1; i <= total_span; i++)
    weight[i] += weight[i - 1];
  //
  // generate the random tasks
  //
  srandom(NMec + 314161 * T + 271829 * P);
  problem->NMec = NMec;
  problem->T = T;
  problem->P = P;
  problem->I = (ignore_profit == 0) ? 0 : 1;
  for (i = 0; i < T; i++)
  {
    //
    // task starting an ending dates
    //
    r = 1 + (int)random() % weight[total_span]; // 1 .. weight[total_span]
    for (span = 0; span < total_span; span++)
      if (r <= weight[span])
        break;
    problem->task[i].starting_date = (int)random() % (total_span - span + 1);
    problem->task[i].ending_date = problem->task[i].starting_date + span - 1;
    //
    // task profit
    //
    // the task profit is given by r*task_span, where r is a random variable in the range 50..300 with a probability
    //   density function with shape (two triangles, the area of the second is 4 times the area of the first)
    //
    //      *
    //     /|   *
    //    / |       *
    //   /  |           *
    //  *---*---------------*
    // 50 100 150 200 250 300
    //
    scale = (int)random() % 12501; // almost uniformly distributed in 0..12500
    if (scale <= 2500)
      problem->task[i].profit = 1 + round((double)span * (50.0 + sqrt((double)scale)));
    else
      problem->task[i].profit = 1 + round((double)span * (300.0 - 2.0 * sqrt((double)(12500 - scale))));
  }
  //
  // sort the tasks by the starting date
  //
  qsort((void *)&problem->task[0], (size_t)problem->T, sizeof(problem->task[0]), compare_tasks);
  //
  // finish
  //
  if (problem->I != 0)
    for (i = 0; i < problem->T; i++)
      problem->task[i].profit = 1;
#define DIR_NAME problem->dir_name
  if (snprintf(DIR_NAME, sizeof(DIR_NAME), "%06d", NMec) >= sizeof(DIR_NAME))
  {
    fprintf(stderr, "Directory name too large!\n");
    exit(1);
  }
#undef DIR_NAME
#define FILE_NAME problem->file_name
  if (snprintf(FILE_NAME, sizeof(FILE_NAME), "%06d/%02d_%02d_%d.txt", NMec, T, P, problem->I) >= sizeof(FILE_NAME))
  {
    fprintf(stderr, "File name too large!\n");
    exit(1);
  }
#undef FILE_NAME
}

#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// problem solution (place your solution here)
//

int recurse(problem_t *prob, int t)
{

  int busy_save;                                                                        
  int profit_save;
  int task_save;
  if (t >= prob->T)                                                                       // verifica????o do caso terminal: t(numero de task a ser avaliade) >= prob->T (n??mero de tasks k o problema gerou)
  {
    prob->casos++;                                                                        // incremento dos casos possiveis
    if (prob->total_profit > prob->biggest_profit)                                        // verifica????o do profit da sequencia de taks ?? superior ou inferior ?? do m??ximo profit at?? agr obtido
    {
      prob->biggest_profit = prob->total_profit;                                          // defenir o maior profit obtido at?? agr como o profit obtido pela sequ??ncia de tasks  
      for (int j = 0; j < prob->T; j++)                                                   // itera????o por todas as tasks
      {
        prob->task[j].best_assigned_to = prob->task[j].assigned_to;                       // definir o task assignment obtido pela sequ??ncia pr??via de tasks como a melhor sequ??ncia at?? agr    
      }
    }
    return 1;
  }

  recurse(prob, t + 1);                                                                   // chamada da fun????o como argumento a pr??xima task sem a sua adi????o a qlqr programador

  for (int p = 0; p < prob->P; p++)                                                       // itera????o por tds os programadores
  {
    if (prob->busy[p] < prob->task[t].starting_date && prob->task[t].assigned_to < 0)     // verifica????o se um programador tem a sua pr??via task com um ending date menor ao starting date da task k se pretende adicionar e verifica????o se a task est?? assigned a ningu??m
    {
      busy_save = prob->busy[p];                                                          // guardar o estado do programador
      task_save = prob->task[t].assigned_to;                                              // guardar o assignment da task
      profit_save = prob->total_profit;                                                   // guardar o profit

      prob->busy[p] = prob->task[t].ending_date;                                          // defini????o do ending date como o ending date da task do programador
      prob->task[t].assigned_to = p;                                                      // assignment da task para o programador disponivel
      prob->total_profit += prob->task[t].profit;                                         // icremento ao profit total igual ao profit da task
      recurse(prob, t + 1);                                                               // chamada da recurs??o da task seguinte com o assignment da task avaliada previamente
      prob->busy[p] = busy_save;                                                          // redefini????o dos valores pr??vios de busy
      prob->task[t].assigned_to = task_save;                                              // redefini????o dos valores pr??vios do assignment
      prob->total_profit = profit_save;                                                   // redefini????o dos valores pr??vios do profit
      break;                                                                              // break caso a fun????o encontre um programador disponivel
    }
  }
  return 0;
}

#if 0
int nonRec(problem_t *problem){
#define TASK problem->task[t]
#define PROG problem->busy[0]
  int t = 0;
  PROG = TASK.ending_date;
  problem->biggest_profit+=TASK.profit;
  TASK.best_assigned_to = 0;

  for( t = 1; t < problem->T; t++){
    if( TASK.starting_date > PROG )
    {
      PROG = TASK.ending_date;
      problem->biggest_profit+=TASK.profit;
      TASK.best_assigned_to = 0;
    }
  }
#undef TASK
#undef PROG
  return 1;
}
#endif

#if 1
int nonRec(problem_t *problem, int programmer){
#define TASK  problem->task[t]
#define PROG  problem->busy[programmer]
  int t = 0;

  for( t = 0; t < problem->T; t++){                                                       // itera????o pelas tasks tdas do problema 
    if( TASK.starting_date > PROG && TASK.best_assigned_to == -1 )                        // verifica????o se a task possui um starting date maior ao ending date date da task previamente assigned e verifica????o se esta n??o est?? assigned
    {
      PROG = TASK.ending_date;                                                            // defeni????o do ending como busy do programador a se dar assignment
      problem->biggest_profit+=TASK.profit;                                               // incremento do profit (neste caso numero de tasks assigned)
      TASK.best_assigned_to = programmer;                                                 // assignment da task ao programador
    }
  }

#undef TASK
#undef PROG
  return 1;
}
#endif

#if 1

static void solve(problem_t *problem)
{
  FILE *fp;
  int i;

  //
  // open log file
  //
  (void)mkdir(problem->dir_name, S_IRUSR | S_IWUSR | S_IXUSR);
  fp = fopen(problem->file_name, "w");
  if (fp == NULL)
  {
    fprintf(stderr, "Unable to create file %s (maybe it already exists? If so, delete it!)\n", problem->file_name);
    exit(1);
  }
  //
  // solve
  //
  problem->cpu_time = cpu_time();
#define TASK problem->task[t]
  for (int t = 0; t < problem->T; t++)
  {
    TASK.assigned_to = -1;
    TASK.best_assigned_to = -1;
  }
#undef TASK
#define PROG problem->busy[p]
  for (int p = 0; p < problem->P; p++)
  {
    PROG = -1;
  }
#undef PROG
  problem->casos = 0;
  problem->biggest_profit = 0;
  problem->total_profit = 0;

//
  if (problem->I == 1 && problem->P == 1)
  {
    qsort((void *)&problem->task[0], (size_t)problem->T, sizeof(problem->task[0]), compare_tasks_E);
    puts("nrecurse");
    nonRec(problem, 0);
  }
  else
  {
    puts("recurse");
    recurse(problem, 0);
  }
  // call your (recursive?) function to solve the problem here
  problem->cpu_time = cpu_time() - problem->cpu_time;
  //
  // save solution data
  //
#define TASK problem->task[t]
  for (int t = 0; t < problem->T; t++)
  {
    fprintf(fp, "P%d\t%d T%d %d \n", TASK.best_assigned_to, TASK.starting_date, t, TASK.ending_date);
  }
#undef TASK
  fprintf(fp, "NMec = %d\n", problem->NMec);
  fprintf(fp, "Viable Sol. = %lld\n", problem->casos);
  fprintf(fp, "Profit = %d\n", problem->biggest_profit);
  fprintf(fp, "T = %d\n", problem->T);
  fprintf(fp, "P = %d\n", problem->P);
  fprintf(fp, "Profits%s ignored\n", (problem->I == 0) ? " not" : "");
  fprintf(fp, "Solution time = %.3e\n", problem->cpu_time);
  fprintf(fp, "Task data\n");
#define TASK problem->task[i]
  for (i = 0; i < problem->T; i++)
    fprintf(fp, "%02d  %3d %3d %5d\n", i, TASK.starting_date, TASK.ending_date, TASK.profit);
#undef TASK
  fprintf(fp, "End\n");
  //
  // terminate
  //
  if (fflush(fp) != 0 || ferror(fp) != 0 || fclose(fp) != 0)
  {
    fprintf(stderr, "Error while writing data to file %s\n", problem->file_name);
    exit(1);
  }
}

#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// main program
//

int main(int argc, char **argv)
{
  problem_t problem;
  int NMec, T, P, I;

  NMec = (argc < 2) ? 2020 : atoi(argv[1]);
  T = (argc < 3) ? 5 : atoi(argv[2]);
  P = (argc < 4) ? 2 : atoi(argv[3]);
  I = (argc < 5) ? 0 : atoi(argv[4]);
  init_problem(NMec, T, P, I, &problem);
  solve(&problem);
  return 0;
}