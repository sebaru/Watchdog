var isMoving = false;
var rotationDuration = 12500;
var gameDuration = 25000;
var gameTimeOut;
var randomColIndex = 0;
var randomCol = '';
var totalPoints = 0;
var colors = ['rouge', 'orange', 'rose', 'violet', 'joker'];
var direction = 1;
var nextColorDeg = 0;

var points_map = [40, 50, 60, 150, 80, 90, 80, 100, 70, 60, 50, 150, 30, 20, 10, 100];
var colors_map = [3, 1, 2, 4, 0, 3, 1, 4, 2, 0, 3, 4, 1, 2, 0, 4];

var startIndex = 0;


$(document).ready(function() {
    console.log("document loaded");


    $('.play-bt').click(function() {
        if (isMoving) stopRotation();
    })

    $('.rejouer-bt').click(function() {
        $('.roulette').velocity({ rotateZ: 0 }, { duration: "0", easing: "linear"});
        $.modal.close();
        restart();
    });

    $('.envoyer-bt, .envoyer-btt').click(function() {
        $('#envoyerModal').modal({
            clickClose: false,
            showClose: true,
            escapeClose: false,
            closeText: ''
        });
    });

    $('#envoyerModal').on($.modal.AFTER_CLOSE, function(event, modal) {
      //loose
      var currentValue = $('#loserwinner').val();
    
      if(currentValue == 'false'){
        $('#gameoverModal').modal({
            clickClose: false,
            showClose: false,
            escapeClose: false
        });

      //win
      }else{
        $('#thanksModal').modal({
            clickClose: false,
            showClose: false,
            escapeClose: false
        });
     }
    });


    $('.envoyer-act').click(function(e) {
        e.preventDefault();
        var badForm = false;

        $(".finput").each(function(index, value) {
            if (!fvalidate($(this))) {
                badForm = true;
                $(this).val('');
                $(this).addClass('error');
                $(this).attr('placeholder', $(this).attr('err_placeholder'));
            }
        });

        if (!badForm) {
            sendForm();
        }
    });

    $(".finput").focus(function() {
        if ($(this).hasClass('error')) {
            $(this).removeClass('error');
        }
    });
    
    
    $('.partager-bt').click(function(){
        shareOnFB();
    });


    $('.close_regles').click(function() {
        if ($(".regles").hasClass("active")) {
            unpause();
        }else{
            pause();
        }
        $(".regles").toggleClass("active");
    });

    $('.playbt').click(function() {
        play();
    });

	$( "#add_joueur" ).click(function() {
		
		if ($("#terms").is(":checked")) var terms=1;
		else var terms = '';
		
		if ($("#promotion").is(":checked")) var promotion=1;
		else var promotion = 0;
		
		$('#validation_errors').html('');
		
		$.post( "jeu/add_joueur", { 
			last_name: $("#last_name").val(),  
			first_name: $("#first_name").val(),
			email: $("#email").val(),
			address: $("#address").val(),
			zip_code: $("#zip_code").val(),
			city: $("#city").val(),
			terms: terms,
			promotion: promotion,
		})
		.done(function( data ) {
			if (data=='success'){
				$('#validation_errors').html('');
				$('#thanksModal').modal({
					clickClose: false,
					showClose: false,
					escapeClose: false
				});
			}
			else $('#validation_errors').html(data);
		});
	});
	
    resize();

});

$(window).resize(function() {
    resize()
});

function resize() {
    // var areaHeight =  window.innerHeight - 200;
    // if(areaHeight < 600){
    //     var zoom = areaHeight / 600;
    //     $('.jeu-ui').css('zoom',zoom);
    // }
}

function play() {
    console.log('play');
    $(".game").show();
    $(".intro").hide();
    init();
    setTimeout(function() { loopMe(); }, 2000);
}

function pause(){
    isMoving = false;
    $('.roulette').velocity('stop');
}

function unpause(){
    setTimeout(function() { makeFullRotation(); }, 1000);
}

function init() {
    randomColIndex = getRandomInt(0, 3);
    randomCol = colors[randomColIndex];

    var $div = $('.curseur');
    $.each(colors, function(i, v) {
        $div.removeClass(v);
    });
    $div.addClass(randomCol);


}

function restart() {
    init();
    setTimeout(function() { loopMe(); }, 1000);
    setPoints(0);
    direction = 1;
}

function loopMe() {
    isMoving = true;
    gameTimeOut = setTimeout(function() { bad(); }, gameDuration);
    makeFullRotation();
}

function stopRotation() {
    clearTimeout(gameTimeOut);
    isMoving = false;
    var deg = getRotationDegrees($('.roulette'));
    var result_points = degToPoints(deg);
    var result_color = degToColor(deg);
    console.log(deg, result_points);

    if (result_color == randomColIndex || result_color == 4) {
        good(result_points);
    }
    else {
        bad();
    }
}

function makeFullRotation() {
    isMoving = true;
    var deg = getRotationDegrees($('.roulette'));
    startIndex = degToIndex(deg);
    if (direction == 1) {
        $('.roulette').velocity({ rotateZ: "+=360" }, { duration: rotationDuration, easing: "linear", complete: makeFullRotation });
    }
    else {
        $('.roulette').velocity({ rotateZ: "-=360" }, { duration: rotationDuration, easing: "linear", complete: makeFullRotation });
    }

}

// function onProgress(elements, complete, remaining, start, tweenValue) {
//     if (!isMoving) return;
//     //calculate next color rotation
//     var deg = getRotationDegrees($('.roulette'));
//     var currentIndex = degToIndex(deg);
//     var prevIndex = currentIndex - direction;

//     if (currentIndex == startIndex || prevIndex == startIndex) return;

//     if (prevIndex < 0) prevIndex = colors_map.length - 1;
//     if (prevIndex >= colors_map.length) prevIndex = 0;

//     console.log(colors[randomColIndex], colors[colors_map[prevIndex]])

//     if (randomColIndex == colors_map[prevIndex]) {
//         isMoving = false;
//         setTimeout(bad, rotationDuration / 32);
//     }
// }

function good(points) {
    $('.roulette').velocity('stop');
    addPoints(points);
    if(totalPoints >= 300){
        win();
    }else{
        direction *= -1;
        init();
        loopMe();
    }
}

function bad() {
    $('.roulette').velocity('stop');
    if (totalPoints >= 300) {
        win();
    }
    else {
        lose();
    }
}

function win() {
    $('#loserwinner').val(true);
    $('#winModal .resultat').html('KISSES : ' + totalPoints);
    $('#winModal').modal({
        clickClose: false,
        showClose: false,
        escapeClose: false
    });
}

function lose() {
    $('#loserwinner').val(false);
    $('#gameoverModal .resultat').html('KISSES : ' + totalPoints);
    $('#gameoverModal').modal({
        clickClose: false,
        showClose: false,
        escapeClose: false
    });
}

function sendForm() {
    var form = $('#envoyer_form');
    $.ajax({
        type: "POST",
        url: form.attr('action'),
        data: form.serialize(),
        success: function(response) {
            $(".finput").val('');
            $('.envoyer-act').text('SENT')
        }
    });
}

// TOOLS
function getRandomInt(min, max) {
    return Math.floor(Math.random() * (max - min + 1)) + min;
}

function getRotationDegrees(obj) {
    var matrix = obj.css("-webkit-transform") ||
        obj.css("-moz-transform") ||
        obj.css("-ms-transform") ||
        obj.css("-o-transform") ||
        obj.css("transform");
    if (matrix !== 'none') {
        var values = matrix.split('(')[1].split(')')[0].split(',');
        var a = values[0];
        var b = values[1];
        var angle = Math.round(Math.atan2(b, a) * (180 / Math.PI));
    }
    else { var angle = 0; }
    return (angle < 0) ? angle + 360 : angle;
}

function degToPoints(deg) {
    deg = Math.abs(deg);
    return points_map[Math.floor(deg / 22.5)];
}

function degToColor(deg) {
    deg = Math.abs(deg);
    return colors_map[Math.floor(deg / 22.5)];
}

function degToIndex(deg) {
    deg = Math.abs(deg);
    return Math.floor(deg / 22.5);
}

function addPoints(points) {
    totalPoints += points;
    $('#points').html(totalPoints);
}

function setPoints(points) {
    totalPoints = points;
    $('#points').html(totalPoints);
}

function fvalidate(field) {
    if (field.attr('type') == 'text' && field.val().length > 2) return true;
    if (field.attr('type') == 'email' && validateEmail(field.val())) return true;
    return false;
}

function validateEmail(email) {
    var re = /^(([^<>()\[\]\\.,;:\s@"]+(\.[^<>()\[\]\\.,;:\s@"]+)*)|(".+"))@((\[[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}])|(([a-zA-Z\-0-9]+\.)+[a-zA-Z]{2,}))$/;
    return re.test(email);
}

function shareOnFB() {
        var urlindex = location.href.replace("mobile.html", "");
        window.open('https://www.facebook.com/sharer.php?u='+urlindex, 'fbShareWindow', 'height=450, width=550, top=' + ($(window).height() / 2 - 275) + ', left=' + ($(window).width() / 2 - 225) + ', toolbar=0, location=0, menubar=0, directories=0, scrollbars=0');
        return false;
}
