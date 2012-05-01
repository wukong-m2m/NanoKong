$(function () {

    // Accordion
    $("#accordion").accordion({
        header: "h3"
    });

    // Tabs
    $('#tabs2, #tabs').tabs();

    // Buttons
    $('button').button();

    //Anchors, Submit
    $('.button,#sampleButton').button();

    


    // Dialog
    $('#dialog').dialog({
        autoOpen: false,
        width: 600,
        buttons: {
            "Ok": function () {
                $(this).dialog("close");
            },
            "Cancel": function () {
                $(this).dialog("close");
            }
        }
    });

    // Dialog Link
    $('#dialog_link').click(function () {
        $('#dialog').dialog('open');
        return false;
    });

    // Modal Link
    $('#modal_link').click(function () {
        $('#dialog-message').dialog('open');
        return false;
    });

    // Datepicker
    $('#datepicker').datepicker({
        inline: true
    });


    // Progressbar
    $("#progressbar").progressbar({
        value: 20
    });

    //hover states on the static widgets
    $('#dialog_link, #modal_link, ul#icons li').hover(

    function () {
        $(this).addClass('ui-state-hover');
    }, function () {
        $(this).removeClass('ui-state-hover');
    });

    //Autocomplete
    var availableTags = ["ActionScript", "AppleScript", "Asp", "BASIC", "C", "C++", "Clojure", "COBOL", "ColdFusion", "Erlang", "Fortran", "Groovy", "Haskell", "Java", "JavaScript", "Lisp", "Perl", "PHP", "Python", "Ruby", "Scala", "Scheme"];

    $("#tags").autocomplete({
        source: availableTags
    });


    //Dialogs
    $("#dialog-message").dialog({
        autoOpen: false,
        modal: true,
        buttons: {
            Ok: function () {
                $(this).dialog("close");
            }
        }
    });


    //Remove focus from buttons
    $('.ui-dialog :button').blur();

    //load json file
    $("#reprogramming").hide();
    loadScenario();
    
/*
    $("#slider-horizontal").slider({
        range: "min",
        min: 0,
        max: 255,
        value: 100,
        slide: function (event, ui) {
            $("#amount").text(ui.value);
        },

        change: function(event, ui)
        {   
            setThreshold(ui.value);
        }

    });
    $("#amount").text($("#slider-horizontal").slider("value"));
*/

    //split button
    $("#rerun").button().click(function () {
        alert("Running the last action");
    }).next().button({
        text: false,
        icons: {
            primary: "ui-icon-triangle-1-s"
        }
    }).click(function () {
        alert("Could display a menu to select an action");
    }).parent().buttonset();
    


    $("#upload1").button().click(function () {
      reprogram(1);
    });
    $("#upload2").button().click(function () {
      reprogram(2);
    });

    $("#thresholdLow").button().click(function () {
      setThreshold(15);
    });
    $("#thresholdHigh").button().click(function () {
      setThreshold(240);
    });
    $("#thresholdMax").button().click(function () {
      setThreshold(255);
    });
    $("#toggleOccupancy").button().click(function () {
      if (nodestatus['occupied'] == 0)
        setPeopleInRoom(1);
      else
        setPeopleInRoom(0);
    });

/*    
    $("#radioset").buttonset();
    $('#radioset input:radio').change(function() {
        var value = $(this).val();
        switch (value) {
            case 'scenario1':
                reprogram(1);
                break;
            case 'scenario2':
                reprogram(2);
                break;   
        }
    });
    
    $("#format").buttonset();
    $('#format input:checkbox').click(function() {
        var value = $(this).val();
        if($("#check1").attr("checked"))	setPeopleInRoom(1);
        else	setPeopleInRoom(0);

    });
    */        
    
    
    setInterval( "loadScenario();", 1000 );
    
});

function setThreshold(new_threshold)
{
    //updaet threshold
    $.get('http://nielsmbp.local:5000/setThreshold?threshold='+new_threshold, function(data){});
    loadScenario();
}

function setPeopleInRoom(status)
{
    $.get('http://nielsmbp.local:5000/setPeopleInRoom?peopleinroom='+status, function(data){});
    loadScenario();
}

function reprogram(id)
{
//    $("#reprogramming").show();
    jQuery.ajaxSetup({async:false});
    $.get('http://nielsmbp.local:5000/reprogram?scenario='+id, function(data){
      alert("Done");
    });
    jQuery.ajaxSetup({async:true});
//    $("#reprogramming").hide();
    loadScenario();
}

var nodestatus;

function loadScenario()
{
//  $.get('http://nielsmbp.local:5000/updateStatus', function(data){});
  
//	$("#loading").show();
    $.ajax({
        dataType: 'json',
		    url: 'http://nielsmbp.local/~niels/getStatus',
		//url: 'http://localhost/~janetyc/intelproject/scenario1.json',
        success:function(data) {
          nodestatus = data;
          label = "";

          x = "<dl>";
          x += "<dt>Current scenario</dt><dd><div style='margin-top:5px'>"+ data['scenario'] +"</div></dd>";
          x += "<br>"
          x += "<br>"
          x += "<br>"
          x += "<br>"
          x += "<dt>Light sensor / Threshold</dt>"
          if (data['threshold'] >= data['lightsensor']) {
            label = "<span class='label success'>&lt=</span>";
          } else {
            label = "<span class='label important'>&gt</span>";
          }
          x += "<dd><div style='margin-top:5px'>"+ data['lightsensor'] + "&nbsp&nbsp" + label  + "&nbsp&nbsp" + data['threshold'] +"</div></dd>";
          if (data['scenario'] == 2) {
            x += "<br>"
            x += "<br>"
            if (data['occupied'] == 1)
              label = "<span class='label success'>Yes</span>";
            else
              label = "<span class='label important'>No</span>";
            x += "<dt>Occupied</dt><dd><div style='margin-top:5px'>" + label + "</div></dd>";
          }
          x += "<br>"
          x += "<br>"
          if (data['lamp_on'] == 1)
            label = "<span class='label success'>On</span>";
          else
            label = "<span class='label important'>Off</span>";
          x += "<dt>Lamp</dt><dd><div style='margin-top:5px'>" + label +"</div></dd>";
          x += "</dl>";
          
          $("#status_info").empty();
          $("#status_info").append(x);
           if(data['scenario'] == 1){
              $("#scenario_img").replaceWith('<div id="scenario_img"><img src="pics/scenario1.png"/></div>');
              $("#format").hide()
           } else if(data['scenario'] == 2){
			        $("#scenario_img").replaceWith('<div id="scenario_img"><img src="pics/scenario2.png"/></div>');
			        $("#format").show()
           }
//           $("#loading").fadeOut("slow");
        },
        error:function(data){
		//alert("error!");
        }
    });
    
    
}

var $tab_title_input = $("#tab_title"),
    $tab_content_input = $("#tab_content");
var tab_counter = 2;

// tabs init with a custom tab template and an "add" callback filling in the content
var $tabs = $("#tabs2").tabs({
    tabTemplate: "<li><a href='#{href}'>#{label}</a></li>",
    add: function (event, ui) {
        var tab_content = $tab_content_input.val() || "Tab " + tab_counter + " content.";
        $(ui.panel).append("<p>" + tab_content + "</p>");
    }
});

// modal dialog init: custom buttons and a "close" callback reseting the form inside
var $dialog = $("#dialog2").dialog({
    autoOpen: false,
    modal: true,
    buttons: {
        Add: function () {
            addTab();
            $(this).dialog("close");
        },
        Cancel: function () {
            $(this).dialog("close");
        }
    },
    open: function () {
        $tab_title_input.focus();
    },
    close: function () {
        $form[0].reset();
    }
});

// addTab form: calls addTab function on submit and closes the dialog
var $form = $("form", $dialog).submit(function () {
    addTab();
    $dialog.dialog("close");
    return false;
});

// actual addTab function: adds new tab using the title input from the form above


function addTab() {
    var tab_title = $tab_title_input.val() || "Tab " + tab_counter;
    $tabs.tabs("add", "#tabs-" + tab_counter, tab_title);
    tab_counter++;
}

// addTab button: just opens the dialog
$("#add_tab").button().click(function () {
    $dialog.dialog("open");
});

// close icon: removing the tab on click
// note: closable tabs gonna be an option in the future - see http://dev.jqueryui.com/ticket/3924
$("#tabs span.ui-icon-close").live("click", function () {
    var index = $("li", $tabs).index($(this).parent());
    $tabs.tabs("remove", index);
});

//Filament datepicker
$('#rangeA').daterangepicker();
$('#rangeBa, #rangeBb').daterangepicker();


//Dynamic tabs
var $tab_title_input = $("#tab_title"),
    $tab_content_input = $("#tab_content");
var tab_counter = 2;

// tabs init with a custom tab template and an "add" callback filling in the content
var $tabs = $("#tabs2").tabs({
    tabTemplate: "<li><a href='#{href}'>#{label}</a></li>",
    add: function (event, ui) {
        var tab_content = $tab_content_input.val() || "Tab " + tab_counter + " content.";
        $(ui.panel).append("<p>" + tab_content + "</p>");
    }
});

// modal dialog init: custom buttons and a "close" callback reseting the form inside
var $dialog = $("#dialog2").dialog({
    autoOpen: false,
    modal: true,
    buttons: {
        Add: function () {
            addTab();
            $(this).dialog("close");
        },
        Cancel: function () {
            $(this).dialog("close");
        }
    },
    open: function () {
        $tab_title_input.focus();
    },
    close: function () {
        $form[0].reset();
    }
});

// addTab form: calls addTab function on submit and closes the dialog
var $form = $("form", $dialog).submit(function () {
    addTab();
    $dialog.dialog("close");
    return false;
});

// actual addTab function: adds new tab using the title input from the form above


function addTab() {
    var tab_title = $tab_title_input.val() || "Tab " + tab_counter;
    $tabs.tabs("add", "#tabs-" + tab_counter, tab_title);
    tab_counter++;
}

// addTab button: just opens the dialog
$("#add_tab").button().click(function () {
    $dialog.dialog("open");
});

// close icon: removing the tab on click
// note: closable tabs gonna be an option in the future - see http://dev.jqueryui.com/ticket/3924
$("#tabs span.ui-icon-close").live("click", function () {
    var index = $("li", $tabs).index($(this).parent());
    $tabs.tabs("remove", index);
});




//Wijmo
$("#menu1").wijmenu();
$(".wijmo-wijmenu-text").parent().bind("click", function () {
    $("#menu1").wijmenu("hideAllMenus");
});
$(".wijmo-wijmenu-link").hover(function () {
    $(this).addClass("ui-state-hover");
}, function () {
    $(this).removeClass("ui-state-hover");
});

//Toolbar
$("#play, #shuffle").button();
$("#repeat").buttonset();



