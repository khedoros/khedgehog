/**
 * File retrorgb.js.
 *
 * Custom javascript for retrorgb
 *
 * 
 */

( function( $ ) {
  
  $(function() {
     // add some classes to youtube iframe
     $('article p > iframe[src^="https://www.youtube.com/embed/"]').addClass('video-inner').parent().addClass('video-wrapper')
  });

   window.RetroRGB = {
     getContentWidth: function() {
       // use width of theme h1 title
       var header = document.getElementsByClassName('entry-header');
       if (header.length === 0) {
          return 400;
       }
       return header[0].offsetWidth || 400;
      },
      getContentHeight: function() {
        var content = document.getElementsByClassName('content-area');
        if (content.length === 0) {
          return 400;
        }
        return content[0].offsetHeight || 400;
      }
  };
	
} )( jQuery );
