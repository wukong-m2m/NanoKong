// vim: ts=4 sw=4
$(document).ready(function() {
    application_init();
});

function application_init()
{
    $('#menu #appadd').click(function() {
        $.post('/applications/new', function(data) {
            if (data.status == '1') {
                alert(data.mesg);
            }
            else {
                application_fill();
            }
        });
    });

    $('#menu #zwave').click(function() {
        $.get('/testrtt', function(data) {
            if (data.status == '1') {
                alert(data.mesg);
            }
            else {
                $('#content').html(data.testrtt);
                setup_testrtt();
            }
        });
    });

    application_fill();
}

function application_fill()
{
    $.ajax({
        url: '/applications',
        type: 'POST',
        dataType: 'json',
        success: function(r) {
            application_fillList(r);
        }
    });
}

function content_scaffolding(topbar, editor)
{
    $('#content').append(topbar);
    $('#content').append(editor);
}

function application_fillList(r)
{
    var i;
    var len = r.length;
    var m = $('#content');
    var obj, act;

    m.empty();
    applist = $('<table id=applist></table>');
    for(i=0; i<len; i++) {
        var appentry = $('<tr class=listitem></tr>');

        var name = $('<td class=appname id=appname'+i+'></td>');
        name.html('<a app_id="' + r[i].id + '" href="#">' + r[i].name + '</a>');

        var index = i;
        name.click(function() {
            var app_id = $(this).find('a').attr('app_id')
            $('#content').empty();
            $.post('/applications/' + app_id, function(data) {
                // injecting script to create application interface
                content_scaffolding(data.topbar, $('<iframe width="100%" height="100%" src="/applications/' + app_id + '/fbp/load"></iframe>'));

                //$('#content').html('<div id="topbar"></div><iframe width="100%" height="100%" src="/applications/' + app_id + '/fbp/load"></iframe>');
                //$('#topbar #back').click(function() {
                    //application_fill();
                //});
            });
        });

        var act = $('<td class=appact id=appact'+i+'></td>');

        // solution against lazy evaluation
        var id = r[i].id;

        var monitor = $('<button class=appmonitor id=appmonitor'+i+'></button>');
        var deploy = $('<button class=appdeploy id=appdeploy'+i+'></button>');
        var remove = $('<button class=appdel id=appdel'+i+'></button>');

        monitor.click(function() {
            $('#content').empty();
            var topbar;

            $.get('/applications/' + id, function(data) {
                if (data.status == 1) {
                    alert(data.mesg);
                } else {
                    topbar = data.topbar;
                    $.get('/applications/' + id + '/monitor', function(data) {
                        if (data.status == 1) {
                            alert(data.mesg);
                            application_fill();
                        } else {
                            // injecting script to create application interface
                            page = $(data.page);
                            console.log(page);
                            content_scaffolding(topbar, page);
                        }
                    });
                }
            });
        });

        deploy.click(function() {
            $('#content').empty();
            var topbar;

            $.get('/applications/' + id, function(data) {
                if (data.status == 1) {
                    alert(data.mesg);
                } else {
                    topbar = data.topbar;
                    $.get('/applications/' + id + '/deploy', function(data) {
                        if (data.status == 1) {
                            alert(data.mesg);
                            application_fill();
                        } else {
                            // injecting script to create application interface
                            page = $(data.page);
                            console.log(page);
                            content_scaffolding(topbar, page);
                        }
                    });
                }
            });

        });

        remove.click(function() {
            $.ajax({
                type: 'delete',
                url: '/applications/' + id, 
                success: function(data) {
                    if (data.status == 1) {
                        alert(data.mesg);
                    }

                    application_fill();
                }
            });
        });

        act.append(monitor);
        act.append(deploy);
        act.append(remove);

        appentry.append(name);
        appentry.append(act);
        applist.append(appentry);
        //application_setupButtons(i, r[i].id);
    }

    m.append(applist);
}

/*
function application_setupButtons(i, id)
{
    $('#appmonitor'+i).click(function() {
        alert("not implemented yet");
    });
    $('#appdeploy'+i).click(function() {
        $.get('/applications/' + id + '/deploy', function(data) {
            if (data.status == 1) {
                alert(data.mesg);
            } else {
                deploy_show(data.page, id);
            }
        });
    });
    $('#appdel'+i).click(function() {
        $.ajax({
            type: 'delete',
            url: '/applications/' + id, 
            success: function(data) {
                if (data.status == 1) {
                    alert(data.mesg);
                }

                application_fill();
            }
        });
    });
}
*/

/*
function application_setupLink(app_id)
{
    $.post('/applications/' + app_id, function(data) {
        // create application
        $('#content').html('<div id="topbar"></div><iframe width="100%" height="100%" src="/applications/' + app_id + '/fbp/load"></iframe>');
        $('#topbar').append(data.topbar);
        $('#topbar #back').click(function() {
            application_fill();
        });
    });
}
*/

/*
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

            $.post('/applications/' + id + '/deploy', {selected_node_ids: nodes}, function(data) {
                deploy_poll(id, data.version);
            });
        }
    });
}
*/

/*
function deploy_poll(id, version)
{
    $.post('/applications/'+id+'/deploy/poll', {version: version}, function(data) {
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
*/

function setup_testrtt()
{
    // testrtt page
    $('#content #back').click(function() {
        application_fill();
    });

    $('#content #include').click(function() {
        console.log('include');
        $('#log').html('<p>The basestation is ready to include devices.</p>');
        /*
        $.post('/testrtt/include', function(data) {
            $('#log').html(data.log);
        });
        */
    });

    $('#content #exclude').click(function() {
        console.log('include');
        $('#log').html('<p>The basestation is ready to exclude devices.</p>');
        /*
        $.post('/testrtt/exclude', function(data) {
            $('#log').html(data.log);
        });
        */
    });
}

// might have to worry about multiple calls :P
function poll(id, version, repeat)
{
    var forceRepeat = false;
    if (typeof repeat != 'undefined') {
        // repeat is an object with one key 'repeat', or it will be pass-by-value and therefore can't be stopped
        forceRepeat = repeat.repeat;
    }

    console.log('poll');
    $.post('/applications/'+id+'/poll', {version: version}, function(data) {
        console.log(data);
        _.each(data.log, function(line) {
            $('#log').append('<pre>' + data.log + '</pre>');
        });
        
        console.log('should ignore?');
        console.log(forceRepeat);
        if (forceRepeat || data.returnCode < 0) {
            console.log("didn't ignore");
            setTimeout(function() {
                poll(id, data.version, repeat);
            }, 1000);
        }
    });
}
