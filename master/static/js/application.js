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
        $.get('/application/' + id + '/deploy', function(data) {
            if (data.status == 1) {
                alert(data.mesg);
            } else {
                deploy_show(data.page, id);
            }
        });
    });
}

function deploy_show(page, id)
{
    // deployment page
    $('#content').html(page);
    $('#content #back').click(function() {
        application_fill();
    });

    $('#content #deploy').click(function() {
        if ($('#content input').length == 0) {
            alert('The master cannot detect any nearby deployable nodes. Please move them in range of the basestation and try again.');
        } else if ($('#content input[type="checkbox"]:checked').length == 0) {
            alert('Please select at least one node');
        } else {
            var nodes = new Array();
            nodes = _.map($('#content input[type="checkbox"]:checked'), function(elem) {
                return parseInt($(elem).val(), 10);
            });

            console.log(nodes);

            $.post('/application/' + id + '/deploy', {selected_node_ids: nodes}, function(data) {
                deploy_poll(id, data.version);
            });
        }
    });
}

function deploy_poll(id, version)
{
    $.post('/application/'+id+'/deploy/poll', {version: version}, function(data) {
        $('#progress #compile_status').html('<p>' + data.deploy_status + '</p>');
        $('#progress #normal').html('<h2>NORMAL</h2><pre>' + data.normal.join('\n') + '</pre>');
        $('#progress #urgent_error').html('<h2>URGENT</h2><pre>' + data.error.urgent.join('\n') + '</pre>');
        $('#progress #critical_error').html('<h2>CRITICAL</h2><pre>' + data.error.critical.join('\n') + '</pre>');

        if (data.compile_status < 0) {
            deploy_poll(id, data.version);
        }
        else {
            $('#progress').dialog({buttons: {Ok: function() {
                $(this).dialog('close');
            }}})
        }
    });
}
