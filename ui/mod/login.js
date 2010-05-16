/**
* @author sebastien
*/
Ext.BLANK_IMAGE_URL = 'lib/ext-3.2.1/resources/images/default/s.gif';

Ext.namespace('NMod.Login.Ui');
/**
*
*/
NMod.Login.Ui.load = function(){
    if (NMod.Login.Ui.loaded !== undefined) {
        return;
    }
    NMod.Login.Ui.loaded = true;
    
    NLib.Session.clear();
    
	function doSubmit(){
		loginFormPanel.hide();
		// Add login action
		loginFormPanel.getForm().submit({
				text: 'Login',
				method: 'POST',
				clientValidation: true,
				failure: function(form, action){
					Ext.getCmp('login-form-panel').hide();
					Ext.MessageBox.show({
							title: 'nawis',
							msg: 'Authentication failed.',
							width: 150,
							closable: false
					});
					setTimeout(function(){
							Ext.MessageBox.hide();
							loginFormPanel.show();
							Ext.getCmp('login-username-textfield').focus();
					}, 300000);
				},
				success: function(f, action){
					loginFormPanel.hide();
					var data = Ext.util.JSON.decode(action.response.responseText);
					NLib.Session.setLevel(data.level);
					window.open('/ui/nawis.html', "_self");
				},
				url: NLib.Path.root('api/auth')
		}); 	
	}
	
	
	var userNameTextField = {
			xtype: 'textfield',
			fieldLabel: 'Username',
			el: 'username',
			id: 'login-username-textfield',
			inputType: 'text',
			enableKeyEvents: true,
			autoCreate: {
				tag: "input",
				autocomplete: "on"
			},
			listeners: {
    			keyup: function(tf, e){ // setup an onkeypress event handler
    				if (e.getKey() === e.ENTER && this.getValue().length > 0) {// listen for the ENTER key
    					Ext.getCmp('pwd-username-textfield').focus();
    				}
    			}
    		}
	};
	

	var pwdTextField =  {
			xtype: 'textfield',
			fieldLabel: 'Password',
			id: 'pwd-username-textfield',
			el: 'password',
			inputType: 'password',
			enableKeyEvents: true,
			autoCreate: {
				tag: "input",
				autocomplete: "on"
			},
			listeners: {
    			keyup: function(tf, e){ // setup an onkeypress event handler
    				if (e.getKey() === e.ENTER && tf.getValue().length > 0) {// listen for the ENTER key
    					doSubmit();
    				}
    			}
    		}
	};
		

	var loginButton = {
			xtype: 'button',
			text: 'Login',
			type: 'submit',
			width: 75,
			onClick: doSubmit
	};
	

	var loginButtonHBox = new Ext.Container({
			layout: 'hbox',
			border: false,
			layoutConfig: {
				align: 'middle',
				pack: 'center'
			},
			items: loginButton
	});
	
	var loginFormPanel = new Ext.FormPanel({
			id: 'login-form-panel',
			renderTo: Ext.getBody(),
			/*bodyCssClass: 'n-background',
			defaults: {
				bodyCssClass: 'n-background'
			},*/
			border: false,
			labelAlign: 'right',
    		width: 350,
			items: [userNameTextField, pwdTextField, loginButtonHBox]
	});
	
	loginFormPanel.render(document.body);
	Ext.get('login-form-panel').center();
    Ext.getCmp('login-username-textfield').focus(false, 500);

  
	setTimeout(function(){
			Ext.get('loading').remove();
			Ext.get('loading-mask').fadeOut({
					remove: true
			});
	}, 250);
};

Ext.onReady(function(){
		Ext.QuickTips.init();
		
		// turn on validation errors beside the field globally
		Ext.form.Field.prototype.msgTarget = 'side';
		Ext.form.Field.prototype.labelSeparator = '';
		
		// Must be called as last
		NMod.Login.Ui.load();
});
