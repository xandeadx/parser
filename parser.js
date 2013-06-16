(function ($) {
  Drupal.behaviors.parser = {
    attach: function (context, settings) {
      // Show result in popup window
      $('input.check-button:not(.check-button-processed)', context).click(function() {
        var $textarea = $(this).parent().next().find('textarea');
        var params = {
          test_url: $('#edit-test-url').val(),
          site_charset: $('#edit-site-charset').val(),
          charset_fix: $('#edit-charset-fix').attr('checked') ? 1 : 0,
          field_name: $textarea.attr('name'),
          field_value: $textarea.val(),
          list_mode_code: $('#edit-list-mode-code').val(),
          headers: $('#edit-headers').val()
        };
      
        $('<div id="parser-dialog">Загрузка...</div>').dialog({
          open: function() {
            $.post(Drupal.settings.parserCheckPath, params, function(response) {
              $('#parser-dialog').text($.trim(response));
            });
          },
          close: function() {
            $('#parser-dialog').remove();
          },
          title: 'Результат выполнения кода',
          width: 800,
          height: 400
        });
      }).addClass('check-button-processed');
    
      // CodeMirror init
      var codeMirrors = [];
      $('.form-textarea-wrapper:not(.resizable) > textarea:not(.codemirror-processed)', context).each(function() {
        var i = codeMirrors.length;
        codeMirrors[i] = CodeMirror.fromTextArea(this, {
          mode: 'text/x-php',
          matchBrackets: true,
          onBlur: function() {
            codeMirrors[i].save();
          }
        });
      }).addClass('codemirror-processed');

      // Refresh code mirror on open vertical tab
      $('.vertical-tab-button a:not(.codemirror-refreshed), .fieldset-legend a:not(.codemirror-refreshed)', context).click(function() {
        for (i in codeMirrors) {
          codeMirrors[i].refresh();
        }
      }).addClass('codemirror-refreshed');
    }
  };
}(jQuery));
