package enumeration;

public enum CommandSC {
	/** Connection */
	BIENVENUE,
	CONNECTE,
	
	/** Deconnection */
	DECONNEXION,
	
	/** Session start */
	SESSION,
	VAINQUEUR,
	
	/** Reflection phase */
	TOUR,
	TUASTROUVE,
	ILATROUVE,
	FINREFLEXION,
	
	/** Bid phase */
	VALIDATION,
	ECHEC,
	NOUVELLEENCHERE,
	FINENCHERE,
	
	/** Resolution phase */
	SASOLUTION,
	BONNE,
	MAUVAISE,
	FINRESO,
	TROPLONG
}
