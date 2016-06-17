package rasenderroboter;

import enumeration.Color;
import enumeration.Direction;
import exception.InvalidShiftingRobotException;


public class ShiftingRobot {
    // Color of the robot
    private Color robotColor;
    
    // Direction to go
    private Direction robotDirection;
    
    public ShiftingRobot(String color, String direction) throws InvalidShiftingRobotException {
        robotColor = Color.valueOf(color);
        robotDirection = Direction.valueOf(direction);
        
        if (robotColor == null || robotDirection == null) {
        	throw new InvalidShiftingRobotException("Invalid parameters");
        }
    }
    
    /*
     * Get color
     */
    public Color getRobotColor() {
        return robotColor;
    }
    
    /*
     * Get direction
     */
    public Direction getRobotDirection() {
        return robotDirection;
    }
}
