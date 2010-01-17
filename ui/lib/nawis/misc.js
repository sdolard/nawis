/**
 * KSLib Misc namespace
 */
Ext.namespace('KSLib.Misc');
                                    
/**
 * Return true if server is localhost or 127.0.0.1
 */
KSLib.Misc.isLocalServer = function(){ 
	var hostName = String(location.hostname); 
    if (location.hostname.toLowerCase() == "localhost" || hostName == "127.0.0.1") {
		return true;
	}
	return false;
};
