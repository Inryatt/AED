from dataclasses import dataclass
import os
import math
import random
import array
import sys
import time
import operator
MAX_T=64
MAX_P=10

# adaptado de https://www.geeksforgeeks.org/python-program-for-quicksort/
def partition(arr, low, high):
    i = (low-1)         # index of smaller element
    pivot = arr[high]     # pivot
 
    for j in range(low, high):
 
        # If current element is smaller than or
        # equal to pivot
        if arr[j].starting_date <= pivot.starting_date:
 
            # increment index of smaller element
            i = i+1
            arr[i], arr[j] = arr[j], arr[i]
 
    arr[i+1], arr[high] = arr[high], arr[i+1]
    return (i+1)

def quickSort(arr, low, high):
    if len(arr) == 1:
        return arr
    if low < high:
 
        # pi is partitioning index, arr[p] is now
        # at right place
        pi = partition(arr, low, high)
 
        # Separately sort elements before
        # partition and after partition
        quickSort(arr, low, pi-1)
        quickSort(arr, pi+1, high)

def partitionE(arr, low, high):
    i = (low-1)         # index of smaller element
    pivot = arr[high]     # pivot
 
    for j in range(low, high):
 
        # If current element is smaller than or
        # equal to pivot
        if arr[j].ending_date <= pivot.ending_date:
 
            # increment index of smaller element
            i = i+1
            arr[i], arr[j] = arr[j], arr[i]
 
    arr[i+1], arr[high] = arr[high], arr[i+1]
    return (i+1)

def quickSortE(arr, low, high):
    if len(arr) == 1:
        return arr
    if low < high:
 
        # pi is partitioning index, arr[p] is now
        # at right place
        pi = partitionE(arr, low, high)
 
        # Separately sort elements before
        # partition and after partition
        quickSort(arr, low, pi-1)
        quickSort(arr, pi+1, high)

@dataclass
class task_t:
    starting_date: int
    ending_date: int
    profit: int
    assigned_to: int
    best_assigned_to: int 

    def __init__(self):
        self.starting_date= -1
        self.ending_date= -1
        self.profit= -1
        self.assigned_to= -1
        self.best_assigned_to= -1
    

@dataclass
class problem_t:
    NMec: int
    T: int
    P: int
    I: int
    casos: int
    total_profit: int
    biggest_profit: int
    cpu_time: float
    task: task_t
    busy: []  # should be ints
    dir_name: str
    file_name: str

    def __init__(self):
        self.NMec= -1
        self.T= -1
        self.P= -1
        self.I= -1
        self.casos= 0
        self.total_profit= 0
        self.biggest_profit= 0
        self.cpu_time= 0
        self.task= [task_t() for i in range(MAX_T+1)] 
        self.busy= [-1 for i in range(MAX_P+1)] #should be ints
        self.dir_name= ""
        self.file_name= ""


MAX_T=64
MAX_P=10


def init_problem(NMec,T,P,ignore_profit,problem):
    if(NMec < 1 or NMec > 999999):
        print('Bad NMec (1 <= NMex ({nmec:d}) <= 999999)\n'.format(nmec = NMec))
        exit(1)
    if(T < 1 or T > MAX_T):
        print("Bad T (1 <= T ({t:d}) <= {maxt:d})\n".format(t = T, maxt = MAX_T))
        exit(1)
    if(P < 1 or P > MAX_P):
        print("Bad P (1 <= P ({p:d}) <= {maxp:d})\n".format(p = P, maxp = MAX_P))
        exit(1)
    #
    # the starting and ending dates of each task satisfy 0 <= starting_date <= ending_date <= total_span
    #

    total_span = int((10 * T + P - 1) / P)
    if(total_span < 30):
        total_span = 30
    #
    # probability of each possible task duration
    #
    # task span relative probabilities
    #
    # |  0  0  4  6  8 10 12 14 16 18 | 20 | 19 18 17 16 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1 | smaller than 1
    # |  0  0  2  3  4  5  6  7  8  9 | 10 | 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 | 30 31 ... span
    #
    weight = [0 for i in range(total_span + 1)]
    sum1= 298.0
    sum2= total_span - 29
    tail= 100
    scale = math.ceil(tail * 10.0 * sum2 / sum1)
    if(scale < tail):
        scale = tail
    weight[0] = 0
    weight[1] = 1
    for i in range(2,11):
        weight[i] = scale * (2 * i)
    for i in range(11,30):
        weight[i] = scale * (30 - i)
    for i in range(30, total_span + 1):
        weight[i] = tail
    #
    # accumulate the weigths (cummulative distribution)
    #
    for i in range(1,total_span + 1):
        weight[i] += weight[i - 1]
    #
    # generate the random tasks
    #
    random.seed(NMec + 314161 * T + 271829 * P)
    problem.NMec = NMec
    problem.T = T
    problem.P = P
    problem.I = ignore_profit
    for i in range(0,T):
        #
        # task starting an ending dates
        #
        r = 1 + random.randint(0,2^30-1) % weight[total_span] # 1 .. weight[total_span]
        for span in range(0,total_span): 
            if(r <= weight[span]):
                break
        problem.task[i].starting_date = int(random.randint(0,2^30-1) % (total_span - span + 1))
        problem.task[i].ending_date = int(problem.task[i].starting_date + span - 1)
        #
        # task profit
        #
        # the task profit is given by r*task_span, where r is a random variable in the range 50..300 with a probability
        #   density function with shape (two triangles, the area of the second is 4 times the area of the first)
        #
        #      *
        #     /|   *
        #    / |       *
        #   /  |           *
        #  *---*---------------*
        # 50 100 150 200 250 300
        #
        scale = random.randint(0,2^30-1) % 12501 # almost uniformly distributed in 0..12500 
        if(scale <= 2500):
            problem.task[i].profit = 1 + round(span * (50.0 + math.sqrt(scale)))
        else:
            problem.task[i].profit = 1 + round(span * (300.0 - 2.0 * math.sqrt(12500 - scale)))
    #
    # sort the tasks by the starting date
    #
    quickSort(problem.task,0,problem.T-1)
    #
    #finish
    #
    if(problem.I != 0):
        for i in range(0,problem.T):
            problem.task[i].profit = 1

    dir = "{nmec:06d}".format(nmec = NMec)
    if(len(dir) >= 16):
        print("Directory name too large!\n")
        exit(1)
    else:
        problem.dir_name = dir

    file = "{nmec:06d}/{t:02d}_{p:02d}_{i:d}.txt".format(nmec = NMec, t = T, p = P, i = problem.I)
    if(len(file) >= 64):
        print("File name too large!\n")
        exit(1)
    else:
        problem.file_name = file

    

##########################################################################################################################################################
#
#
#problem solution (place your solution here)
#
#

def recurse(prob,t):
    if t>=prob.T:
        prob.casos=prob.casos+1
        if prob.total_profit > prob.biggest_profit:
            prob.biggest_profit=prob.total_profit
            for j in range(0,prob.T):
                prob.task[j].best_assigned_to=prob.task[j].assigned_to
        return True

    recurse(prob,t+1)

    for p in range(0,prob.P):
        if (prob.busy[p] < prob.task[t].starting_date and prob.task[t].assigned_to < 0):
            busy_save = prob.busy[p]
            task_save = prob.task[t].assigned_to
            profit_save = prob.total_profit

            prob.busy[p] = prob.task[t].ending_date
            prob.task[t].assigned_to = p
            prob.total_profit += prob.task[t].profit
            recurse(prob,t+1)
            prob.busy[p] = busy_save
            prob.task[t].assigned_to = task_save
            prob.total_profit = profit_save
            break
    return 0

def nonRec(problem,programmer):
    for t in range(0,problem.T):
        if problem.task[t].starting_date > problem.busy[programmer] and problem.task[t].best_assigned_to == -1:
            problem.busy[programmer] = problem.task[t].ending_date
            problem.biggest_profit = problem.biggest_profit + problem.biggest_profit
            problem.task[t].best_assigned_to = programmer
            break
    return 1


def solve(problem):
    os.mkdir(problem.dir_name)
    fp = open(problem.file_name,"w")
    #
    # solve
    #
    problem.cpu_time = time.time() #IMPORTANT: THIS NEEDS CHECKING AND MAYBE PROCESSING TO BE IN SAME FORMAT AS THE C SCRIPT
    for t in range(0,problem.T):
        problem.task[t].assigned_to = -1
        problem.task[t].best_assigned_to = -1
    for p in range(0,problem.P):
        problem.busy[p] = -1
    #undef PROG        
    problem.casos = 0
    problem.biggest_profit = 0
    problem.total_profit = 0
    
    if(problem.I ==1 and problem.P ==1):
        fp.write("nrecurse\n")
        quickSortE(problem.task,0,problem.T-1)
        nonRec(problem, p)
    else:
        fp.write("recurse\n")
        recurse(problem, 0)        
    problem.cpu_time = time.time() - problem.cpu_time      
    #
    # save solution data
    #
    for t in range(0,problem.T):
        fp.write("P{:d}\t{:d} T{:d} {:d} \n".format(problem.task[t].best_assigned_to,problem.task[t].starting_date,t,problem.task[t].ending_date)) 
    fp.write("NMec = {nmec:d}\n".format(nmec=problem.NMec))
    fp.write("Viable Sol. = {casos:d}\n".format(casos=problem.casos))
    fp.write("Profit = {bprofit:d}\n".format(bprofit=problem.biggest_profit))
    fp.write("T = {t:d}\n".format(t=problem.T))
    fp.write("P = {p:d}\n".format(p=problem.P))
    if(problem.I):
        fp.write("Profits ignored\n")   
    else:
        fp.write("Profits not ignored\n")
    fp.write("Solution time = {:3e}\n".format(problem.cpu_time))
    fp.write("Task data\n")
    for i in range(0,problem.T):    
        fp.write("{:02d}  {:3d} {:3d} {:5d}\n".format(i,problem.task[i].starting_date,problem.task[i].ending_date,problem.task[i].profit))
    fp.write("End\n")
    #
    #terminate
    #
       
def main():
    problem = problem_t()
    argc = len(sys.argv)
    #NMec
    if(argc < 2):
        NMec = 2020
    else:
        NMec = int(sys.argv[1])
    #T
    if(argc < 3):
        T = 5
    else:
        T = int(sys.argv[2])
    #P
    if(argc < 4):
        P = 2
    else:
        P = int(sys.argv[3])
    #I
    if(argc < 5):
        I = 0
    else:
        I = int(sys.argv[4])
    init_problem(NMec,T,P,I,problem) 
    solve(problem)
    return 0   
main()

