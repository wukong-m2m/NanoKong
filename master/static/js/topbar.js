$(function() {
  $('#save').click(function() {
    $.ajax({
      type: 'put',
      url: '/application/' + current_application, 
      data: {name: $('#name').val(), desc: $('#desc').val()}, 
      success: function(data) {
        if (data.status == 1) {
          alert(data.mesg);
        }
      }
    });
  });
});
