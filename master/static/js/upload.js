$(function() {
  $('#form1').submit(function() {
    console.log('submitting...');
    $(this).ajaxSubmit({
      beforeSubmit: function() {
        $('#status ul').append($('<li>uploading...</li>'));
      },
      success: function() {
        queryStatus();
      }
    });
    return false;
  });

  queryStatus = function() {
    $.ajax({
      type: 'POST',
      url: '/status',
      async: true,
      cache: false,
      success: function(data) {
        console.log(data);
        if (data.status == 0) {
          if (data.current_status == 0) {
            setTimeout('queryStatus()', 10);
          } else if (data.current_status == 1) {
            $('#status ul').empty();
            $('#status ul').append($('<li>waiting...</li>'));
            $('#status ul').append($('<li>uploading...</li>'));
            $('#status ul').append($('<li>extracting...</li>'));
            setTimeout('queryStatus()', 10);
          } else if (data.current_status == 2) {
            $('#status ul').empty();
            $('#status ul').append($('<li>waiting...</li>'));
            $('#status ul').append($('<li>uploading...</li>'));
            $('#status ul').append($('<li>extracting...</li>'));
            $('#status ul').append($('<li>converting...</li>'));
            setTimeout('queryStatus()', 10);
          } else if (data.current_status == 3) {
            $('#status ul').empty();
            $('#status ul').append($('<li>waiting...</li>'));
            $('#status ul').append($('<li>uploading...</li>'));
            $('#status ul').append($('<li>extracting...</li>'));
            $('#status ul').append($('<li>converting...</li>'));
            $('#status ul').append($('<li>compiling...</li>'));
            setTimeout('queryStatus()', 10);
          } else {
            $('#status ul').empty();
            $('#status ul').append($('<li>waiting...</li>'));
            $('#status ul').append($('<li>uploading...</li>'));
            $('#status ul').append($('<li>extracting...</li>'));
            $('#status ul').append($('<li>converting...</li>'));
            $('#status ul').append($('<li>compiling...</li>'));
            $('#status ul').append($('<li>done...</li>'));
          }
        } else {
          setTimeout('queryStatus()', 10);
        }
      },
      error: function(request, status, error) {
        console.log('error thrown: ' + error);
        setTimeout('queryStatus()', 10);

      }
    });
  }

});
