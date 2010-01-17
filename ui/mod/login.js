/**
* @author sebastien
*/
Ext.BLANK_IMAGE_URL = 'lib/ext-3.1.0/resources/images/default/s.gif';

Ext.namespace('KSMod.Login.Ui');
/**
*
*/
KSMod.Login.Ui.load = function(){
    if (KSMod.Login.Ui.loaded !== undefined) {
        return;
    }
    KSMod.Login.Ui.loaded = true;
    
    KSLib.Session.clear();
    
	function doSubmit(){
		loginFormPanel.hide();
		// Add login action
		loginFormPanel.getForm().submit({
				text: 'Login',
				method: 'POST',
				clientValidation: true,
				failure: function(form, action){
					loginFormPanel.hide();
					Ext.MessageBox.show({
							title: 'nawis',
							msg: 'Authentication failed.',
							width: 150,
							closable: false
					});
					setTimeout(function(){
							Ext.MessageBox.hide();
							loginFormPanel.show();
							userNameTextField.focus();
					}, 300000);
				},
				success: function(form, action){
					loginFormPanel.hide();
					var data = Ext.util.JSON.decode(action.response.responseText);
					KSLib.Session.setLevel(data.level);
					window.open('/ui/nawis.html', "_self");
				},
				url: KSLib.Path.root('api/auth')
		}); 	
	}
	
	
	var userNameTextField = new Ext.form.TextField({
			fieldLabel: 'Username',
			el: 'username',
			id: 'username-text-field',
			inputType: 'text',
			enableKeyEvents: true,
			autoCreate: {
				tag: "input",
				autocomplete: "on"
			},
			listeners: {
    			keyup: function(tf, e){ // setup an onkeypress event handler
    				if (e.getKey() === e.ENTER && this.getValue().length > 0) {// listen for the ENTER key
    					pwdTextField.focus();
    				}
    			}
    		}
	});
	

	var pwdTextField = new Ext.form.TextField({
			fieldLabel: 'Password',
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
	});
		

	var loginButton = new Ext.Button({
			text: 'Login',
			type: 'submit',
			width: 75,
			onClick: doSubmit
	});
	

	var loginButtonHBox = new Ext.Container({
			layout: 'hbox',
			border: false,
			layoutConfig: {
				align: 'middle',
				pack: 'center'
			},
			//bodyCssClass: 'n-background',
			items: loginButton
	});
	
	var loginFormPanel = new Ext.form.FormPanel({
			id: 'login-form-panel',
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
	
    userNameTextField.focus(false, 500);

  
	setTimeout(function(){
			Ext.get('loading').remove();
			Ext.get('loading-mask').fadeOut({
					remove: true
			});
	}, 250);
}

Ext.onReady(function(){
		Ext.QuickTips.init();
		
		// turn on validation errors beside the field globally
		Ext.form.Field.prototype.msgTarget = 'side';
		Ext.form.Field.prototype.labelSeparator = '';
		
		// Must be called as last
		KSMod.Login.Ui.load();
});
