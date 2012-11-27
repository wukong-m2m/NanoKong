// vim: ts=4 sw=4
$(document).ready(function() {
    application_init();
});

function application_init()
{

    window.options = {repeat: false};
    $('#application').click(function() {
        $('#node-editor').parent().removeClass('active');
        $('#application').parent().addClass('active');
        $('#locationTree').parent().removeClass('active');
        window.options.repeat = false;
        application_fill();
    });

    $('#node-editor').click(function() {
        $('#node-editor').parent().addClass('active');
        $('#application').parent().removeClass('active');
        $('#locationTree').parent().removeClass('active');
        window.options.repeat = false;
        $.get('/testrtt', function(data) {
            if (data.status == '1') {
                alert(data.mesg);
            }
            else {
                $('#content').html(data.testrtt);
            }
        });
    });
    
    $('#locationTree').click(function() {
        $('#node-editor').parent().removeClass('active');
        $('#application').parent().removeClass('active');
        $('#locationTree').parent().addClass('active');
        window.options.repeat = false;
        $.post('/test/tree', function(data) {
	    		make_tree(data);
	    		$('#content').append(data.node);
/*    	
		$.ajax({
			url: '/test/tree',
			type: 'POST',
			dataType: 'json',
			success: function(r) {
				make_tree(r);
			}
*/		});                    
    });
    
    application_fill();
    
}

function make_tree(rt)
{
	$('#content').empty();
	var r = JSON.parse(rt.loc);
	
    var temp = 0
    var html_tree = ""
    html_tree += '<ul>'
    for( i in r){
		l = i % 10
		if(l == 0){
			html_tree += '<li id="'+ r[i] +'">'+r[i]+'</li>'
		}else if(l == temp){
			if(r[i].indexOf("#") == -1){
				html_tree += '<a role=button id="node'+r[i].substring(0,1)+'" data-toggle=modal href="#myModal" class="btn more">'+r[i]+'</button></a>'
			}else{
				html_tree += '<li id="'+ r[i] +'">'+r[i]+'</li>'
			}
		}else if(l > temp){
			if(r[i].indexOf("#") == -1){
				html_tree += '<ul><a role=button id="node'+r[i].substring(0,1)+'" data-toggle=modal href="#myModal" class="btn more">'+r[i]+'</button></a>'
			}else{
				html_tree += '<ul><li id="'+ r[i] +'">'+r[i]+'</li>'
			}
		}else if(l < temp){
			m = temp - l
			for(var j=0; j<m ;j++){
				html_tree += '</ul>'
			}
			html_tree += '<li id="'+ r[i] +'">'+r[i]+'</li>'
		}

		temp = l
	}
	for(var j=0; j<temp+1; j++){
		html_tree += '</ul>'
	}
//	DebugPrint(html_tree);
	$('#content').append(html_tree);
}

function DebugPrint(str)
{
    var out = document.getElementById("debug");
    if (!out) return;
    out.value += str;
}


function application_fill()
{
    $('#content').empty();
    $('#content').append($('<p><button id="appadd" class="btn btn-primary">Add new application</button></p>'));
    $.ajax({
        url: '/applications',
        type: 'POST',
        dataType: 'json',
        success: function(r) {
            application_fillList(r);
        }
    });

    $('#appadd').click(function() {
        $.post('/applications/new', function(data) {
            if (data.status == '1') {
                alert(data.mesg);
            }
            else {
                application_fill();
            }
        });
    });
}

function application_fillList(r)
{
    var i;
    var len = r.length;
    var m = $('#content');
    var obj, act;

    applist = $('<table id=applist></table>');
    for(i=0; i<len; i++) {
        var appentry = $('<tr class=listitem></tr>');

        var name = $('<td class=appname id=appname'+i+'></td>');
        //name.html('<a app_id="' + r[i].id + '" href="#">' + r[i].name + '</a>');
        name.html('<b app_id="' + r[i].id + '"><i>' + r[i].name + '</i></b>');

        var index = i;
        name.click(function() {
            var topbar;
            var app_id = $(this).find('b').attr('app_id');

            $('#content').empty();
            $('#content').block({
                message: '<h1>Processing</h1>',
                css: { border: '3px solid #a00' }
            });

            $.post('/applications/' + app_id, function(data) {
                if (data.status == 1) {
                    alert(data.mesg);
                    application_fill();
                } else {
                    // injecting script to create application interface
                    //content_scaffolding(data.topbar, $('<div class="img-rounded" style="height: 100%; padding: 10px;"><iframe width="100%" height="100%" src="/applications/' + app_id + '/fbp/load"></iframe></div>'));
                    //$('#content').html('<div id="topbar"></div><iframe width="100%" height="100%" src="/applications/' + app_id + '/fbp/load"></iframe>');

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
                            $('#content').unblock();
                        }
                    });
                }
            });
        });

        var act = $('<td class=appact id=appact'+i+'></td>');

        // solution against lazy evaluation
        var id = r[i].id;

        //var monitor = $('<button class=appmonitor id=appmonitor'+i+'></button>');
        //var deploy = $('<button class=appdeploy id=appdeploy'+i+'></button>');
        //var remove = $('<button class=appdel id=appdel'+i+'></button>');
        var remove = $('<button class=close id=appdel'+i+'>&times;</button>');

/*
        monitor.click(function() {
            $('#content').empty();
            var topbar;

            $('#content').block({
                message: '<h1>Processing</h1>',
                css: { border: '3px solid #a00' }
            });

            $.get('/applications/' + id, {title: "Monitoring"}, function(data) {
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
                            $('#content').unblock();
                        }
                    });
                }
            });
        });

        deploy.click(function() {
            $('#content').empty();
            var topbar;

            $('#content').block({
                message: '<h1>Processing</h1>',
                css: { border: '3px solid #a00' }
            });

            $.get('/applications/' + id, {title: "Deployment"}, function(data) {
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
                            $('#content').unblock();
                        }
                    });
                }
            });
        });
*/

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

        //act.append(monitor);
        //act.append(deploy);
        act.append(remove);

        appentry.append(name);
        appentry.append(act);
        applist.append(appentry);
        //application_setupButtons(i, r[i].id);
    }

    m.append(applist);
}

function content_scaffolding(topbar, editor)
{
    $('#content').append(topbar);
    $('#content').append(editor);
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

// might have to worry about multiple calls :P
function poll(url, version, options)
{
    var forceRepeat = false;
    if (typeof options != 'undefined') {
        // repeat is an object with one key 'repeat', or it will be pass-by-value and therefore can't be stopped
        forceRepeat = options.repeat;
    }

    console.log('poll');
    $.post(url, {version: version}, function(data) {
        _.each(data.logs, function(line) {
            if (line != '') {
                $('#log').append('<pre>' + line + '</pre>');
            }
        });

        // TODO:mapping_results too
        // TODO:node infos too

        if (forceRepeat) {
            console.log('repeat');
        } else {
            console.log('no repeat');
        }
        if (forceRepeat || data.returnCode < 0) {
            console.log("repeating");
            setTimeout(function() {
                poll(url, data.version, options);
            }, 1000);
        }
    });
}
