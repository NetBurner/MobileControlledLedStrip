$(function () {
    var $document = $(document);
    var selector = '#red_slider';
    var $element = $(selector);
    // For ie8 support
    var textContent = ('textContent' in document) ? 'textContent' : 'innerText';

    // Basic rangeslider initialization
    $element.rangeslider({
        // Deactivate the feature detection
        polyfill: false,

        rangeClass: 'rangeslider__red',

        // Callback function
        onInit: function()
        {
            ValueOutput(this.$element[0]);
        },
        // Callback function
        onSlide: function( position, value )
        {
        },
        // Callback function
        onSlideEnd: function( position, value )
        {
        }
    });

    selector = '#blue_slider';
    $element = $(selector);
    // Basic rangeslider initialization
    $element.rangeslider({
        // Deactivate the feature detection
        polyfill: false,

        rangeClass: 'rangeslider__blue',

        // Callback function
        onInit: function()
        {
            ValueOutput( this.$element[0] );
        },
        // Callback function
        onSlide: function( position, value )
        {
        },
        // Callback function
        onSlideEnd: function( position, value )
        {
        }
    });

    selector = '#green_slider';
    $element = $(selector);
    // Basic rangeslider initialization
    $element.rangeslider({
        // Deactivate the feature detection
        polyfill: false,

        rangeClass: 'rangeslider__green',

        // Callback function
        onInit: function()
        {
            ValueOutput(this.$element[0]);
        },
        // Callback function
        onSlide: function( position, value )
        {
        },
        // Callback function
        onSlideEnd: function( position, value )
        {
        }
    });

    // Example functionality to demonstrate a value feedback
    var selector = '[data-rangeslider]';
    function ValueOutput( element )
    {
        var value = element.value;
        var output = element.parentNode.getElementsByTagName('output')[0] || element.parentNode.parentNode.getElementsByTagName('output')[0];
        output[textContent] = value;
    }

    $document.on('input', 'input[type="range"], ' + selector, function (e) {
        ValueOutput(e.target);
    });
});