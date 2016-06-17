package rasenderroboter;


import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.io.PrintWriter;
import java.net.Socket;
import java.util.List;

import util.CommandParser;
import enumeration.CommandCS;


public class Player implements Runnable {
    // Username
    private String username;
    
    // Score of the player
    private int score = 0;
    
    // Its current bid
    private int bid = 0;
    
    // The game engine
    final private Game gameEngine;
    
    // Its socket
    private Socket socket;
    
    // To receive/send
    private BufferedReader br;
    private PrintWriter pw;
    
    public Player(Socket socket, final Game gameEngine) {
        this.socket = socket;

        this.gameEngine = gameEngine;         
        
        try {
            br = new BufferedReader(
                    new InputStreamReader(this.socket.getInputStream()));
            
            pw = new PrintWriter(new BufferedWriter(
                    new OutputStreamWriter(this.socket.getOutputStream())),
                    // Enable autoflush
                    true); 
        }
        catch (IOException e) {
            e.printStackTrace();           
        }      
    }
    
    /*
     * Return the score
     */
    public int getScore() {
        return score;
    }
    
    /*
     * Increment the score by one
     */
    public void increaseScore() {
        ++score;
    }
    
    /*
     * Reset the score to 0
     */
    public void resetScore() {
        score = 0;
    }
    
    /*
     * Return the bid
     */
    public int getBid() {
        return bid;
    }

    /*
     * Set a bid
     */
    public void setBid(int bid) {
        this.bid = bid;
    }
    
    /*
     * Reset the bid to 0
     */
    public void resetBid() {
        setBid(0);
    }
    
    /*
     * Return the username
     */
    public String getUsername() {
        return username;
    }
    
    /*
     * If it is the first bid
     */
    public boolean firstBid() {
        return bid == 0;
    }
    
    @Override
    public void run() {
        try {
            List<String> command;
            CommandCS commandCS;
            
            // Wait for the CONNEXION/ message
            do {
                command = receiveCommandFromClient();
                commandCS = CommandCS.valueOf(command.get(0));
            } while (commandCS != CommandCS.CONNEXION);
    
            
            // Update username
            username = command.get(1);
            
            synchronized (gameEngine) {
                gameEngine.addPlayer(this);
            }
            
            boolean wantsToDisconnect = false;
            // Main loop
            do {
                // Receive a command
                command = receiveCommandFromClient();
                
                if (command != null) {
	                // Get the first part of the command
	                commandCS = CommandCS.valueOf(command.get(0));
	                
	                // The player wants to disconnect
	                wantsToDisconnect = commandCS == CommandCS.SORT;
	    
	                if (!wantsToDisconnect) {
	                    synchronized (gameEngine.getPhase()) {
	                        switch (gameEngine.getPhase()) {
	                            case REFLECTION:
	                                if (commandCS == CommandCS.SOLUTION) {
	                                    int submittedBid = Integer.parseInt(command.get(2));
	                                    gameEngine.manageReflectionPhase(this, submittedBid);
	                                }
	                            break;
	                                
	                            case BID:
	                                if (commandCS == CommandCS.ENCHERE) {
	                                    int submittedBid = Integer.parseInt(command.get(2));
	                                    gameEngine.manageBidPhase(this, submittedBid);
	                                }
	                            break;
	                             
	                            case RESOLUTION:
	                                if (commandCS == CommandCS.SOLUTION) {
	                                    String shiftingsSubimitted = command.get(2);
	                                    gameEngine.manageResolutionPhase(this, shiftingsSubimitted);
	                                }
	                            break;
	                        }
	                    }
	                }
                }
                // command == null means that's the client left the game without
                // sending SORT/user
                else {
                	wantsToDisconnect = true;
                }
            } while (!wantsToDisconnect);
        }
        catch (IOException e) {
        }
        
        // The player leaves the game
        synchronized (gameEngine) {
            gameEngine.removePlayer(this);
        }
        
        try {
            socket.close();
        }
        catch (IOException e) {
            e.printStackTrace();
        }
    }
    
    /*
     * Send a command to the client
     */
    public void sendCommandToClient(String ...args) {
        StringBuilder sb = new StringBuilder();
        
        for (String arg : args) {
            sb.append(arg);
            sb.append(CommandParser.SEPARATOR);
        }
        
        pw.println(sb);
    }
    
    /*
     * Receive a command from the client
     */
    private List<String> receiveCommandFromClient() throws IOException {
    	return CommandParser.getCommands(br.readLine());
     }
    
    @Override
    public String toString() {
        return "(" + username + "," + score + ")"; 
    }
}
