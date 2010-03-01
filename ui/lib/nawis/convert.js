/**
* Convert lib namespace
*/
Ext.namespace('NLib.Convert');

// Data
NLib.Convert.KB = 1024;
NLib.Convert.MB = 1024 * NLib.Convert.KB;
NLib.Convert.GB = 1024 * NLib.Convert.MB;
NLib.Convert.TB = 1024 * NLib.Convert.GB;

// Time
NLib.Convert.SECOND = 1000;
NLib.Convert.MINUTE = 60 * NLib.Convert.SECOND;
NLib.Convert.HOUR = 60 * NLib.Convert.MINUTE;
NLib.Convert.DAY = 24 * NLib.Convert.HOUR;

/**
* Convert byte to human readable form
* @param {Object} bytes
*/
NLib.Convert.byteToHuman = function(bytes){
	if (bytes === undefined || typeof bytes !== "number") {
		throw new Error("bytes must be a number");
	}
	
	function unitByteToHuman(unit, unitString, bytes){
		var remainder = bytes % unit;
		if (remainder > 0) {
			var decimal = ((remainder / unit) + 0.05).toString().substring(0, 3);
			if (decimal.charAt(0) == 1) {
				
				return (bytes / unit + 1).toFixed(0).toString() + unitString;
			}
			decimal = decimal.slice(2);
			if (decimal.charAt(0) === 0) {
				return (bytes / unit).toFixed(0).toString() + unitString;
			}
			return (bytes / unit).toFixed(0).toString() + "." + decimal + unitString;
		}
		return (bytes / unit).toFixed(0).toString() + unitString;
	}
	
	
	if (bytes < NLib.Convert.KB) // less than 1 KB
	{
		return bytes.toString() + "B";
	}
	if (bytes < NLib.Convert.MB) // less than 1 MB
	{
		return unitByteToHuman(NLib.Convert.KB, "KB", bytes);
	}
	if (bytes < NLib.Convert.GB) // less than 1 GB
	{
		return unitByteToHuman(NLib.Convert.MB, "MB", bytes);
	}
	if (bytes < NLib.Convert.TB) // less than 1 TB
	{
		return unitByteToHuman(NLib.Convert.GB, "GB", bytes);
	}
	return unitByteToHuman(NLib.Convert.TB, "TB", bytes);
};


/**
* Convert milliseconds to human readable form
* @param {Object} milliseconds
* THIS FUNCTION NEED A FIX...
*/
NLib.Convert.millisecondsToHuman = function(milliseconds, showMs){
	if (milliseconds === undefined || typeof milliseconds !== "number") {
		throw new Error("milliseconds must be a number");
	}
	
	function prefix(s, c, l){
		while (s.length < l) {
			s = c + s;
		}
		return s;
	}
	
	var result = "";
	
	var days = milliseconds / NLib.Convert.DAY;
	var remainder = milliseconds % NLib.Convert.DAY;
	if (days > 1) {
		result += days.toFixed(0).toString() + "d ";
	}
	
	var hours = remainder / NLib.Convert.HOUR;
	remainder = remainder % NLib.Convert.HOUR;
	if (hours > 1) {
		result += hours.toFixed(0).toString() + ":";
	}
	
	var minutes = remainder / NLib.Convert.MINUTE;
	remainder = remainder % NLib.Convert.MINUTE;
	if (minutes > 1) {
		result += prefix(minutes.toFixed(0).toString(), "0", (hours > 1 ? 2 : 1)) + ":";
	}
	
	var seconds = remainder / NLib.Convert.SECOND;
	remainder = remainder % NLib.Convert.SECOND;
	result += prefix(seconds.toFixed(0).toString(), "0", (minutes > 1 ? 2 : 1));
	
	if (showMs !== undefined && showMs) {
		var ms = remainder;
		if (ms > 1) {
			result += "." + complete(ms.toFixed(0).toString(), "0", 3);
		}
	}
	
	return result;
};

NLib.Convert.toBool = function(value){
	if (value === undefined) {
		return false;
	}
	if (typeof(value) === 'boolean') {
		return value;
	}
	if (typeof(value) === 'number') {
		return value > 0;
	}
	if (typeof(value) === 'string') {
		return value === 'true' || value === '1';
	}
	
	return false;
};

NLib.Convert.toParamValue = function(value){
	// query = *( pchar / "/" / "?" )
	// pchar = unreserved / pct-encoded / sub-delims / ":" / "@"
	// sub-delims = "!" / "$" / "&" / "'" / "(" / ")" / "*" / "+" / "," / ";" / "="
	
	// 'foo bar'   > foo+bar
	// '"foo bar"' > foo%20bar
	// 'foo+bar'   > foo%2Bbar
	// '"foo+bar"' > foo+bar
	
	if (value === undefined || value.length === 0) {
		return "";
	}
	
	// http://stackoverflow.com/questions/64904/parsings-strings-extracting-words-and-phrases-javascript
	var valuePattern=/("[^"]*")|([^\s"]+)/g;
	var valueArray = value.match(valuePattern);
	if (valueArray === null)
	{	
		valueArray = [value]; // To manage '"+' value
	}
	
	for(i = 0; i < valueArray.length; ++i)
	{
		var v = valueArray[i];
		if (v.charAt(0) === '"' && v.length > 2 && v.charAt(v.length - 1) === '"')
		{
			v = v.substring(1, v.length - 1); // we remove starting and ending " char
		}
		
		valueArray[i] = NLib.Convert.percentEncodeURIReservedChar(v);
	}
	
	
	return valueArray.join("+");
};

NLib.Convert.percentEncodeURIReservedChar = function(value){
	//  gen-delims  = ":" / "/" / "?" / "#" / "[" / "]" / "@"
    //  sub-delims  = "!" / "$" / "&" / "'" / "(" / ")" / "*" / "+" / "," / ";" / "="
    
	return !value ? value : String(value).
	replace(':', '%3A').  // gen-delims
	replace('/', '%2F').  
	replace('?', '%3F').  
	replace('#', '%23').  
	replace('[', '%5B').  
	replace(']', '%5D').  
	replace('@', '%40').  
	replace('!', '%21').  // sub-delims
	replace('$', '%24').   
	replace('&', '%26').  
	replace("'", '%27').   
	replace('(', '%28').  
	replace(')', '%29').   
	replace('*', '%2A').
	replace('+', '%2B').
	replace(',', '%2C').
	replace(';', '%3B').
	replace('=', '%3D');
};

