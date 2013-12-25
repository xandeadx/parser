<?php

/**
 * Prepare field type "text"
 */
function _parser_prepare_field_text(&$result, $field_info, $field_instance, $entity, $job, $base_url) {
  if ($field_instance['settings']['text_processing']) {
    if (is_string($result)) {
      $result = array('value' => $result);
    }
    if (empty($result['format'])) {
      $result['format'] = 'filtered_html';
    }
    $result_value = &$result['value'];
  }
  else {
    $result_value = &$result;
  }

  $result_value = trim($result_value);
  if (!empty($field_info['settings']['max_length']) && drupal_strlen($result_value) > $field_info['settings']['max_length']) {
    $original_result = $result_value;
    $result_value = drupal_substr($result_value, 0, $field_info['settings']['max_length']);
    _parser_watchdog('
      Обрезан текст в поле "' . $field_info['field_name'] . '":
      "<i>' . check_plain($original_result) . '</i>" =>
      "<i>' . check_plain($result_value) . '</i>".
    ');
  }
}

/**
 * Prepare field type "text_long"
 */
function _parser_prepare_field_text_long(&$result, $field_info, $field_instance, $entity, $job, $base_url) {
  if (!$result) {
    $result = NULL;
    return;
  }
  if ($field_instance['settings']['text_processing']) {
    if (is_string($result)) {
      $result = array('value' => $result);
    }
    if (!empty($result['value'])) {
      $result['value'] = trim($result['value']);
    }
    if (empty($result['format'])) {
      $result['format'] = 'filtered_html';
    }
  }
}

/**
 * Prepare field type "text_with_summary"
 */
function _parser_prepare_field_text_with_summary(&$result, $field_info, $field_instance, $entity, $job, $base_url) {
  _parser_prepare_field_text_long($result, $field_info, $field_instance, $entity, $job, $base_url);
  
  if (!empty($result['summary'])) {
    $result['summary'] = trim($result['summary']);
  }
}

/**
 * Prepare field type "number_integer"
 */
function _parser_prepare_field_number_integer(&$result, $field_info, $field_instance, $entity, $job, $base_url) {
  if (!is_integer($result)) {
    $result = trim($result);
    $result = (int)$result;
  }
}

/**
 * Prepare field type "number_float"
 */
function _parser_prepare_field_number_float(&$result, $field_info, $field_instance, $entity, $job, $base_url) {
  if (!is_float($result)) {
    $result = trim($result);
    $result = (float)$result;
  }
}

/**
 * Prepare field type "taxonomy_term_reference"
 */
function _parser_prepare_field_taxonomy_term_reference(&$result, $field_info, $field_instance, $entity, $job, $base_url) {
  if (is_string($result)) {
    $term_name = trim($result);
    
    if (!$term_name) {
      $result = NULL;
      return;
    }

    $voc_machine_name = $field_info['settings']['allowed_values'][0]['vocabulary'];
    $vid = taxonomy_vocabulary_machine_name_load($voc_machine_name)->vid;
    $term = parser_get_term_by_name($term_name, $vid);
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
function _parser_prepare_field_file(&$result, $field_info, $field_instance, $entity, $job, $base_url) {
  if (!$result) {
    $result = NULL;
    return;
  }
  if (is_string($result)) {
    $result = array('file' => $result);
  }
  if (!$result['file']) {
    $result = NULL;
    return;
  }
  if (is_numeric($result['file'])) {
    return;
  }
  
  $file_url = parser_get_absolute_url($base_url, $result['file']);
  $file_name = _parser_get_file_name_from_url($file_url);
  $file_ext = _parser_get_file_extension_from_url($file_name);
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
  if ($allowed_file_extensions && !in_array($file_ext, $allowed_file_extensions)) {
    _parser_watchdog('Файл "<i>' . check_plain($file_url) . '</i>" закачан не будет, так как его тип "<i>' . check_plain($file_ext) . '</i>" не в списке допустимых.');
    $result = NULL;
    return;
  }
  
  // Download file to cache
  $cache_uri = _parser_download_url_to_cache($file_url, $job->headers, FALSE);
  if (!$cache_uri) {
    $result = NULL;
    return;
  }
  
  // Copy file from cache to $dest_dir
  $file = _parser_copy_file($cache_uri, $dest_dir . '/' . $file_name);
  if ($file) {
    $result['file'] = $file->fid;
  }
  else {
    $result = NULL;
  }
}

/**
 * Prepare field type "image"
 */
function _parser_prepare_field_image(&$result, $field_info, $field_instance, $entity, $job, $base_url) {
  _parser_prepare_field_file($result, $field_info, $field_instance, $entity, $job, $base_url);
}
