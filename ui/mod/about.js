/**
* @author sebastien
*/

Ext.namespace('NMod.About.Ui');

/**
*
*/
NMod.About.Ui.load = function(){
	
	if (NMod.About.Ui.loaded  !== undefined){
		return;
	}
	NMod.About.Ui.loaded = true;
    
	
    /*
    * Global var
    */
    var aboutHtmlText = NVersion + '<br>';
    aboutHtmlText += "This software is written by Sébastien Dolard.<br>Email: sdolard@gmail.com<br><br>nawis use Silk icon set 1.3.<br>";
    aboutHtmlText += "Silk (<a href=\"http://www.famfamfam.com/lab/icons/silk/\" target=\"_blank\">http://www.famfamfam.com/lab/icons/silk/</a>)";
    aboutHtmlText += " is a smooth icon set, containing a large variety of icons in strokably-soft PNG format.<br>";
    aboutHtmlText += "Silk is licensed under a Creative Commons Attribution 2.5 License: ";
    aboutHtmlText += "<a href=\"http://creativecommons.org/licenses/by/2.5/\" target=\"_blank\">http://creativecommons.org/licenses/by/2.5/</a>.<br>";
    aboutHtmlText += "Silk is made by Mark James. If you have any questions about this icon set please contact mjames@gmail.com.<br><br>";
	aboutHtmlText += "The program is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.";
    
    var aboutToolbarButton = new Ext.Button({
    		text: 'About',
    		id: 'aboutToolbarButton',
    		icon: NLib.Path.res('help'),
    		cls: 'x-btn-text-icon'
    });
    
    aboutToolbarButton.on('click', function(){
    		var aboutWindows = new Ext.Window({
    				id: 'aboutWindows',
    				title: 'About',
    				iconCls: 'ks-window-about-icon',
    				modal: true,
    				layout: 'fit',
    				width: 500,
    				height: 300,
    				closeAction: 'close',
    				plain: true,
    				border: false,
    				items: new Ext.Panel({
    						width: 400,
    						html: aboutHtmlText
    				}),
    				buttons: [{
    						text: 'Close',
    						handler: function(){
    							aboutWindows.close();
    							delete aboutWindows;
    						}
    				}]
    		});
    		aboutWindows.show(Ext.get('aboutToolbarButton'));
    });
    
    NMod.About.Ui.accessButton = aboutToolbarButton;
    return NMod.About.Ui;
};
