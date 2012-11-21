<?php

/**
 * Prepare Node
 */
function _parser_prepare_entity_node($entity, $entity_wrapper) {
  $entity->title = trim($entity->title);
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
  if (empty($entity->language)) {
    $entity->language = LANGUAGE_NONE;
  }
}

