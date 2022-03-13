package java_solution;

public class Task implements Comparable<Task> {
	private int starting_date;
	private int ending_date;
	private int profit;
	private int assigned_to;
	private int bestAssignedTo;
	
	
	public Task(int starting_date, int ending_date,
			int profit, int assigned_to, int best_assigned_to) {
		this.starting_date = starting_date;
		this.ending_date = ending_date;
		this.profit = profit;
		this.assigned_to = assigned_to;
		this.bestAssignedTo = best_assigned_to;
	}

	public Task() {
		this.starting_date = -1;
		this.ending_date = -1;
		this.profit = -1;
		this.assigned_to = -1;
		this.bestAssignedTo = -1;
	}
	
	
	
	public int getAssigned_to() {
		return assigned_to;
	}
	public void setAssigned_to(int assigned_to) {
		this.assigned_to = assigned_to;
	}
	public int getBest_assigned_to() {
		return bestAssignedTo;
	}
	public void setBest_assigned_to(int best_assigned_to) {
		this.bestAssignedTo = best_assigned_to;
	}
	public int getStarting_date() {
		return starting_date;
	}
	public int getEnding_date() {
		return ending_date;
	}
	public int getProfit() {
		return profit;
	}
	public void setStartingDate(int starting_date) {
		this.starting_date =  starting_date;
	}
	public void setEndingDate(int ending_date) {
		this.ending_date = ending_date ;
	}
	public void setProfit(int profit) {
		this.profit = profit;
	}
	@Override
	public int compareTo(Task t2) {
		return 0;
	}
	public int compare(Task t2) {
		int thisstart = this.getStarting_date();
		int t2start = t2.getStarting_date();
		if(thisstart != t2start) return thisstart < t2start ? -1 : 1;
		int thisending = this.getEnding_date();
		int t2ending = t2.getEnding_date();
		if(thisending != t2ending) return thisending < t2ending ? -1 : 1;
		return 0;
	}
	public boolean isAvailable() {
		return this.assigned_to < 0;
		}
	public boolean isAvailableNumberOfTasks() {
		return this.bestAssignedTo < 0;
		}

}