// vim: ts=4 sw=4

window.polling = null;

$(document).ready(function() {
    init();
});

function init()
{
    window.options = {repeat: false};
    // Top bar
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
        $.post('/loc_tree', function(data) {
	    		display_tree(data);
	    		$('#content').append(data.node);
	    		load_landmark(data.xml);
		    });                    
    });
    
    application_fill();
    
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

    applist = $('<table id=applist></table>');
    for(i=0; i<len; i++) {
        // Html elements
        var appentry = $('<tr class=listitem></tr>');
        var name = $('<td class=appname data-app_id="' + r[i].id + '" id=appname'+i+'><b><i>' + r[i].name + '</i></b></td>');
        var act = $('<td class=appact id=appact'+i+'></td>');

        // Acts
        //var monitor = $('<button class=appmonitor data-app_id="' + r[i].id + '" id=appmonitor'+i+'></button>');
        //var deploy = $('<button class=appdeploy data-app_id="' + r[i].id + '" id=appdeploy'+i+'></button>');
        //var remove = $('<button class=appdel data-app_id="' + r[i].id + '" id=appdel'+i+'></button>');
        var remove = $('<button class=close data-app_id="' + r[i].id + '" id=appdel'+i+'>&times;</button>');

        // Enter application
        name.click(function() {
            var topbar;
            var app_id = $(this).data('app_id');

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
                    $.get('/applications/' + app_id + '/deploy', function(data) {
                        if (data.status == 1) {
                            alert(data.mesg);
                            application_fill();
                        } else {
                            // injecting script to create application interface
                            page = $(data.page);
                            console.log(page);
                            content_scaffolding(topbar, page);
                            $('#content').unblock();

                            // start polling
                            window.options = {repeat: true};
                            $('#deploy_results').dialog({modal: true, autoOpen: true, width: 600, height: 300}).dialog('open');
                            $('#deploy_results #wukong_status').text('Waiting from master');
                            $('#deploy_results #application_status').text("");

                            poll('/applications/' + current_application + '/poll', 0, window.options, function(data) {
                                console.log(data)
                                if (data.wukong_status == "" && data.application_status == "") {
                                    $('#deploy_results').dialog('destroy').remove();
                                } else {
                                    $('#deploy_results').dialog({modal: true, autoOpen: true, width: 600, height: 300}).dialog('open');
                                }

                                $('#deploy_results #wukong_status').text(data.wukong_status);
                                $('#deploy_results #application_status').text(data.application_status);

                            });
                        }
                    });
                }
            });
        });


        remove.click(function() {
            var app_id = $(this).data('app_id');
            $.ajax({
                type: 'delete',
                url: '/applications/' + app_id,
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

function application_polling(app_id)
{
    // start polling
    window.options = {repeat: true};
    $('#deploy_results').dialog({modal: true, autoOpen: true, width: 600, height: 300}).dialog('open');
    $('#deploy_results #wukong_status').text('Waiting from master');
    $('#deploy_results #application_status').text("");

    poll('/applications/' + app_id + '/poll', 0, window.options, function(data) {
        console.log(data)
        if (data.wukong_status == "" && data.application_status == "") {
            $('#deploy_results').dialog('destroy');
        } else {
            $('#deploy_results').dialog({modal: true, autoOpen: true, width: 600, height: 300}).dialog('open');
        }

        $('#deploy_results #wukong_status').text(data.wukong_status);
        $('#deploy_results #application_status').text(data.application_status);

    });
}

function content_scaffolding(topbar, editor)
{
    $('#content').append(topbar);
    $('#content').append(editor);
}


// might have to worry about multiple calls :P
function poll(url, version, options, callback)
{
    var forceRepeat = false;
    if (typeof options != 'undefined') {
        // repeat is an object with one key 'repeat', or it will be pass-by-value and therefore can't be stopped
        forceRepeat = options.repeat;
    }

    console.log('polling');
    $.post(url, {version: version}, function(data) {
        if (typeof callback != 'undefined') {
            callback(data)
        }

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
                poll(url, data.version, options, callback);
            }, 1000);
        }
        if (typeof callback != 'undefined') {
            callback(data)
        }

        _.each(data.logs, function(line) {
            if (line != '') {
                $('#log').append('<pre>' + line + '</pre>');
            }
        });
        window.polling = null;
    });
}


function display_tree(rt)
{
	$('#content').empty();
	$('#content').append('<script type="text/javascript" src="/static/js/jquery.treeview.js"></script>'+
		'<script type="text/javascript" src="/static/js/tree_expand.js"></script>'+
		'<script type="text/javascript" src="/static/js/jquery.cookie.js"></script>'
		);

	var node_data = JSON.parse(rt.loc);
    var tree_level = 0;
    var html_tree = '';
    html_tree = '<table width="100%">';
    html_tree += '<tr><td width="5%"></td><td></td><td></td></tr>';
    html_tree += '<tr><td></td><td>';
    html_tree += '<ul id="display" class="treeview">';
    for(var i=0; i<node_data.length;i++){
		  if(node_data[i][1] == tree_level){
			  //do nothing
		  }else if(node_data[i][1] > tree_level){
		    html_tree +='<ul>';
		    tree_level = node_data[i][1];
	    }else if (node_data[i][1]<tree_level){
	      for(var j=0; j<tree_level-node_data[i][1] ;j++){
				  html_tree += '</ul></li>';
			  }
			  tree_level = node_data[i][1];
	    }
	    //see locationTree.py, class locationTree.toJason() function for detailed data format
		  if(node_data[i][0] == 0){ //this is a tree node
		    if (node_data[i][1] ==0){  //root
		          html_tree += '<li id="'+ node_data[i][2][1] +'"><button class="locTreeNode" id='+node_data[i][2][0]+'>'+node_data[i][2][1]+'</button>';
            }else{
                for (var j=i+1;j<node_data.length;++j){
  		            if (node_data[j][1]==node_data[i][1]){
  		                html_tree += '<li  id="'+ node_data[i][2][1] +'"><button class="locTreeNode" id='+node_data[i][2][0]+'>'+node_data[i][2][1]+'</button>';
  		                break;
  		            }
  		            if (node_data[j][1]<node_data[i][1]){
  		                html_tree += '<li  id="'+ node_data[i][2][1] +'"><button class="locTreeNode" id='+node_data[i][2][0]+'>'+node_data[i][2][1]+'</button>';
  		                break;
  		            }
  		            if (j==node_data.length-1){
  		                html_tree += '<li id="'+ node_data[i][2][1] +'"><button class="locTreeNode" id='+node_data[i][2][0]+'>'+node_data[i][2][1]+'</button>';
                    }
  	            }
	        }
		  }else if (node_data[i][0] == 1){            //this is a sensor 
				html_tree += '<li id=se'+node_data[i][2][0]+' data-toggle=modal  role=button class="btn" >'+node_data[i][2][0]+node_data[i][2][1]+'</li>';
		  }else if(node_data[i][0]==2){               //this is a landmark
				html_tree += '<li id=lm'+node_data[i][2][1]+' data-toggle=modal  role=button class="btn" >'+node_data[i][2][0]+node_data[i][2][1]+'</li>';
		  }
		}
	html_tree += '</td><td valign="top">';
	html_tree += '<button id="saveTree">SAVE Landmarks</button>'
				 +'<button id="addNode">ADD Landmark</button>'
				 +'<button id="delNode">DEL Landmark</button>'
				 +'<button type="button" class="set_node">Save Node Configuration</button><br>'
				 +'type <div id="nodeType"></div><br>'
				 +'ID <input id="SensorId" type=text size="10"><br>'
				 +'Location <input id="locName" type=text size="100"><br>'
				 +'Local Coordinate <input id="localCoord" type=text size="100"><br>'
				 +'Global Coordinate <input id="gloCoord" type=text size="100"><br>'
				 +'Size <input id = "size" type=text size="100"><br>'
				 +'Direction <input id = "direction" type=text size="100"><br>'
				 +'Distance Modifier <button id="addModifier">Add Modifier</button> ' 
				 +'<button id="delModifier">Delete Modifier</button><br>' 
				 + 'start treenode ID<input id = "distmod_start_id" type=text size="20"><br>'
				 + 'end treenode ID<input id = "distmod_end_id" type=text size="20"><br>'
				 + 'distance<input id = "distmod_distance" type=text size="20"><br>'
				 + 'Existing Modifiers <div id="distmod_list"></div><br>';
	html_tree += 'Add/Del Object <input id="node_addDel" type=text size="50"><br>';
//	html_tree += 'add/del location <input id="loc_addDel" type=text size="50">'
	html_tree += '</td></tr></table>';
	$('#content').append(html_tree);
	$("#display").treeview({
  collapsed: true,
  unique: true});
}

