/**
* Convert lib namespace
*/
Ext.namespace('KSLib.Convert');

// Data
KSLib.Convert.KB = 1024;
KSLib.Convert.MB = 1024 * KSLib.Convert.KB;
KSLib.Convert.GB = 1024 * KSLib.Convert.MB;
KSLib.Convert.TB = 1024 * KSLib.Convert.GB;

// Time
KSLib.Convert.SECOND = 1000;
KSLib.Convert.MINUTE = 60 * KSLib.Convert.SECOND;
KSLib.Convert.HOUR = 60 * KSLib.Convert.MINUTE;
KSLib.Convert.DAY = 24 * KSLib.Convert.HOUR;

/**
* Convert byte to human readable form
* @param {Object} bytes
*/
KSLib.Convert.byteToHuman = function(bytes){
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
	
	
	if (bytes < KSLib.Convert.KB) // less than 1 KB
	{
		return bytes.toString() + "B";
	}
	if (bytes < KSLib.Convert.MB) // less than 1 MB
	{
		return unitByteToHuman(KSLib.Convert.KB, "KB", bytes);
	}
	if (bytes < KSLib.Convert.GB) // less than 1 GB
	{
		return unitByteToHuman(KSLib.Convert.MB, "MB", bytes);
	}
	if (bytes < KSLib.Convert.TB) // less than 1 TB
	{
		return unitByteToHuman(KSLib.Convert.GB, "GB", bytes);
	}
	return unitByteToHuman(KSLib.Convert.TB, "TB", bytes);
};


/**
* Convert milliseconds to human readable form
* @param {Object} milliseconds
* THIS FUNCTION NEED A FIX...
*/
KSLib.Convert.millisecondsToHuman = function(milliseconds, showMs){
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
	
	var days = milliseconds / KSLib.Convert.DAY;
	var remainder = milliseconds % KSLib.Convert.DAY;
	if (days > 1) {
		result += days.toFixed(0).toString() + "d ";
	}
	
	var hours = remainder / KSLib.Convert.HOUR;
	remainder = remainder % KSLib.Convert.HOUR;
	if (hours > 1) {
		result += hours.toFixed(0).toString() + ":";
	}
	
	var minutes = remainder / KSLib.Convert.MINUTE;
	remainder = remainder % KSLib.Convert.MINUTE;
	if (minutes > 1) {
		result += prefix(minutes.toFixed(0).toString(), "0", (hours > 1 ? 2 : 1)) + ":";
	}
	
	var seconds = remainder / KSLib.Convert.SECOND;
	remainder = remainder % KSLib.Convert.SECOND;
	result += prefix(seconds.toFixed(0).toString(), "0", (minutes > 1 ? 2 : 1));
	
	if (showMs !== undefined && showMs) {
		var ms = remainder;
		if (ms > 1) {
			result += "." + complete(ms.toFixed(0).toString(), "0", 3);
		}
	}
	
	return result;
};

KSLib.Convert.toBool = function(value){
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

KSLib.Convert.toParamValue = function(value){
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
	var valuePattern=/("[^"]+"|[^"\s]+)/g;
	var valueArray = value.match(valuePattern);
	
	for(i = 0; i < valueArray.length; ++i)
	{
		var v = valueArray[i];
		if (v.charAt(0) === '"' && v.charAt(v.length - 1) === '"')
		{
			v = v.substring(1, v.length - 1); // we remove starting and ending " char
		}
		
		valueArray[i] = KSLib.Convert.percentEncodeURIReservedChar(v);
	}
	
	
	return valueArray.join("+");
};

KSLib.Convert.percentEncodeURIReservedChar = function(value){
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

