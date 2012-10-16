$(function() {
    window.options = {repeat: true};

    $('a#nodes-btn').click(function(e) {
        e.preventDefault();
        $(this).tab('show');

        console.log('refresh nodes');
        /*
        $('#nodes').block({
            message: '<h1>Processing</h1>',
            css: { border: '3px solid #a00' }
        });
        */
        $.post('/nodes/refresh', function(data) {
            //$('#nodes').unblock();
            $('#nodes').html(data.nodes);
        });
    });

    $('a#mapping_results-btn').click(function() {
        $.post('/applications/' + current_application + '/deploy/map', function(data) {
            // Already an object
            console.log(data)
            if (data.status == 1) {
                alert(data.mesg);
            } else {
                var $table = $('#mapping_results table tbody');
                $table.empty();

                console.log(data.mapping_results);
                if (data.mapping_results) {
                    $('#deploy').removeAttr('disabled');
                }
                else {
                    $('#deploy').attr('disabled', 'disabled');
                }

                _.each(data.mapping_results, function(result) {
                    var compiled = _.template('<tr class=success><td><%= instanceId %></td><td><%= name %></td><td><%= nodeId %></td><td><%= portNumber %></td></tr>');
                    $table.append(compiled(result));
                });

                poll('/applications/' + current_application + '/poll', 0, window.options);
            }
        });
    });

    // Actually deploy
    $('a#log-btn').click(function() {
        e.preventDefault();
        $(this).tab('show');

        $.post('/applications/' + current_application + '/deploy', function(data) {
            // Already an object
            console.log(data)
            if (data.status == 1) {
                alert(data.mesg);
            } else {
                poll('/applications/' + current_application + '/poll', 0, window.options);
            }
        });
    });
});
