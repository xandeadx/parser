<?php

/**
 * Prepare Taxonomy term
 */
function _parser_prepare_entity_taxonomy_term($entity, $entity_wrapper) {
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

