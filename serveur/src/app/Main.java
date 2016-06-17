package app;

import java.io.IOException;
import java.net.UnknownHostException;

import server.ThreadPooledServer;
import util.Logger;

public class Main {
	public static void main(String[] args) throws IOException {
		// Address must be specify
		if (args.length == 1) {
			try {
				// Create and launch server
				ThreadPooledServer tps = new ThreadPooledServer(args[0]);
				tps.run();
			}
			// Invalid address
			catch (UnknownHostException e) {
				Logger.forceLogln("The follow address is invalid : " + args[0]);
			}
			catch (IOException e) {
				e.printStackTrace();
			}
		}
		else {
			Logger.forceLogln("Usage: ant run adress");
		}
	}
}
