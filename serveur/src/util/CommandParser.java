package util;

import java.util.ArrayList;
import java.util.List;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class CommandParser {
    // Separator
    final public static String SEPARATOR = "/";
    
    // Max number of part of the command ; xxx/yyy/zzz/
    final public static int MAX = 3;

    // Pattern of each part
    final private static Pattern pattern = Pattern.compile("([A-Za-z0-9]+)");
    
    /*
     * Return all parts of the command in a List
     */
    public static List<String> getCommands(String message) {
    	List<String> commands = null;
    	if (message != null) {
	        commands = new ArrayList<>(MAX);
	        Matcher matcher = pattern.matcher(message);
	        while (matcher.find()) {
	            commands.add(matcher.group(0));
	        }
    	}
    	
        return commands;
    }
    
}
