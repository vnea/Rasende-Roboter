package rasenderroboter;

public class Target {
    // Position
    private Tile pos;
    
    // Symbol
    final static public String SYMBOL = "*";
    
    public Target() {
    }

    /*
     * Get x position
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
    
    @Override
    public String toString() {
        return getX() + "," + getY();
    }
}
