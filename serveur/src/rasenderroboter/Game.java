package rasenderroboter;

import java.io.File;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Random;
import java.util.Set;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import enumeration.CommandSC;
import enumeration.Phase;
import exception.InvalidShiftingRobotException;
import util.Logger;

public class Game implements Runnable {
    // Time of each phase
    final private static long TIME_REFLECTION_PHASE = 30000;
    final private static long TIME_BID_PHASE = 30000;
    final private static long TIME_RESOLUTION_PHASE = 30000;
    
    // Folder of all boards
    final private static String FOLDER_BOARDS_PATH = "res/boards/";
    final private static File folderBoards = new File(FOLDER_BOARDS_PATH);
    
    // Random to select a random file
    private Random random = new Random();
    
    // Number of minimum players
    final static private int NB_MIN_PLAYERS = 2;
    
    // Board of the game
    private Board board = new Board();
    
    // Session is currently ruuning = there is at least NB_MIN_PLAYERS min players
    private volatile Boolean runningSession = false;
    
    // List of all players
    private List<Player> players = new ArrayList<>();
    
    // Players who submitted a bid
    private Set<Player> playersWhichSubmittedABid = new HashSet<>();
    
    // Players in current turn
    private Set<Player> playersInCurrentTurn = new HashSet<>();
    
    // Current player for the resolution phase
    private Player currentPlayer = null;

    // Turn of the game
    private int turn = 1;
    
    // Phase of the game
    private volatile Phase phase = null;
    
    // Check if a bid has been done during the reflection phase
    private Boolean bidSubmittedInReflectionPhase = false;
    
    // Check
    private Boolean solutionSubmittedInResolutionPhase = false;
    
    // Allow a player to join a turn or not
    private Boolean playersAllowToJoinTurn = true;
    
    public Game() {
    	Logger.enableLogger(false);
        createNewSession();
    }
    
    /*
     * Load an enigma 
     */
    public void loadRandomEnigma() {
        board.loadEnigma();
    }
    
    /*
     * Add a new player
     */
    public void addPlayer(Player player) {
        Logger.logln("Player " + player.getUsername() + " joins the game.");

        // Add the player
        players.add(player);
                
        // Send : BIENVENUE/user/
        player.sendCommandToClient(CommandSC.BIENVENUE.name(), player.getUsername());
        
        // Send : CONNECTE/user/
        sendCommandToOtherPlayers(player, CommandSC.CONNECTE.name(), player.getUsername());
        
        // Send : SESSION/plateau/
        player.sendCommandToClient(CommandSC.SESSION.name(), board.strBoard());
        
        // Allow him to join the turn or not
        synchronized (playersAllowToJoinTurn) {
            if (playersAllowToJoinTurn) {
                playersInCurrentTurn.add(player);
            }
        }
        
        // Update flag runningSession 
        runningSession = players.size() >= NB_MIN_PLAYERS;
        Logger.logln("runningSession = " + Boolean.toString(runningSession));
    }
    
    /*
     * Remove a player
     */
    public void removePlayer(Player player) {
        Logger.logln("Player " + player.getUsername() + " leaves the game.");
        sendCommandToAllPlayers(CommandSC.DECONNEXION.name(), player.getUsername());
        
        // Remove the player
        players.remove(player);
        playersWhichSubmittedABid.remove(player);
        playersInCurrentTurn.remove(player);
        
        // Update the flag runningSession 
        if (players.size() < NB_MIN_PLAYERS || playersInCurrentTurn.size() < NB_MIN_PLAYERS) {
            runningSession = false;
        }
        
        if (playersInCurrentTurn.size() == 1) {
            // There is one player left, he is the winner
        	Iterator<Player> it = playersInCurrentTurn.iterator();
        	
        	Player winner = (Player) it.next();
        	winner.sendCommandToClient(CommandSC.VAINQUEUR.name(), strAssessment());
        	
        	Logger.logln(winner.getUsername());
            
            synchronized (playersAllowToJoinTurn) {
                playersAllowToJoinTurn = true;
                
                playersInCurrentTurn.clear();
                playersWhichSubmittedABid.clear();
                
                for (Player p : players) {
                    playersInCurrentTurn.add(p);
                }
            }
            
            createNewSession();
            sendCommandToAllPlayers(CommandSC.SESSION.name(), board.strBoard());
        }
    }
    
    /*
     * Find the current player for the resolution phase
     */
    private boolean findNewCurrentPlayer() {
        boolean found = false;
        Iterator<Player> it = playersWhichSubmittedABid.iterator();
        currentPlayer = null;
        
        // Define a first current player
        if (it.hasNext()) {
            found = true;
            currentPlayer = (Player) it.next();
        }
        
        // Check if another can be a valid candidate
        while (it.hasNext()) {
            Player player = (Player) it.next();
            
            if (player.getBid() < currentPlayer.getBid() ) {
                found = true;
                currentPlayer = player;
            }
        }
        
        return found;
    }
    
    /*
     * Create a new session
     */
    private void createNewSession() {
        turn = 1;
        phase = null;
        currentPlayer = null;
        
        bidSubmittedInReflectionPhase = false;

        solutionSubmittedInResolutionPhase = false;
        
        // Reset the score for each player
        for (Player player :  players) {
            player.resetScore();
        }
        
        // Load new board
        loadRandomBoard();
    }
    
    /*
     * Load a random board
     */
    public void loadRandomBoard() {
       int numFile = random.nextInt(folderBoards.list().length);
       board.load(FOLDER_BOARDS_PATH + folderBoards.list()[numFile].toString());
    }
    
    /*
     * Return the phase
     */
    public Phase getPhase() {
    	return phase;
    }
    
    /*
     * Go to the next turn of the current session
     */
    public void nextTurn(CommandSC commandSC) {
        // No current player anymore
        currentPlayer = null;
        
        // We're not in a phase anymore
        phase = null;

        playersWhichSubmittedABid.clear();
        
        // Go to next turn
        ++turn;
        
        // Reset bid for all players
        for (Player player : players) {
            player.resetBid();
        }
        
        Logger.logln(commandSC.toString());
   
        // Send : BONNE/ or FINRESO/
        sendCommandToAllPlayersInCurrentTurn(commandSC.name());
    }

    /*
     * Check if a player submitted a bid equal to another bid of a player
     */
    public boolean isPlayerBidEqualToAnotherBid(Player player, int currentBid) {
        boolean playerBidEqualToAnotherBid = false;
        for (int i = 0; i < players.size() && !playerBidEqualToAnotherBid; ++i) {
            if (players.get(i) != player) {
                playerBidEqualToAnotherBid = currentBid == players.get(i).getBid();
            }
        }
        
        return playerBidEqualToAnotherBid;
    }
    
    /*
     * Check if a solution is valid
     */
    private boolean isSolutionValid(String shiftings, int expectedNbMoves) {
    	// Check we have a pair number
    	boolean isSolutionValid = shiftings.length() % 2 == 0;
    	
    	// Get the number of moves
        final int NB_MOVES = shiftings.length() / 2;
        
        // Solution can be accepted only if lower than expectedNbMoves
        isSolutionValid = NB_MOVES <= expectedNbMoves;
        if (isSolutionValid) {
            Pattern pattern = Pattern.compile("([A-Z]{2})");
            Matcher matcher = pattern.matcher(shiftings);
            
            List<ShiftingRobot> shiftingsRobots = new ArrayList<>(NB_MOVES);
            
            try {
	            while (matcher.find()) {
	                String shifting = matcher.group(0);
	                shiftingsRobots.add(new ShiftingRobot(shifting.substring(0, 1),
	                                                      shifting.substring(1, 2)));
	            }
	            
	            isSolutionValid = board.isSolutionValid(shiftingsRobots);
            }
            // Exception thrown if invalid shifting receive
            catch (InvalidShiftingRobotException e) {
            	e.printStackTrace();
            	isSolutionValid = false;
            }
        }
        
        return isSolutionValid;
    }
    
    /*
     * Return the winner of the session
     */
    public Player getWinner() {
    	Player winner = null;
    	boolean winnerFound = false;
    	
    	Iterator<Player> it = playersInCurrentTurn.iterator();
    	while (it.hasNext() && !winnerFound) {
    	    Player player = (Player) it.next();
            if (winnerFound = player.getScore() == board.getObjectiveScore()) {
                winner = player;
            }
    	}
    	
    	return winner;
    }

    /*
     * Wait until end of reflection phase
     */
    private void waitEndOfReflectionPhase() {
    	bidSubmittedInReflectionPhase = false;
        long currentTime = System.currentTimeMillis();
        while (phase == Phase.REFLECTION && System.currentTimeMillis() - currentTime < TIME_REFLECTION_PHASE && runningSession && !bidSubmittedInReflectionPhase) {
        }
    }
    
    /*
     * Wait until end of bid phase
     */
    private void waitEndOfBidPhase() {
        long currentTime = System.currentTimeMillis();
        while (phase == Phase.BID && System.currentTimeMillis() - currentTime < TIME_BID_PHASE && runningSession) {
        }
    }
    
    /*
     * Wait until end of resolution phase
     */
    private void waitEndOfResolutionhase() {
    	synchronized (solutionSubmittedInResolutionPhase) {
        	solutionSubmittedInResolutionPhase = false;
		}
        long currentTime = System.currentTimeMillis();
        while (phase == Phase.RESOLUTION && System.currentTimeMillis() - currentTime < TIME_RESOLUTION_PHASE && runningSession && !solutionSubmittedInResolutionPhase) {
        }
    }
    
    /*
     * Player plays in reflection phase
     */
    public void manageReflectionPhase(Player player, int bid) {
        if (phase == Phase.REFLECTION && runningSession && playersInCurrentTurn.contains(player)) {
            phase = Phase.BID;
            
            bidSubmittedInReflectionPhase = true;
            playersWhichSubmittedABid.add(player);
            player.setBid(bid);
            
            // Send : ILATROUVE/user/coups/
            sendCommandToOtherPlayersInCurrentTurn(player, CommandSC.ILATROUVE.name(), player.getUsername(), Integer.toString(bid));
            
            // Send : TUASTROUVE/
            player.sendCommandToClient(CommandSC.TUASTROUVE.name());
        }
    }
    
    /*
     * Player plays in bid phase
     */
    public void manageBidPhase(Player player, int bid) {
        if (phase == Phase.BID && runningSession && playersInCurrentTurn.contains(player)) {
            // The player is playing
            playersWhichSubmittedABid.add(player);
            
            // Case of the first bid of a player
            boolean firstBid = player.firstBid();
            
            // We need to check if the submitted bid is not the same as the bid of another player
            boolean isPlayerBidEqualToAnotherBid = isPlayerBidEqualToAnotherBid(player, bid);
            
            boolean isBidValid = firstBid && !isPlayerBidEqualToAnotherBid;
            if (isBidValid) {
                // We can store the submitted bid
                player.setBid(bid);
            }
            
            if (!firstBid) {
                // The submitted bid must be less than the last submitted bid
                isBidValid = bid < player.getBid() && !isPlayerBidEqualToAnotherBid;
                
                if (isBidValid) {
                    player.setBid(bid);
                }
            }
            
            // Submitted bid has been accepted
            if (isBidValid) {
                // Send : NOUVELLEENCHERE/user/coups/
                sendCommandToOtherPlayersInCurrentTurn(player, CommandSC.NOUVELLEENCHERE.name(), player.getUsername(), Integer.toString(bid));  
                
                // Send : VALIDATION/
                player.sendCommandToClient(CommandSC.VALIDATION.name());
            }
            else {
                // Send : ECHEC/
                player.sendCommandToClient(CommandSC.ECHEC.name(), player.getUsername());
            }
        }
    }
    
    /*
     * Player plays in resolution phase
     */
    public void manageResolutionPhase(Player player, String shiftings) {
        if (phase == Phase.RESOLUTION && runningSession && player == currentPlayer && playersInCurrentTurn.contains(player)) {
        	        	
            // Send : SASOLUTION/user/deplacements/
            sendCommandToOtherPlayersInCurrentTurn(currentPlayer, CommandSC.SASOLUTION.name(), currentPlayer.getUsername(), shiftings);
            
            synchronized (solutionSubmittedInResolutionPhase) {
                // If the submitted solution is valid
                if (isSolutionValid(shiftings, currentPlayer.getBid())) {
                    currentPlayer.increaseScore();   
                    
                    // Next turn + Send : BONNE/
                    nextTurn(CommandSC.BONNE);
                }
                else {
                    // Can't play anymore (for this turn)
                    playersWhichSubmittedABid.remove(currentPlayer);
                    if (findNewCurrentPlayer()) {
                        // Send : MAUVAISE/user/
                        sendCommandToAllPlayersInCurrentTurn(CommandSC.MAUVAISE.name(), currentPlayer.getUsername());
                    }
                    else {
                        // Send : FINRESO/
                        nextTurn(CommandSC.FINRESO);
                    }
                }
                
                solutionSubmittedInResolutionPhase = true;
            }
        }
    }
    
    /*
     * Run
     */
    @Override
    public void run() {
        do {
            while (runningSession) {
                do {
                    // Next player will have to wait for the next turn to join the game
                    synchronized (playersAllowToJoinTurn) {
                        playersAllowToJoinTurn = false;
                    }
                    
                    Logger.logln("Start of turn.");
                    
                    if (runningSession) {
	                    // Load a new enigma
	                    loadRandomEnigma();
	                    
	                    Logger.logln(board.strFullBoard());
	                    
	                    // Send : TOUR/enigme/bilan/
	                    sendCommandToAllPlayersInCurrentTurn(CommandSC.TOUR.name(), board.strEnigma(), strAssessment());
                    }
                    
                    phase = Phase.REFLECTION;
        
                    // Reflection phase
                    Logger.logln("\nWaiting for end of " + Phase.REFLECTION + " phase.");
                    waitEndOfReflectionPhase();
                    Logger.logln("End of " + Phase.REFLECTION + " phase.");
    
                    
                    // No bidSubmittedInReflectionPhase = Time (TIME_REFLECTION_PHASE) to receive a bid has expired
                    synchronized (bidSubmittedInReflectionPhase) {
                        if (runningSession && !bidSubmittedInReflectionPhase) {
                            Logger.logln("Bid not done");
                            
                            phase = Phase.BID;
                            
                            // Send : FINREFLEXION/
                            sendCommandToAllPlayersInCurrentTurn(CommandSC.FINREFLEXION.name());
                        }
                    }
                    
                    // Bid phase
                    Logger.logln("\nWaiting for end of " + Phase.BID + " phase.");
                    waitEndOfBidPhase();
                    Logger.logln("End of " + Phase.BID + " phase.\n");
    
                    
                    // Resolution phase
                    phase = Phase.RESOLUTION;
                    if (runningSession) {
                        // findNewCurrentPlayer = true -> a player submitted a bid
                        if (findNewCurrentPlayer()) {
                            Logger.logln("\nCurrent player found.");
        
                            if (runningSession) {
                                // Send : FINENCHERE/user/coups/
                                sendCommandToAllPlayersInCurrentTurn(CommandSC.FINENCHERE.name(), currentPlayer.getUsername(), Integer.toString(currentPlayer.getBid()));
                            }
                
                            // Wait until resolution phase ends
                            while (runningSession && phase == Phase.RESOLUTION && findNewCurrentPlayer()) {
                                // Wait until the current player submits a solution
                            	waitEndOfResolutionhase();

                                synchronized (solutionSubmittedInResolutionPhase) {
                                    // If the current submitted a solution
                                    if (!solutionSubmittedInResolutionPhase && runningSession) {
                                        // He can't play anymore for the turn
                                        playersWhichSubmittedABid.remove(currentPlayer);
                                        
                                        // Find the the current player
                                        if (findNewCurrentPlayer()) {
                                            // Send : TROPLONG/user/
                                            sendCommandToAllPlayersInCurrentTurn(CommandSC.TROPLONG.name(), currentPlayer.getUsername());
                                        }
                                        // End of the turn
                                        else{
                                            if (runningSession) {
                                                // Send : FINRESO/
                                                nextTurn(CommandSC.FINRESO);
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        // No current player found = end of turn
                        else {
                            Logger.logln("\nCurrent player not found.");
        
                            if (runningSession) {
                                // Send : FINRESO/
                                nextTurn(CommandSC.FINRESO);
                            }
                        }
                        
                        // Allow all user awaiting to join the turn
                        synchronized (playersAllowToJoinTurn) {
                            playersAllowToJoinTurn = true;
                            for (Player player : players) {
                                playersInCurrentTurn.add(player);
                            }   
                        }
                        
                        solutionSubmittedInResolutionPhase = false;
                        Logger.logln("\nEnd of turn.");
                    }
                
                } while (getWinner() == null && runningSession);
                
                // Case of winner = new session
                if (runningSession) {
                    // Send : VAINQUEUR/bilan/
                    sendCommandToAllPlayersInCurrentTurn(CommandSC.VAINQUEUR.name(), strAssessment());
                    createNewSession();
                    
                    // Send : SESSION/plateau/
                    sendCommandToAllPlayers(CommandSC.SESSION.name(), board.strBoard());
                }
            }
        } while (true);
    }

    /*
     * Send a command to all players except the player passed in parameter
     */
    private void sendCommandToOtherPlayers(Player ignoredPlayer, String ...args) {
    	Logger.forceLogln("sendCommandToOtherPlayers : " + args[0]);
    	
        for (Player player : players) {
            if (player != ignoredPlayer) {
                player.sendCommandToClient(args);
            }
        }
    }
    
    /*
     * Send a command to all players in current turn except the player passed in parameter
     */
    private void sendCommandToOtherPlayersInCurrentTurn(Player ignoredPlayer, String ...args) {
		Logger.forceLogln("sendCommandToOtherPlayersInCurrentTurn : " + args[0]);
    	
        for (Player player : playersInCurrentTurn) {
            if (player != ignoredPlayer) {
                player.sendCommandToClient(args);
            }
        }
    }
    
    /*
     * Send a command to all players
     */
    private void sendCommandToAllPlayers(String ...args) {
		Logger.forceLogln("sendCommandToAllPlayers : " + args[0]);
    	
        for (Player player : players) {
            player.sendCommandToClient(args);
        }
    }
    
    /*
     * Send a command to all players in current turn
     */
    private void sendCommandToAllPlayersInCurrentTurn(String ...args) {
		Logger.forceLogln("sendCommandToAllPlayersInCurrentTurn : " + args[0]);
		
        for (Player player : playersInCurrentTurn) {
            player.sendCommandToClient(args);
        }
    }
    
    /*
     * String assessment
     */
    public String strAssessment() {
        StringBuilder sb = new StringBuilder();
        sb.append(turn);
        
        synchronized (players) {
            for (Player player : players) {
                sb.append(player);
            }
        }
        
        return sb.toString();
    }
}
