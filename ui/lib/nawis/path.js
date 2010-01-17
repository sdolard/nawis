/**
 * @author sebastien
 */
Ext.namespace('KSLib.Path');

/**
 *
 * @param {Object} name of the resource
 * All resources are png, so you anly ned to put tehe name without extention
 * @example
 * KSLib.Path.res('accept'); returns 'res/famfamfam_silk_icons_v013/icons/accept.png'
 */
KSLib.Path.res = function(name){
    if (name === undefined || typeof name !== "string") {
        throw new Error("name must be a string");
    }
    return KSLib.Path.root('ui/res/famfamfam_silk_icons_v013/icons/' + name + '.png');
};

KSLib.Path.root = function(path){
    return location.protocol + '//' + location.host + '/' + path;
};
