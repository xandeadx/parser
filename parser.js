(function ($) {
  $(function(){
    $('input.check-button').click(function() {
      var textarea = $(this).parent().next().find('textarea');
      var params = {
        test_url: $('#edit-test-url').val(),
        field_name: textarea.attr('name'),
        field_value: textarea.val()
      };
      
      $('<div id="parser-dialog">Загрузка...</div>').dialog({
        open: function() {
          $.post(Drupal.settings.basePath + 'parser/check', params, function(response) {
            $('#parser-dialog').html('<pre>' + response + '</pre>');
          });
        },
        close: function() {
          $('#parser-dialog').remove();
        },
        title: 'Результат выполнения кода',
        width: 800,
        height: 400
      });
    });
  });
}(jQuery));
