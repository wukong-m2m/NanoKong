// vim: ts=4 sw=4
var FBP_canvas;
var FBP_CANVAS_TOP=50;
var g_lines=[];
var g_nodes=[];
$(document).ready(function() {
	$('#client').append('<div id=toolbar></div>');
	$('#client').append('<div id=content></div>');
	var toolbar = $('#toolbar');
	toolbar.append('<select id=toolbar_type></select>');
	FBP_fillBlockType($('#toolbar_type'));
	toolbar.append('<button id=toolbar_addBlock>Add</button>');
	$('#toolbar_addBlock').click(function() {
		FBP_addBlock();
	});
	toolbar.append('<button id=toolbar_delBlock>Del</button>');
	$('#toolbar_delBlock').click(function() {
		FBP_delBlock();
	});
	toolbar.append('<button id=toolbar_link>Link</button>');
	$('#toolbar_link').click(function() {
		FBP_link();
	});
	toolbar.append('<button id=toolbar_save>Save</button>');
	$('#toolbar_save').click(function() {
		FBP_save();
	});
	toolbar.append('<button id=toolbar_load>Load</button>');
	$('#toolbar_load').click(function() {
		FBP_load();
	});
	toolbar.append('<button id=toolbar_activate>Activate</button>');
	$('#toolbar_activate').click(function() {
		FBP_activate(g_nodes,g_lines);
	});
	FBP_canvastop=$('#canvastop');
	FBP_canvas=$('#canvas');
	$('#canvastop').hide();
	$('#connection').dialog({autoOpen: false});
	$('#canvas').mousedown(function(e) {
		var px = e.pageX;
		var py = e.pageY;
		var l = Line_search(g_lines,px,py);
		if (l != null) {
			$('#msg').html(l.toString());
		} else {
			$('#msg').html('');
		}
		$('#msg').css('left',px).css('top',py).show();
		setTimeout(function() {
			$('#msg').hide();
		},2000);
	});
	$('#fileloader').dialog({autoOpen:false});
	$('#fileloader_file').val('fbp.sce');
});

function FBP_fillBlockType(div)
{
	var blocks = Block.getBlockTypes();
	var i;

	for(i=0;i<blocks.length;i++) {
		div.append('<option val='+blocks[i]+'>'+blocks[i]+'</option>');
	}
}

function FBP_addBlock()
{
	var type = $('#toolbar_type').val();
	var block = Block.factory(type);
	// This should be replaced with the node type system latter
	block.attach($('#content'));
	g_nodes.push(block);
}

function FBP_delBlock()
{
	alert("not implemented yet");
}

function FBP_refreshLines()
{
	FBP_canvas.clearCanvas();
	for(i=0;i<g_lines.length;i++) {
		g_lines[i].draw(FBP_canvas);
	}
}
function FBP_buildConnection(source, obj)
{
	var i;
	var list = source.getSignals();

	$('#connection_src').empty();
	for(i=0;i<list.length;i++) {
		var sig = list[i];
		$('#connection_src').append('<option value='+i+'>'+sig.name+'</option>');
	}
	list = obj.getActions();
	$('#connection_act').empty();
	for(i=0;i<list.length;i++) {
		var act = list[i];
		$('#connection_act').append('<option value='+i+'>'+act.name+'</option>');
	}
	$('#connection').dialog({
		buttons: {
			'Connect': function() {
				var sig = $('#connection_src').val();
				var act = $('#connection_act').val();
				var l = new Line(source,source.signals[sig].name,obj,obj.actions[act].name);
				g_lines.push(l);
				FBP_refreshLines();
				$('#connection').dialog("close");
			},
			'Cancel': function() {
				$('#connection').dialog("close");
			}
		}
	}).dialog("open");

}

function FBP_link()
{
	if (Block.current == null) {
		alert("select a source first");
		return;
	}
	FBP_source = Block.current;
	var x1,y1,x2,y2;

	var pos = FBP_source.getPosition();
	var size = FBP_source.getSize();
	x1 = pos[0]+size[0]/2;
	y1 = pos[1]+size[1]/2-FBP_CANVAS_TOP;
	$('#canvastop').show();

	$('#canvastop').mousemove(function(e) {
		x2 = e.pageX;
		y2 = e.pageY-FBP_CANVAS_TOP;
		FBP_canvastop.clearCanvas().drawLine({
			strokeStyle: "#000",
			strokeWidth: 4,
			x1: x1, y1:y1,
			x2: x2, y2:y2
		});
	});
	$('#canvastop').mousedown(function(e) {
		var obj = Block.hitTest(e.pageX,e.pageY);
		if (obj) {
			$('#canvastop').unbind();
			$('#canvastop').hide();
			FBP_buildConnection(FBP_source, obj);
		}
		FBP_source = null;
	});
}
function FBP_save()
{
	data = JSON.stringify(FBP_serialize(g_nodes,g_lines));
	$.ajax({
		url:'/cgi-bin/file?save=fbp.sce',
		data: data,
		type:'POST'
	});
}

function FBP_load()
{
	$('#fileloader').dialog({buttons: {
			'OK': function() {
				g_filename = $('#fileloader_file').val();
				$.ajax({
					url:'/cgi-bin/file?load='+$('#fileloader_file').val(),
					success: function(r) {
						meta = JSON.parse(r);
						$('#content').empty();
						g_nodes = [];
						g_lines = [];
						var hash={};
						for(i=0;i<meta.nodes.length;i++) {
							n = Block.restore(meta.nodes[i]);
							// This should be replaced with the node type system latter
							n.attach($('#content'));
							hash[n.id] = n;
							g_nodes.push(n);
						}
						for(i=0;i<meta.lines.length;i++) {
							var a = meta.lines[i];
							a.source = hash[a.source];
							a.dest = hash[a.dest];
							g_lines.push(Line.restore(a));
						}
						FBP_refreshLines();
					}
				});
				$('#fileloader').dialog("close");
			},
			'Cancel': function() {
				$('#fileloader').dialog("close");
			}
	}});
	$('#fileloader').dialog("open");
}


function FBP_serialize(gnodes,glines)
{
	var i;
	var len = gnodes.length;
	var meta={};
	var nodes = [];
	for(i=0;i<len;i++) {
		nodes.push(gnodes[i].serialize({}));
	}
	meta['nodes'] = nodes;
	len = glines.length;
	var lines=[];
	for(i=0;i<len;i++) {
		lines.push(glines[i].serialize());
	}
	meta['lines'] = lines;
	return meta;
}
function FBP_activate(gnodes,glines)
{
	var linehash={};
	var i;
	var len = glines.length;

	for(i=0;i<len;i++) {
		if (linehash[glines[i].source.id]) {
			linehash[glines[i].source.id].push(glines[i]);
		} else {
			linehash[glines[i].source.id] =[(glines[i])];
		}
	}
	var xml='<application name="'+g_filename+'">\n';
	for(var k in linehash) {
		var source = linehash[k][0].source;
		xml = xml + '    <component type="'+source.type+'" instanceId="'+source.id+'">\n';
		len = linehash[k].length;
		for(i=0;i<len;i++) {
			var line = linehash[k][i];
			xml = xml + '        <link fromProperty="'+line.signal+'" toInstanceId="'+line.dest.id+'" toProperty="'+line.action+'"/>\n';
		}
		xml = xml + '    </component>\n';
	}
	alert(xml);
}

function Signal(name)
{
	this.name = name;
}

function Action(name)
{
	this.name = name;
}


