package rasenderroboter;

import enumeration.Color;
import enumeration.Direction;
import enumeration.Wall;

public class Robot {
    // Postion
    private Tile pos;
    
    // Color
    private Color color;
    
    public Robot(Color color) {
        this.color = color;
    }

    /*
     * Get x postion
     */
    public int getX() {
        return pos.getX();
    }

    /*
     * Get y position
     */
    public int getY() {
        return pos.getY();
    }
    
    /*
     * Set position
     */
    public void setPosition(Tile pos) {
        this.pos = pos;
    }
    
    /*
     * Get color
     */
    public Color getColor() {
        return color;
    }
    
    /*
     * Get position
     */
    public Tile getPos() {
        return pos;
    }
    
    /*
     * Check if the robot is on a target
     */
    public boolean isOnTarget() {
        return pos.targetPresent();
    }
    
    /*
     * Move a robot to the direction
     */
    public void moveTo(Direction direction, Board board) {
        switch (direction) {
            // Go TOP
            case H:
            {
                int posY = getY() - 1;
                while (posY >= 0 && !board.robotPresentAtTile(getX(), posY) && !board.wallPresentAtTile(getX(), getY(), Wall.H) && !board.wallPresentAtTile(getX(), posY, Wall.B)) {
                    board.setRobotAtTile(null, getX(), getY());
                    board.setRobotAtTile(this, getX(), posY--);
                }
            }
            break;
            
            // Go DOWN
            case B:
            {
                int posY = getY() + 1;
                while (posY < Board.NB_ROWS && !board.robotPresentAtTile(getX(), posY) && !board.wallPresentAtTile(getX(), getY(), Wall.B)  && !board.wallPresentAtTile(getX(), posY, Wall.H)) {
                    board.setRobotAtTile(null, getX(), getY());
                    board.setRobotAtTile(this, getX(), posY++);
                }
            }
            break;
             
            // Go LEFT
            case G:
            {
                int posX = getX() - 1;
                while (posX >= 0 && !board.robotPresentAtTile(posX, getY()) && !board.wallPresentAtTile(getX(), getY(), Wall.G) && !board.wallPresentAtTile(posX, getY(), Wall.D)) {
                    board.setRobotAtTile(null, getX(), getY());
                    board.setRobotAtTile(this, posX--, getY());
                }
            }
            break;
            
            // Go RIGH
            case D:
            {
                int posX = getX() + 1;
                while (posX < Board.NB_COLS && !board.robotPresentAtTile(posX, getY()) && !board.wallPresentAtTile(getX(), getY(), Wall.D) && !board.wallPresentAtTile(posX, getY(), Wall.G)) {
                    board.setRobotAtTile(null, getX(), getY());
                    board.setRobotAtTile(this, posX++, getY());
                }
            }
            break;
            
            default: 
        }
    }
    
    @Override
    public String toString() {
        return getX() + "," + getY();
    }
}
