package rasenderroboter;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Random;
import java.util.Scanner;

import enumeration.Color;
import enumeration.Wall;

public class Board {
    // Nb rows and cols
    final public static int NB_ROWS = 16;
    final public static int NB_COLS = 16;
    
    // Tiles to represent the board
    private Tile[][] tiles = new Tile[NB_ROWS][NB_COLS];
    
    // All the robor and the main robot to reach the target
    private Map<Color, Robot> robots = new HashMap<>(4);
    private Color colorMainRobot;

    // The target
    private Target target = new Target();
    
    // Score to reach
    private int objectiveScore;
    
    private Random random = new Random();
    
    // Id
    private String id;
    
    // Folder of enigmas
    final private static String FOLDER_ENIGMA_PATH = "res/enigmas/";
    
    public Board() {
        // Init tiles
        for (int y = 0; y < NB_ROWS; ++y) {
            for (int x = 0; x < NB_COLS; ++x) {
                   tiles[y][x] = new Tile(x, y);
            }
        }
        
        // Init robots
        robots.put(Color.R, new Robot(Color.R));
        robots.put(Color.B, new Robot(Color.B));
        robots.put(Color.J, new Robot(Color.J));
        robots.put(Color.V, new Robot(Color.V));
    }
    
    /*
     * Load an enigma
     */
    public void loadEnigma() {
        for (int y = 0; y < NB_ROWS; ++y) {
            for (int x = 0; x < NB_COLS; ++x) {
                tiles[y][x].setRobot(null);
                tiles[y][x].setTarget(null);
            }
        }
        
        File folderEnigmaBoard = new File(FOLDER_ENIGMA_PATH, id);
        File currentEnigma = folderEnigmaBoard.listFiles()[random.nextInt(folderEnigmaBoard.list().length)];
        Scanner sc = null;
        try {
            sc = new Scanner(currentEnigma);
            colorMainRobot = Color.valueOf(sc.nextLine());
            
            for (int i = 0; i < 4; ++i) {
                Color color = Color.valueOf(sc.next());
                int x = sc.nextInt();
                int y = sc.nextInt();
                tiles[y][x].setRobot(robots.get(color));
            }
            
            int x = sc.nextInt();
            int y = sc.nextInt();
            tiles[y][x].setTarget(target);
        }
        catch (FileNotFoundException e) {
            e.printStackTrace();
        }

        if (sc != null) {
            sc.close();
        }
    }
    
    /*
     * Load a board from a file path
     */
    public void load(String filePath) {
        for (int y = 0; y < NB_ROWS; ++y) {
            for (int x = 0; x < NB_COLS; ++x) {
                tiles[y][x].removeWalls();
            }
        }
        id = filePath.substring(filePath.lastIndexOf("/") + 1, filePath.lastIndexOf("."));

        try {
            BufferedReader br = new BufferedReader(new FileReader(filePath));
            objectiveScore = Integer.parseInt(br.readLine());

            
            for (int x = 0; x < NB_ROWS * 2 + 1; ++x) {
                br.read();
            }
            br.read();
            
            for (int y = 0; y < NB_ROWS; ++y) {
                // Skip first |
                br.read();
                
                tiles[y][0].setRobot(null);
                tiles[y][0].setTarget(null);
                       
                br.read();

                for (int x = 1; x < NB_COLS; ++x) {
                    char left = (char) br.read();
                    if ('|' == left) {
                        tiles[y][x].setWallPresent(Wall.G, true);
                        tiles[y][x - 1].setWallPresent(Wall.D, true);
                    }

                    br.read();
                }
                // Skip last |
                br.read();
               
                // Skip \n
                br.read();
                
                // Bottom of the row
                for (int x = 0; x < NB_COLS; ++x) {
                    
                    br.read();
                    char val = (char) br.read();
                    
                    if ('-' == val) {
                        tiles[y][x].setWallPresent(Wall.B, true);
                        
                        if (y + 1 < NB_ROWS - 2) {
                            tiles[y + 1][x].setWallPresent(Wall.H, true);
                        }
                    }
                }
                // Skip +
                br.read();
                
                // Skip \n
                br.read();
            }
            
            br.close();
        }
        catch (FileNotFoundException e) {
            e.printStackTrace();
        }
        catch (IOException e) {
            e.printStackTrace();
        }
    }
    
    /*
     * Set a robot at the tile[y][x]
     */
    public void setRobotAtTile(Robot robot, int x, int y) {
        tiles[y][x].setRobot(robot);
    }
    
    /*
     * Return the bojective score
     */
    public int getObjectiveScore() {
    	return objectiveScore;
    }
    
    /*
     * Set the objective score
     */
    public void setObjectiveScore(int score) {
    	objectiveScore = score;
    }
    
    /*
     * Check if the main robot reach the target
     */
    private boolean mainRobotOnTarget() {        
        return robots.get(colorMainRobot).isOnTarget();
    }
    
    /*
     * Check if a solution if valid
     */
    public boolean isSolutionValid(List<ShiftingRobot> shiftingsRobots) {
        // Save tiles state before simulation
        Tile tileRobotR = robots.get(Color.R).getPos();
        Tile tileRobotB = robots.get(Color.B).getPos();
        Tile tileRobotJ = robots.get(Color.J).getPos();
        Tile tileRobotV = robots.get(Color.V).getPos();

        /** Simulate */
        simulateShitingsRobots(shiftingsRobots);
        
        // Check if the main robot is on the target
        boolean solutionValid = mainRobotOnTarget();
        
        // Reset tiles state
        for (int y = 0; y < NB_ROWS; ++y) {
            for (int x = 0; x < NB_COLS; ++x) {
              tiles[y][x].setRobot(null);
            }
        }
        tileRobotR.setRobot(robots.get(Color.R));
        tileRobotB.setRobot(robots.get(Color.B));
        tileRobotJ.setRobot(robots.get(Color.J));
        tileRobotV.setRobot(robots.get(Color.V));
        
        return solutionValid;
    }
    
    /*
     * Simulate the shiftments of the robots
     */
    private void simulateShitingsRobots(List<ShiftingRobot> shiftingsRobots) {
        for (ShiftingRobot shiftingRobot : shiftingsRobots) {
            Robot currentRobot = robots.get(shiftingRobot.getRobotColor());
            currentRobot.moveTo(shiftingRobot.getRobotDirection(), this);
        }
    }
    
    /*
     * Check is there is a robot at the tile[y][x]
     */
    public boolean robotPresentAtTile(int x, int y) {
        return tiles[y][x].robotPresent();
    }
    
    /*
     * Check if the wall w is present at tile[y][x] 
     */
    public boolean wallPresentAtTile(int x, int y, Wall w) {
        return tiles[y][x].wallPresent(w);
    }
    
    /*
     * String board (sent to the client)
     */
    public String strBoard() {
        StringBuilder sb = new StringBuilder();
        
        for (int y = 0; y < NB_ROWS; ++y) {
            for (int x = 0; x < NB_COLS; ++x) {
                sb.append(tiles[y][x]);
            }
        }
        
        return sb.toString();
    }
    
    /*
     * String full board (grid with robots + target)
     */
    public String strFullBoard() {
    	StringBuilder sb = new StringBuilder();
        for (int y = 0; y < NB_ROWS; ++y) {
            for (int x = 0; x < NB_COLS; ++x) {
                if (tiles[y][x].wallPresent(Wall.H)) {
                	sb.append("+-");
                }
                else {
                	sb.append("+ ");
                }
            }
            sb.append("+\n");

            
            for (int x = 0; x < NB_COLS; ++x) {
                if (tiles[y][x].wallPresent(Wall.G)) {
                	sb.append("|");
                }
                else {
                	sb.append(" ");
                }
                
                sb.append(tiles[y][x].tileContent());
                
            }
            sb.append("|\n");
        }
        
        for (int x = 0; x < NB_COLS; ++x) {
        	sb.append("+-");
        }
        sb.append("+");
        
        return sb.toString();
    }
    
    /*
     * String enigma
     */
    public String strEnigma() {
        StringBuilder sb = new StringBuilder();
        sb.append("(");
        
        sb.append(robots.get(Color.R));
        sb.append(",");
        
        sb.append(robots.get(Color.B));
        sb.append(",");
        
        sb.append(robots.get(Color.J));
        sb.append(",");
        
        sb.append(robots.get(Color.V));
        sb.append(",");
        
        sb.append(target);
        sb.append(",");
        
        sb.append(colorMainRobot);
        sb.append(")");

        return sb.toString();
    }
}
