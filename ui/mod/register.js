/**
* @author sebastiend
*/
Ext.namespace('KSMod.Register.Ui');

/**
*
*/
KSMod.Register.Ui.load = function(){
    if (KSMod.Register.Ui.loaded !== undefined) {
        return;
    }
    KSMod.Register.Ui.loaded = true;
    
    // First name
    var firstNameLabel = new Ext.form.TextField({
    		fieldLabel: 'First name',
    		listeners: {
    			valid: function(f){
    				validControls();
    			}
    		}
    });
    
    // Last name
    var lastNameLabel = new Ext.form.TextField({
    		fieldLabel: 'Last name',
    		listeners: {
    			valid: function(f){
    				validControls();
    			}
    		}
    });
    
    // Email
    var emailLabel = new Ext.form.TextField({
    		fieldLabel: 'Email address',
    		vtype: 'email',
    		listeners: {
    			valid: function(f){
    				validControls();
    			},
    			invalid: function(f, msg){
    				validControls();
    			}
    		}
    });
    
    // Password
    var passwordLabel = new Ext.form.TextField({
    		fieldLabel: 'Password',
    		inputType: 'password',
    		minLength: 8,
    		listeners: {
    			valid: function(f){
    				validControls();
    			},
    			invalid: function(f, msg){
    				validControls();
    			}
    		}
    });
    
    // Confirm password
    var confirmPasswordLabel = new Ext.form.TextField({
    		fieldLabel: 'Confirm password',
    		inputType: 'password',
    		minLength: 8,
    		enableKeyEvents: true,
    		listeners: {
    			valid: function(f){
    				if (passwordLabel.getValue() !== confirmPasswordLabel.getValue()) {
    					confirmPasswordLabel.markInvalid("Password doesn't match.");
    				}
    				validControls();
    			},
    			invalid: function(f, msg){
    				validControls();
    			},
    			keypress: function(tf, e){ // setup an onkeypress event handler
    				if (e.getKey() === e.ENTER) {// listen for the ENTER key
    					registerAction.execute();
    				}
    			}
    		}
    });
    
    
    // Valid controls
    function validControls(){
        if (firstNameLabel.getValue() === "" ||
        	lastNameLabel.getValue() === "" ||
        emailLabel.getValue() === "" ||
        passwordLabel.getValue() === '') {
        registerAction.disable();
        return false;
        }
        
        if (!emailLabel.isValid(true)) {
            registerAction.disable();
            return false;
        }
        
        if (!passwordLabel.isValid(true)) {
            registerAction.disable();
            return false;
        }
        
        if (passwordLabel.getValue() === undefined ||
        	passwordLabel.getValue() === '' ||
        passwordLabel.getValue().length < 8) {
        registerAction.disable();
        return false;
        }
        if (passwordLabel.getValue() !== confirmPasswordLabel.getValue()) {
            registerAction.disable();
            return false;
        }
        registerAction.enable();
        return true;
    }
    
    var ajaxConnection = KSLib.Ajax.newAjaxConnection(KSLib.Ajax.newWaitMask("Please wait..."));
    
    // Register function
    function register(){
        ajaxConnection.request({
        		url: '../api/user/register',
        		callback: function(options, success, response){
        			if (success) {
        				var xmlReader = new Ext.data.XmlReader({
        						record: "response"
        				}, Ext.data.Record.create([{
        							name: 'registered'
        				}, {
        					name: 'reason'
        				}]));
        				var registerResponse = xmlReader.read(response);
        				var data = registerResponse.records[0].data;
        				if (KSLib.Convert.toBool(data.registered)) {
        					Ext.MessageBox.show({
        							title: 'nawis: registeration succeed',
        							msg: 'This need a validation from administrator in order to be active.',
        							width: 315,
        							closable: true,
        							icon: Ext.MessageBox.INFO,
        							buttons: {
        								ok: 'Ok'
        							},
        							fn: function(buttonId, text, opt){
        								window.open('/ui', "_self");
        							}
        					});
        				}
        				else {
        					var msg;
        					if (data.reason === 'already_registered') {
                                Ext.MessageBox.show({
                                		title: 'nawis: registeration failed',
                                		msg: 'This email address is already registered. Have you forget your password?',
                                		width: 315,
                                		icon: Ext.MessageBox.ERROR,
                                		buttons: {
                                			yes: 'I forgot my password',
                                			no: 'I will review my registration order'
                                		},
                                		closable: true,
                                		fn: function(buttonId, text, opt){
                                			switch (buttonId) {
                                            case "yes":
                                                Ext.MessageBox.show({
                                                		title: 'nawis',
                                                		msg: 'Administrator will contact you as soon he will be informed.',
                                                		width: 315,
                                                		icon: Ext.MessageBox.INFO,
                                                		buttons: {
                                                			ok: 'Ok, thanks.'
                                                		},
                                                		fn: function(){
                                                			window.open('/ui', "_self");
                                                		}
                                                });
                                                break;
                                            case "no":
                                                emailLabel.focus(true);
                                                emailLabel.markInvalid('This email address is already used.');
                                                break;
                                            }
                                        }
                                });
                            } else {   
                                Ext.MessageBox.show({
                                		title: 'nawis: registeration failed',
                                		msg: 'Unknown error. Try again later.',
                                		width: 315,
                                		icon: Ext.MessageBox.ERROR,
                                		buttons: {
                                			ok: 'Ok'
                                		},
                                		closable: true,
                                		fn: function(buttonId, text, opt){
                                		}
                                });
                                
                            }
                        }
                    }
                    else {
                    	window.open('/ui', "_self");
                    }
                },
                params: {
                	firstName: firstNameLabel.getValue(),
                	lastName: lastNameLabel.getValue(),
                	email: emailLabel.getValue(),
                	password: passwordLabel.getValue()
                }
        });
    }
    
    // Registration panel
    var registerPanel = new Ext.form.FormPanel({
    		frame: true,
    		autoScroll: true,
    		defaults: {
    			width: 160
    		},
    		items: [firstNameLabel, lastNameLabel, emailLabel, passwordLabel, confirmPasswordLabel]
    });
    
    // Add shared dir action
    var registerAction = new Ext.Action({
    		text: 'Register',
    		disabled: true,
    		handler: function(){
    			if (!validControls()) {
    				return;
    			}
    			register();
    		}
    });
    
    // Add shared dir action
    var cancelAction = new Ext.Action({
    		text: 'Cancel',
    		handler: function(){
    			window.open('/ui', "_self");
    		}
    });
    
    var registerWindow = new Ext.Window({
    		title: KSVersion,
    		iconCls: 'ks-window-login-icon',
    		closable: false,
    		resizable: false,
    		modal: true,
    		layout: 'fit',
    		width: 315,
    		height: 215,
    		autoScroll: true,
    		closeAction: 'hide',
    		plain: true,
    		items: registerPanel,
    		buttons: [cancelAction, registerAction]
    });
    registerWindow.show();
    firstNameLabel.focus(false, 500);
};
