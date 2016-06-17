package server;

import java.io.IOException;
import java.net.InetAddress;
import java.net.ServerSocket;
import java.net.Socket;
import java.net.UnknownHostException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

import rasenderroboter.Game;
import rasenderroboter.Player;
import util.Logger;

public class ThreadPooledServer {
	// Default BACKLOG for the server socket
	final private static int BACKLOG = 50;
	
	// Default port defines in the subject
	final private static int PORT = 2016;
	
	// Nb max threads
    final private static int NB_THREADS = 1000;
    
    // Server scoket
	private ServerSocket serverSocket;
	
	// Thread pool
	private ExecutorService threadPool = Executors.newFixedThreadPool(NB_THREADS);
	
	// Game engine
	final private Game gameEngine = new Game();
	
	public ThreadPooledServer(String addr) throws UnknownHostException, IOException {
		InetAddress inetAddr = InetAddress.getByName(addr);
		
	    // Init server socket
		serverSocket = new ServerSocket(PORT, BACKLOG, inetAddr);
		serverSocket.setReuseAddress(true);
		
		Logger.forceLogln("Server starts.");
	}
	
	public void run() {
	    new Thread(gameEngine).start();
	    boolean end = false;
	    
		// Main loop to receive connection
		do {
			// Waiting for a connection
			Socket clientSocket = null;
			try {
				clientSocket = serverSocket.accept();
			}
			catch (IOException e) {
				e.printStackTrace();
			}
			
			// Create a new player and "execute it"
			Player player;
			player = new Player(clientSocket, gameEngine);
			
            threadPool.execute(player);
		} while (!end);
		
		threadPool.shutdown();
	}
}
