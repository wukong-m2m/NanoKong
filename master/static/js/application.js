// vim: ts=4 sw=4
$(document).ready(function() {
	application_init();
});

function application_init()
{
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
		method: 'POST',
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
	var obj;

	obj = $('#menu');
	obj.empty();
	obj.append('<button id=appadd class=appadd></button>')
	m.empty();
	m.html('<table id=applist></table>');
	applist = $('#applist');
	for(i=0;i<len;i++) {
		if ((i % 2) == 0) {
			applist.append('<tr class=listitem_even><td class=appname id=appname'+i+'></td><td id=appact'+i+'></td></tr>');
		} else {
			applist.append('<tr class=listitem_odd><td class=appname id=appname'+i+'></td><td class=appact id=appact'+i+'></td></tr>');
		}
		$('#appact'+i).append('<button class=appmonitor id=appmonitor'+i+'></button>');
		$('#appact'+i).append('<button class=appdeploy id=appdeploy'+i+'></button>');
		$('#appact'+i).append('<button class=appdel id=appdel'+i+'></button>');
		obj = $('#appname'+i);
		obj.html(r[i].name);
		obj.click(function() {
			
		});
		application_setupButtons(i);
	}
}

function application_setupButtons(i)
{
		$('#appmonitor'+i).click(function() {
				alert("not implemented yet");
		});
		$('#appdel'+i).click(function() {
				alert("not implemented yet");
		});
		$('#appdeploy'+i).click(function() {
				alert("not implemented yet");
		});
}
