/**
* @author sebastiend
*/
/**
*
*/
Ext.namespace('KSMod.PicturePreview.Ui');

/**
*
*/
KSMod.PicturePreview.Ui.load = function(){
    if (KSMod.PicturePreview.Ui.loaded !== undefined) {
        return;
    }
    KSMod.PicturePreview.Ui.loaded = true;
    
    /**
    * Picture preview template
    */
    var pictureTemplate = new Ext.XTemplate(
    	'<div class="picture-panel-template">', 
    	'<tpl for=".">', 
    	'<div class="picture-panel-info">',
    	'{name}<br>',  
    	'<span>{sizeString}</span><br>', 
    	'</div>', 
    	'<div class="picture-panel-img">',
    	'<img src="{url}" alt="{alt}" title="{title}"">', 
    	'</div>', 
    	'</tpl>', 
    	'</div>');
    pictureTemplate.compile();
    var pictureTemplateEl = '';
    
    
    function initPictureTemplate(el){
        pictureTemplateEl = el;
        // Picture preview
        pictureTemplate.overwrite(pictureTemplateEl, {
        		url: Ext.BLANK_IMAGE_URL,
        		title: '',
        		name: '',
        		sizeString: '',
        		dateString: ''
        });
    }
    
    function clearPictureTemplate(){
        initPictureTemplate(pictureTemplateEl);
    }
    
    /*
    * 
    */
    function overwritePictureTemplate(config){
    	var url = KSLib.Path.root('api/picture/resize/' + config.hash) + '?height=163&width=262';
    	var name = '<a href="' + KSLib.Path.root('api/picture/resize/' + config.hash) + '" target="_blank" >' + config.name + '</a>';
    	if (config.isMusic) {
    		url = KSLib.Path.root('api/music/id3picture/' + config.hash);
    		name = '<a href="' + KSLib.Path.root('api/music/id3picture/' + config.hash) + '" target="_blank" >' + config.name + '</a>';
    	}
    	
        pictureTemplate.overwrite(pictureTemplateEl, {
        		url: url,
        		title: config.name,
        		alt: config.name,
        		name: name,
        		sizeString: config.sizeString,
        		dateString: config.dateString
        });
    }
    
    KSMod.PicturePreview.Ui.init = initPictureTemplate;
    KSMod.PicturePreview.Ui.overwrite = overwritePictureTemplate;
    KSMod.PicturePreview.Ui.clear = clearPictureTemplate;
};
