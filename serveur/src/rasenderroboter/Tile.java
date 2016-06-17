package rasenderroboter;

import java.util.HashMap;
import java.util.Map;

import enumeration.Wall;

public class Tile {
    // x coordinate
    private int x;
    
    // y coordinate
    private int y;
    
    // Walls present for each 4 sides 
    Map<Wall, Boolean> walls = new HashMap<>(4);
    
    // Robot on the tile
    private Robot robot = null;
    
    // Target on the tile
    private Target target = null;
    
    public Tile(int x, int y) {
        this.x = x;
        this.y = y;
        
        removeWalls();
    }
    
    /*
     * Remove all walls
     */
    public void removeWalls() {
        setWallPresent(Wall.H, false);
        setWallPresent(Wall.B, false);
        setWallPresent(Wall.G, false);
        setWallPresent(Wall.D, false);
    }
    
    /*
     * Check if a wall is present
     */
    public boolean wallPresent(Wall wall) {
        return walls.get(wall);
    }
    
    /*
     * Set if a wall if present
     */
    public void setWallPresent(Wall wall, Boolean present) {
        // A tile on the border of the board must have a wall

        switch (wall) {
            case H:
                walls.put(wall, present || y == 0);
                break;
            case B:
                walls.put(wall, present || y == Board.NB_ROWS - 1);
                break;
            case G:
                walls.put(wall, present || x == 0);
                break;
            case D:
                walls.put(wall, present || x == Board.NB_COLS - 1);
                break;
        }
    }
    
    /*
     * Get x pos
     */
    public int getX() {
        return x;
    }
    
    /*
     * Get y pos
     */
    public int getY() {
        return y;
    }

    /*
     * Set the robot
     */
    public void setRobot(Robot robot) {
        this.robot = robot;
        
        if (robot != null) {
            robot.setPosition(this);
        }
    }
    
    /*
     * Set the target
     */
    public void setTarget(Target target) {
        this.target = target;
        
        if (target != null) {
            target.setPosition(this);
        }
    }
    
    /*
     * Check if a robot is present
     */
    public boolean robotPresent() {
        return robot != null;
    }
    
    /*
     * Check if a target is present
     */
    public boolean targetPresent() {
        return target != null;
    }
    
    /*
     * Return the tile content (robot, target, or nothing)
     */
    public String tileContent() {
        String str = null;

        if (robot != null) {
            str = robot.getColor().name();
        }
        else if (target != null) {
            str = Target.SYMBOL;
        }
        else {
            str = " ";
        }
        
        return str;
    }
    
    @Override
    public String toString() {
        StringBuilder sb = new StringBuilder();
        for (Map.Entry<Wall, Boolean> entry : walls.entrySet()) {
            if (entry.getValue()) {
                sb.append("(");
                sb.append(x);
                
                sb.append(",");
                sb.append(y);
                
                sb.append(",");
                sb.append(entry.getKey());
                sb.append(")");
            }
        }
        
        return sb.toString();
    }
}
