package java_solution;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.Arrays;
import java.util.Iterator;
import java.util.List;

public class Main {
  static int tail = 100;
  static int sum1 = 298;
  static int MAX_T = 64;
  static int MAX_P = 10;

  public static void main(String argv[]) {
    // "/home/whoknows/Documents/AED/A01/002020/05_02_0.txt"
    Problem_T problem = new Problem_T();
    for (int i = 0; i < MAX_T; i++) {
      problem.task[i] = new Task();
      if (i < 10)
        problem.prog[i] = new Programador();
    }
    Integer NMec, T, P, I;
    int argc = argv.length;
    NMec = (argc < 2) ? 2020 : Integer.parseInt(argv[0]);
    T = (argc < 3) ? 5 : Integer.parseInt(argv[1]);
    P = (argc < 4) ? 2 : Integer.parseInt(argv[2]);
    I = (argc < 5) ? 0 : Integer.parseInt(argv[3]);
    init_problem(NMec, T, P, I, problem);
    try {
      solve(problem);
    } catch (IOException e) {
      e.printStackTrace();
    }

  }

  public static void init_problem(Integer nmec, Integer T, Integer P, Integer I, Problem_T problem) {

    // int i,r,scale,span,total_span;
    //
    // input validation
    //

    if (nmec < 1 || nmec > 999999) {
      System.err.print(String.format("Bad NMec (1 <= NMex (%d) <= 999999)\n", nmec));
      System.exit(-1);
    }
    if (T < 1 || T > MAX_T) {
      System.err.print(String.format("Bad T (1 <= T (%d) <= %d)\n", T, MAX_T));
      System.exit(-1);
    }
    if (P < 1 || P > MAX_P) {
      System.err.print(String.format("Bad P (1 <= P (%d) <= %d)\n", P, MAX_P));
      System.exit(-1);
    }
    problem.setNMec(nmec);
    problem.setP(P);
    problem.setT(T);
    problem.setI(I);
    List<String> fps = null; // initialize a list
    try {

      // MUST CHANGE
      // tem de ser o ficheiro que o setor ja gerou mais as alteracoes que eu fiz
      // para dar para fazer os graficos
      fps = Files.readAllLines(Paths.get(String.format("/home/inryatt/AED/AED/A01/t%06d/task_%06d_%02d_%02d_%02d.txt",
          problem.getNMec(),problem.getNMec(), problem.getT(), problem.getP(), problem.getI())));// read file
    } catch (IOException e1) {
      System.err.println("ERRO");
      e1.printStackTrace();
      System.exit(-1);
    }

    int c = 0;

    for (int i = 0; i < fps.size(); i += 3) {
      //
      // public Task(int starting_date, int ending_date,
      // int profit, int assigned_to, int best_assigned_to) {
      problem.task[c] = new Task(Integer.parseInt(fps.get(i)), Integer.parseInt(fps.get(i + 1)),
          Integer.parseInt(fps.get(i + 2)), -1, -1);
      c++;
    }

    Arrays.sort(problem.getTask());
  }

  public static void solve(Problem_T problem) throws IOException {
    Path current_path = Paths.get("" + System.getProperty("user.dir") + String.format("/%06d", problem.getNMec()));
    File fp = null;
    fp = new File(current_path.toFile(),
        String.format("%02d_%02d_%d.txt", problem.getT(), problem.getP(), problem.getI()));
    System.out.println(fp + " path " + current_path + "current path" + System.getProperty("user.dir"));
    fp.getParentFile().mkdirs();
    System.out.println(fp.createNewFile());
    FileWriter fpStream = null;
    try {
      fpStream = new FileWriter(fp, false); // true to append
    } catch (Exception e) {
      System.err.println("Something went wrong while opening files");
      System.out.println(e.getMessage());
      System.exit(-1);
    }
    // solve problem
    final long startTime = System.nanoTime();

    // place solution

    for (int t = 0; t < problem.T; t++) {
      problem.task[t].setAssigned_to(-1);
      problem.task[t].setBest_assigned_to(-1);
    }

    for (int p = 0; p < problem.P; p++)
      problem.prog[p].setBusy_until(-1);

    problem.setCasos(0);
    problem.setBiggestP(0);
    problem.setTotal_profit(0);

    Arrays.sort(problem.task);
    if (problem.I != 1 && problem.P == 1) {
      System.out.println("nrecurse");
      nonRec(problem, 0);
    } else {
      System.out.println("recurse");
      recurse(problem, 0);
    }

    // endind time
    final long endingTime = System.nanoTime();
    problem.setCpu_time((double) (endingTime - startTime) / 1000000000);
    //
    // save solution data
    //
    for (int t = 0; t < problem.T; t++) {
      fpStream.write(String.format("P%d\t%d T%d %d \n", problem.task[t].getBest_assigned_to(),
          problem.task[t].getStarting_date(), t, problem.task[t].getEnding_date()));
    }
    fpStream.write(String.format("NMec = %d\n", problem.getNMec()));
    fpStream.write(String.format("Viable Sol. = %d\n", problem.getCasos()));
    fpStream.write(String.format("Profit = %d\n", problem.getBiggestP()));
    fpStream.write(String.format("T = %d\n", problem.getT()));
    fpStream.write(String.format("P = %d\n", problem.getP()));
    fpStream.write(String.format("Profits%s ignored\n", (problem.getI() == 0) ? " not" : ""));
    fpStream.write(String.format("Solution time = %.3e\n", problem.getCpu_time()));
    fpStream.write(String.format("Task data\n"));

    for (int i = 0; i < problem.T; i++)
      fpStream.write(String.format("%02d  %3d %3d %5d\n", i, problem.task[i].getStarting_date(),
          problem.task[i].getEnding_date(), problem.task[i].getProfit()));

    fpStream.write(String.format("End\n"));
    //
    // terminate
    //
    fpStream.close();

  }

  private static int recurse(Problem_T prob, int t) {

    int busy_save;
    int profit_save;
    int task_save;
    if (t >= prob.getT()) {
      prob.incCasos();
      if (prob.isBetter()) {
        prob.setBiggestP(prob.getTotal_profit());
        for (int j = 0; j < prob.getT(); j++) {
          prob.task[j].setBest_assigned_to(prob.task[j].getAssigned_to());
        }
      }
      return 1;
    }

    recurse(prob, t + 1);

    for (int p = 0; p < prob.getP(); p++) {
      if (prob.prog[p].canDo(prob.task[t].getStarting_date()) && prob.task[t].isAvailable()) {
        busy_save = prob.prog[p].getBusy_until();
        task_save = prob.task[t].getAssigned_to();
        profit_save = prob.getTotal_profit();

        prob.prog[p].setBusy_until(prob.task[t].getEnding_date());
        prob.task[t].setAssigned_to(p);
        prob.sumTotal_profit(prob.task[t].getProfit());
        recurse(prob, t + 1);
        prob.prog[p].setBusy_until(busy_save);
        prob.task[t].setAssigned_to(task_save);
        prob.setTotal_profit(profit_save);
        break;
      }
    }
    return 0;
  }

  private static int nonRec(Problem_T problem, int g) {
    int t = 0;
    for (t = 0; t < problem.getT(); t++) {
      if (problem.prog[g].canDoNumberOfTasks(problem.task[t].getStarting_date())
          && problem.task[t].isAvailableNumberOfTasks()) {
        problem.prog[g].setBusy_until(problem.task[t].getEnding_date());
        problem.sumBiggest_profit(problem.task[t].getProfit());
        problem.task[t].setBest_assigned_to(g);
      }
    }
    return 1;
  }

}