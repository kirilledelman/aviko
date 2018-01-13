
log( "\npanel.js, this=", this );

(function( go ) {

	var test = new GameObject();
	log( "Init ", go, " test=", test );

	go.awake = function () {
		log( "Awake this=", this, "go=", go, " test=", test );
		go.test = test;
	};

})( this );
