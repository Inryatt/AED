package java_solution;

public class Problem_T {
  int NMec;               // I  student number
  int T;                  // I  number of tasks
  int P;                  // I  number of programmers
  int I;                  // I  if 1, ignore profits
  int total_profit;       // S  current total profit
  int biggestP;       // S  current total profit
  double cpu_time;        // S  time it took to find the solution
  Task[] task = new Task[64];     // IS task data
  Programador[] prog = new Programador[64];
	String dir_name;      // I  directory name where the solution file will be created
  String file_name;     // I  file name where the solution data will be stored
  int casos;
  
  
  
  
  public Problem_T() {
		super();
		NMec = -1;
		T = -1;
		P = -1;
		I = -1;
		this.total_profit = 0;
		this.biggestP = 0;
		this.cpu_time = 0;
		this.casos = 0;
	}
	public int getNMec(){
		return NMec;
	}
	public void setNMec(int nMec) {
		NMec = nMec;
	}
	public int getT() {
		return T;
	}
	public void setT(int t) {
		T = t;
	}
	public int getP() {
		return P;
	}
	public void setP(int p) {
		P = p;
	}
	public int getI() {
		return I;
	}
	public void setI(int i) {
		I = i;
	}
	public int getTotal_profit() {
		return total_profit;
	}
	public void setTotal_profit(int total_profit) {
		this.total_profit = total_profit;
	}
	public void sumTotal_profit(int total_profit) {
		this.total_profit += total_profit;
	}
	public void sumBiggest_profit(int tp) {
		this.biggestP += tp;
	}
	public int getBiggestP() {
		return biggestP;
	}
	public void setBiggestP(int biggestP) {
		this.biggestP = biggestP;
	}
	public double getCpu_time() {
		return cpu_time;
	}
	public void setCpu_time(double cpu_time) {
		this.cpu_time = cpu_time;
	}
	public Task[] getTask() {
		return task;
	}
	public void setTask(Task[] task) {
		this.task = task;
	}
	public String getDir_name() {
		return dir_name;
	}
	public void setDir_name(String dir_name) {
		this.dir_name = dir_name;
	}
	public String getFile_name() {
		return file_name;
	}
	public void setFile_name(String file_name) {
		this.file_name = file_name;
	}
	public int getCasos() {
		return casos;
	}
	public void incCasos() {
		this.casos = casos + 1;
	}
	public void setCasos(int casos) {
		this.casos = casos;
	}
	public boolean isBetter() {
		return this.total_profit > this.biggestP;
	}
	
}