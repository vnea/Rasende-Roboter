package util;

public class Logger {
	private static boolean log = true;
	
	public static void log(String msg) {
		if (log) {
			System.out.print(msg);
		}
	}
	
	public static void logln(String msg) {
		if (log) {
			System.out.println(msg);
		}
	}

	public static void forceLog(String msg) {
		System.out.print(msg);
	}
	
	public static void forceLogln(String msg) {
		System.out.println(msg);
	}
	
	public static void enableLogger(boolean on) {
		log = on;
	}
}
