/**
 * @author sebastiend
 */
Ext.namespace('NLib');

NLib.Session = function(){

    function getCookie(name){
        var cName = "nawis_" + name;
        if (document.cookie.length > 0) {
            var start = document.cookie.indexOf(cName + "=");
            if (start != -1) {
                start = start + cName.length + 1;
                var end = document.cookie.indexOf(";", start);
                if (end == -1) {
                    end = document.cookie.length;
                }
                return unescape(document.cookie.substring(start, end));
            }
        }
        return "";
    }
    
    function setCookie(name, value, path, domain, expires){
        var exdate = new Date();
        exdate.setDate(exdate.getDate() + expires);
        
        document.cookie = "nawis_" + name + "=" + escape(value) +
        ((expires === undefined) ? "" : ";expires=" + exdate.toGMTString()) +
        ((path === undefined) ? "" : ("; path=" + path)) +
        ((domain === undefined) ? "" : ("; domain=" + domain)) +
        ((Ext.isSecure === true) ? "; secure" : "");
    }
    
    function clearCookie(name, path, domain){
        setCookie(name, "", path, domain, -1);
    }
    
    
    function getSessionId(){
        getCookie("sessionId");
    }
    
    function setSessionId(value){
        setCookie("sessionId", value);
    }
    
    function getLevel(){
        return getCookie("level");
    }
    
    function setLevel(value){
        setCookie("level", value);
    }
    
    function getRememberMe(){
        return getCookie("rememberMe") == "1";
    }
    
    function setRememberMe(value){
        setCookie("rememberMe", value ? "1" : "0", undefined, undefined, 30);
    }
    
    function getLogin(){
        return getCookie("login");
    }
    
    function setLogin(value){
        setCookie("login", value, undefined, undefined, 30);
    }
    
    function clearLogin(){
        clearCookie("login");
    }
    
    function getPwd(){
        return getCookie("pwd");
    }
    
    function setPwd(value){
        setCookie("pwd", value, undefined, undefined, 30);
    }
    
    function clearPwd(){
        clearCookie("pwd");
    }
    
    function isLevelSet(l){
        return getLevel().match(l) == l;
    }
    
    function clearSessionCookie(){
        clearCookie("sessionId");
        clearCookie("level");
        if (!getRememberMe()) {
            clearCookie("login");
            clearCookie("pwd");
        }
    }
    
    return {
        getLevel: getLevel,
        setLevel: setLevel,
        isLevelSet: isLevelSet,
        getSessionId: getSessionId,
        setSessionId: setSessionId,
        getRememberMe: getRememberMe,
        setRememberMe: setRememberMe,
        getLogin: getLogin,
        setLogin: setLogin,
        clearLogin: clearLogin,
        getPwd: getPwd,
        setPwd: setPwd,
        clearPwd: clearPwd,
        clear: clearSessionCookie
    };
}();


