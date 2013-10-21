<?php

/**
 * Prepare Node
 */
function _parser_prepare_entity_node($entity, $entity_wrapper) {
  $entity->title = trim($entity->title);

  if (drupal_strlen($entity->title) > 255) {
    $title = $entity->title;
    $entity->title = drupal_substr($title, 0, 255);
    _parser_watchdog('Обрезан заголовок ноды: "<i>' . check_plain($title) . '</i>" => "<i>' . check_plain($entity->title) . '</i>".');
  }
}

/**
 * Prepare Taxonomy term
 */
function _parser_prepare_entity_taxonomy_term($entity, $entity_wrapper) {
  $entity->name = trim($entity->name);

  if (empty($entity->vid)) {
    $entity->vid = taxonomy_vocabulary_machine_name_load($entity->vocabulary_machine_name)->vid;
  }
}

/**
 * Prepare Commerce product
 */
function _parser_prepare_entity_commerce_product($entity, $entity_wrapper) {
  if (drupal_strlen($entity->title) > 255) {
    $title = $entity->title;
    $entity->title = drupal_substr($title, 0, 255);
    _parser_watchdog('Обрезан заголовок товара: "<i>' . check_plain($title) . '</i>" => "<i>' . check_plain($entity->title) . '</i>".');
  }

  if (empty($entity->language)) {
    $entity->language = LANGUAGE_NONE;
  }
}

