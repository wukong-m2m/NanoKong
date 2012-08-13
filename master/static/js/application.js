// vim: ts=4 sw=4
$(document).ready(function() {
    application_init();
});

function application_init()
{
    obj = $('#menu');
    obj.empty();
    obj.append('<button id=appadd class=appadd></button>')
    obj.click(function() {
        $.post('/application/new', function(data) {
            if (data.status == '1') {
                alert(data.mesg);
            }
            else {
                application_fill();
            }
        });
    });
    application_fill();
    /*
    application_fillList([
    {
    name:'application1'
    },
    {
    name:'application2'
    },
    {
    name:'application3'
    },
    {
    name:'application4'
    }
    ]);
    */
}


function application_fill()
{
    $.ajax({
        url: '/application/json',
        type: 'POST',
        dataType: 'json',
        success: function(r) {
            application_fillList(r);
        }
    });
}


function application_fillList(r)
{
    var i;
    var len = r.length;
    var m = $('#content');
    var obj, act;

    m.empty();
    m.html('<table id=applist></table>');
    applist = $('#applist');
    for(i=0;i<len;i++) {
        if ((i % 2) == 0) {
            applist.append('<tr class=listitem_even><td class=appname id=appname'+i+'></td><td class=appact id=appact'+i+'></td></tr>');
        } else {
            applist.append('<tr class=listitem_odd><td class=appname id=appname'+i+'></td><td class=appact id=appact'+i+'></td></tr>');
        }
        act = $('#appact'+i);
        act.append('<button class=appmonitor id=appmonitor'+i+'></button>');
        act.append('<button class=appdeploy id=appdeploy'+i+'></button>');
        act.append('<button class=appdel id=appdel'+i+'></button>');
        obj = $('#appname'+i);
        obj.html('<a app_id="' + r[i].id + '" href="#">' + r[i].name + '</a>');
        var index = i;
        obj.click(function() {
            var app_id = $(this).find('a').attr('app_id')
            application_setupLink(app_id);
        });
        application_setupButtons(i, r[i].id);
    }
}

function application_setupLink(app_id)
{
    $.post('/application/' + app_id, function(data) {
        // create application
        $('#content').html('<div id="topbar"></div><iframe width="100%" height="100%" src="/application/' + app_id + '/fbp/load"></iframe>');
        $('#topbar').append(data.topbar);
        $('#topbar #back').click(function() {
            application_fill();
        });
    });
}

function application_setupButtons(i, id)
{
    $('#appmonitor'+i).click(function() {
        alert("not implemented yet");
    });
    $('#appdel'+i).click(function() {
        $.ajax({
            type: 'delete',
            url: '/application/' + id, 
            success: function(data) {
                if (data.status == 1) {
                    alert(data.mesg);
                } else {
                    application_fill();
                }
            }
        });
    });
    $('#appdeploy'+i).click(function() {
        alert("not implemented yet");
    });
}
