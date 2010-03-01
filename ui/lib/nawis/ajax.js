/**
 * Ajax lib namespace
 */
Ext.namespace('NLib.Ajax');

/**
 *
 * @param {Object} message
 * @param {Object} store
 */
NLib.Ajax.newWaitMask = function(message, store){
    return new Ext.LoadMask(Ext.getBody(), {
        msg: message,
        store: store
    });
};


/**
 *
 * @param {Object} waitMask
 */
NLib.Ajax.newAjaxConnection = function(waitMask){
    return new Ext.data.Connection({
        listeners: {
            'beforerequest': {
                fn: function(con, opt){
                    waitMask.show();
                },
                scope: this
            },
            'requestcomplete': {
                fn: function(con, res, opt){
                    waitMask.hide();
                },
                scope: this
            },
            'requestexception': {
                fn: function(con, res, opt){
                    waitMask.hide();
                },
                scope: this
            }
        }
    });
};
