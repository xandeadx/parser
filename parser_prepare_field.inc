<?php

/**
 * Prepare field type "text"
 */
function _parser_prepare_field_text(&$result, $field_info, $field_instance, $entity, $job, $page_url, &$context_results) {
  $result = trim($result);
  if (!empty($field_info['settings']['max_length']) && drupal_strlen($result) > $field_info['settings']['max_length']) {
    $original_result = $result;
    $result = drupal_substr($result, 0, $field_info['settings']['max_length']);
    watchdog('parser', 'Обрезан текст в поле "' . $field_info['field_name'] . '": "<i>' . check_plain($original_result) . '</i>" => "<i>' . check_plain($result) . '</i>".');
  }
}

/**
 * Prepare field type "text_with_summary"
 */
function _parser_prepare_field_text_with_summary(&$result, $field_info, $field_instance, $entity, $job, $page_url, &$context_results) {
  if (!empty($result['value'])) {
    $result['value'] = trim($result['value']);
  }
  if (!empty($result['summary'])) {
    $result['summary'] = trim($result['summary']);
  }
  if (empty($result['format'])) {
    $result['format'] = 'filtered_html';
  }
}

/**
 * Prepare field type "taxonomy_term_reference"
 */
function _parser_prepare_field_taxonomy_term_reference(&$result, $field_info, $field_instance, $entity, $job, $page_url, &$context_results) {
  if (is_string($result)) {
    $voc_machine_name = $field_info['settings']['allowed_values'][0]['vocabulary'];
    $vid = taxonomy_vocabulary_machine_name_load($voc_machine_name)->vid;
    $term_name = trim($result);
    $term = current(taxonomy_term_load_multiple(array(), array('name' => $term_name, 'vid' => $vid)));
    if (!$term) {
      $term = (object)array('vid' => $vid, 'name' => $term_name);
      taxonomy_term_save($term);
    }
    $result = $term->tid;
  }
}

/**
 * Prepare field type "file"
 */
function _parser_prepare_field_file(&$result, $field_info, $field_instance, $entity, $job, $page_url, &$context_results) {
  if (is_numeric($result['file'])) {
    return;
  }
  
  $file_url = _parser_get_full_url($result['file'], $page_url);
  $file_name = drupal_basename($file_url);
  $file_ext = pathinfo($file_name, PATHINFO_EXTENSION);
  if (function_exists('transliteration_clean_filename')) {
    $file_name = transliteration_clean_filename($file_name);
  }
  if (drupal_strlen(pathinfo($file_name, PATHINFO_FILENAME)) > 64) {
    $file_name = drupal_substr($file_name, 0, 64) . '.' . $file_ext;
  }
  $dest_dir = file_field_widget_uri($field_info, $field_instance);
  file_prepare_directory($dest_dir, FILE_CREATE_DIRECTORY);
  
  $field_values = field_get_items($job->entity_type, $entity, $field_info['field_name']);
  if ($field_values) {
    foreach ($field_values as $field_value) {
      // If file already attached
      if ($field_value['filename'] == $file_name) {
        $result['file'] = $field_value['fid'];
        return;
      }
    }
  }
  
  // Validate by extensions
  $allowed_file_extensions = explode(' ', $field_instance['settings']['file_extensions']);
  if (!in_array($file_ext, $allowed_file_extensions)) {
    watchdog('parser', 'Файл <i>' . check_plain($file_url) . '</i> закачан не будет, так как его тип не в списке допустимых.');
    $result = NULL;
    return;
  }
  
  $cache_uri = 'public://parser_cache/files/' . md5($file_url) . '.' . $file_ext;
  if (!file_exists($cache_uri)) {
    // Download remote file
    $request_result = drupal_http_request($file_url);

    if (!empty($request_result->error)) {
      watchdog('parser', 'Ошибка при закачке файла. <b>Адрес</b>: ' . check_plain($file_url) . ', <b>Ошибка</b>: ' . $request_result->error . ', <b>Результат</b>: ' . check_plain($request_result->data));
      $result = NULL;
      return;
    }
    if (!$request_result->data) {
      watchdog('parser', 'Ошибка при закачке файла. <b>Адрес</b>: ' . check_plain($file_url) . ', <b>Ошибка</b>: сервер не возвратил данных.');
      $result = NULL;
      return;
    }
  
    // Save file to cache
    file_unmanaged_save_data($request_result->data, $cache_uri);
    $context_results['downloaded_files']++;
  }
  else {
    $context_results['files_from_cache']++;
  }
  
  // Copy file from cache to $dest_dir
  $file = (object)array(
    'uid' => $GLOBALS['user']->uid,
    'uri' => $cache_uri,
    'filename' => $file_name,
    'filemime' => file_get_mimetype($cache_uri),
  );
  $file_path = file_stream_wrapper_uri_normalize($dest_dir . '/' . $file_name);
  $file = file_copy($file, $file_path);
  if ($file) {
    $result['file'] = $file->fid;
  }
  else {
    $result = NULL;
    watchdog('parser', 'Ошибка при копировании файла <i>' . $file->uri  . '</i> в <i>' . $file_path . '</i>.');
  }
}

/**
 * Prepare field type "image"
 */
function _parser_prepare_field_image(&$result, $field_info, $field_instance, $entity, $job, $page_url, &$context_results) {
  _parser_prepare_field_file($result, $field_info, $field_instance, $entity, $job, $page_url, $context_results);
}

