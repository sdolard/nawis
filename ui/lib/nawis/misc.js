/**
 * NLib Misc namespace
 */
Ext.namespace('NLib.Misc');
                                    
/**
 * Return true if server is localhost or 127.0.0.1
 */
NLib.Misc.isLocalServer = function(){ 
	var hostName = String(location.hostname); 
    if (location.hostname.toLowerCase() == "localhost" || hostName == "127.0.0.1") {
		return true;
	}
	return false;
};
