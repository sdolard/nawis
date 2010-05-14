/**
* @author sebastiend
*/
/**
*
*/
Ext.namespace('NMod.PicturePreview.Ui');

/**
*
*/
NMod.PicturePreview.Ui.load = function(){
    if (NMod.PicturePreview.Ui.loaded !== undefined) {
        return;
    }
    NMod.PicturePreview.Ui.loaded = true;
    
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
    
    /**
    * 
    */
    function overwritePictureTemplate(config){
    	var height = config.height || 163;
    	var width = config.width || 262;
    	var api = config.api || 'api/picture/resize';
    	if (api.charAt(api.length - 1) == '/') {
    		api = api.slice(0, api.length - 1);
    	}
    	var url = NLib.Path.root(String.format('{0}/{1}?height={2}&width={3}', api, config.hash, height, width));
    	var name = String.format('<a href="{0}/{1}" target="_blank">{2}</a>', NLib.Path.root(api), config.hash, config.name);
    		
        pictureTemplate.overwrite(pictureTemplateEl, {
        		url: url,
        		title: config.name,
        		alt: config.name,
        		name: name,
        		sizeString: config.sizeString,
        		dateString: config.dateString
        });
    }
    
    NMod.PicturePreview.Ui.init = initPictureTemplate;
    NMod.PicturePreview.Ui.overwrite = overwritePictureTemplate;
    NMod.PicturePreview.Ui.clear = clearPictureTemplate;
};
