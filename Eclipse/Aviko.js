
/**
@param {string} [scriptName]
@return {GameObject}
@constructor
*/
function GameObject( scriptName ){}

/**
@type {Number}
*/
GameObject.prototype.x = 0;

/**
@type {Number}
*/
GameObject.prototype.y = 0;


function RenderText( fontName ) {}

/**
@type {string}
*/
RenderText.prototype.text = "";


RenderText.prototype.align = 0;

RenderText.prototype.centered = false;
