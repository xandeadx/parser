<?php

/**
 * Job form.
 */
function parser_job_form($form, &$form_state, $job) {
  $check_button = array('#field_prefix' => '<input type="button" value="проверить" class="form-submit check-button" />');
  $fields_code = isset($job->fields_code) ? $job->fields_code : array();
  $entity_types = entity_get_info();
  $fields_info = field_info_fields();
  $formats = filter_formats();
  $weight = 0;
  
  $entity_type_value = isset($job->entity_type) ? $job->entity_type : 'node';
  if (isset($form_state['values']['entity_type'])) {
    $entity_type_value = $form_state['values']['entity_type'];
  }
  
  $bundle_options = _parser_get_bundle_options($entity_types[$entity_type_value]['bundles']);
  $bundle_value = (isset($job->bundle) && isset($bundle_options[$job->bundle])) ? $job->bundle : key($bundle_options);
  if (isset($form_state['values']['bundle']) && isset($bundle_options[$form_state['values']['bundle']])) {
    $bundle_value = $form_state['values']['bundle'];
  }
  
  $entity_keys = $entity_types[$entity_type_value]['entity keys'];
  $bundle_key = $entity_keys['bundle'];
  $label_key = isset($entity_keys['label']) ? $entity_keys['label'] : '';
  
  list($entity_properties, $entity_properties_fields) = _parser_get_entity_properties($entity_type_value, $bundle_value);
  if ($entity_type_value == 'taxonomy_term') {
    unset($entity_properties['vocabulary']);
  }
  if (isset($entity_properties[$bundle_key])) {
    unset($entity_properties[$bundle_key]);
  }
  
  $fields_instances = field_info_instances($entity_type_value, $bundle_value);
  
  $form['title'] = array(
    '#type' => 'textfield',
    '#title' => 'Название задания',
    '#description' => 'Будет отображаться в списке с заданиями. Пример заполнения: <code>Парсинг проектов на drupal.org</code>',
    '#required' => TRUE,
    '#default_value' => isset($job->title) ? $job->title : '',
    '#maxlength' => 255,
  );
  
  $form['start_url'] = array(
    '#type' => 'textarea',
    '#title' => 'Стартовый URL',
    '#description' => '
      Адрес, с которого парсер начнёт работу. Пример заполнения: <code>https://drupal.org/project/project_theme</code><br />
      В адресе можно использовать маску для числовых значений в формате <code>[mask:min,max]</code>, например: <code>https://drupal.org/project/project_theme?page=[mask:0,49]</code><br />
      В поле можно указать несколько адресов, каждый на новой строке.
    ',
    '#required' => TRUE,
    '#default_value' => isset($job->start_url) ? $job->start_url : '',
  );
  
  $form['test_url'] = array(
    '#type' => 'textfield',
    '#title' => 'URL тестовой страницы',
    '#description' => '
      Адрес любой страницы, попадающей под условие парсинга. Страница будет использоваться для проверки
      работоспособности кода. Пример заполнения: <code>http://drupal.org/project/zen</code>
    ',
    '#required' => TRUE,
    '#default_value' => isset($job->test_url) ? $job->test_url : '',
    '#maxlength' => 255,
  );
  
  $form['depth'] = array(
    '#type' => 'textfield',
    '#title' => 'Глубина',
    '#description' => '
      Глубина, на которую парсер будет ходить по ссылкам. Например 1 означает, что будет распарсен стартовый адрес и
      страницы на которые он ссылается, т.е. парсер пройдёт вглубь на 1 уровень относительно стартовой страницы.
      Укажите 0, если хотите распарсить только стартовый URL.
    ',
    '#required' => TRUE,
    '#default_value' => isset($job->depth) ? $job->depth : '',
    '#size' => 5,
  );
  
  $form['white_list'] = array(
    '#type' => 'textarea',
    '#title' => 'Белый список адресов',
    '#description' => '
      Маски адресов, по которым будет разрешено ходить парсеру. Каждая маска на новой строке. Оставьте пустым, если
      парсеру разрешено ходить по всем найденным ссылкам. Пример заполнения: <code>http://drupal.org/project/*</code>
    ',
    '#default_value' => isset($job->white_list) ? $job->white_list : '',
    '#rows' => 3,
  );

  $form['black_list'] = array(
    '#type' => 'textarea',
    '#title' => 'Чёрный список адресов',
    '#description' => '
      Маски адресов, по которым будет запрещено ходить парсеру. Каждая маска на новой строке. Пример заполнения:
      <code>http://drupal.org/project/uc_*</code>
    ',
    '#default_value' => isset($job->black_list) ? $job->black_list : implode("\n", array('*.jpg', '*.png', '*.zip', '*.rar', '*.pdf')),
    '#rows' => 3,
  );
  
  $form['check_code'] = array(
    '#type' => 'textarea',
    '#title' => 'Код проверки для дальнейшего парсинга страницы',
    '#description' => '
      PHP код без тегов &lt;?php и ?&gt;, который должен вернуть TRUE если страницу нужно парсить в сущность.
      Доступные переменные: $page - html код страницы, $doc - объект phpQuery, $page_url - адрес страницы без учёта редиректов.
      Пример заполнения: <code>return ($doc->find(\'.class-name\')->length() > 0);</code>
    ',
    '#default_value' => isset($job->check_code) ? $job->check_code : '',
    '#rows' => 3,
    '#resizable' => FALSE,
  ) + $check_button;
  
  // Entity type
  $form['entity_type'] = array(
    '#type' => 'select',
    '#title' => 'Тип сущности (entity type)',
    '#description' => 'Выберите тип сущности, который будет создавать парсер.',
    '#options' => _parser_get_entity_type_options($entity_types),
    '#default_value' => isset($job->entity_type) ? $job->entity_type : 'node',
    '#required' => TRUE,
    '#ajax' => array('callback' => 'parser_job_form_entity_type_ajax_callback'),
    '#disabled' => isset($job->jid) && _parser_get_entity_count($job->jid) ? TRUE : FALSE,
  );
  
  // Bundle
  $form['bundle'] = array(
    '#type' => 'select',
    '#title' => 'Подтип сущности (bundle)',
    '#description' => 'Выберите подтип сущности, который будет создавать парсер.',
    '#options' => $bundle_options,
    '#default_value' => isset($job->bundle) ? $job->bundle : key($bundle_options),
    '#required' => TRUE,
    '#prefix' => '<div id="edit-bundle-wrapper">',
    '#suffix' => '</div>',
    '#ajax' => array(
      'callback' => 'parser_job_form_fields_ajax_callback',
      'wrapper' => 'fields-code-wrapper',
    ),
  );
  
  // Remote ID
  $form['remote_id_fieldset'] = array(
    '#type' => 'fieldset',
    '#title' => 'Remote ID',
    '#description' => '
      ID сущности на сайте источнике.
      Этот ID будет сохранён в бд (таблица parser_map) и позволит избежать дублей сущностей, доступных по нескольким URL.
      Так же, зная этот ID, можно с помощью функции parser_get_entity_id_by_remote_id() получить ID созданной сущности.
      Поле является не обязательным, но крайне рекомендованное к заполнению.
    ',
    '#weight' => -1,
    '#group' => 'vertical_tabs',
  );
  
  $form['remote_id_fieldset']['remote_id_code'] = array(
    '#type' => 'textarea',
    '#title' => 'PHP код',
    '#description' => 'PHP код, который должен вернуть число или строку (не длиннее 255 символов).',
    '#default_value' => isset($job->remote_id_code) ? $job->remote_id_code : '',
    '#rows' => 3,
    '#resizable' => FALSE,
  ) + $check_button;
  
  $form['fields_wrapper'] = array(
    '#prefix' => '<div id="fields-code-wrapper">',
    '#suffix' => '</div>',
    '#type' => 'fieldset',
    '#title' => 'Поля',
    '#description' => '
      <p>Поля заполняются без обёртки кода в &lt;?php и ?&gt;. Во всех полях доступны следующие переменные:</p>
      <p>
        $doc - объект phpQuery для страницы<br />
        $page - html код страницы<br />
        $page_url - адрес страницы без учёта редиректов<br />
        $entity - объект сущности<br />
        $entity_wrapper - entity metadata wrapper<br />
        $doc_element - объект phpQuery для элемента (только если включён режим списка)<br />
        $element_html - html код элемента (только если включён режим списка)<br />
      </p>
      <p>Оставьте textarea пустым или верните NULL, если поле заполнять не нужно.</p>
      <p>
        Полезные функции:<br />
        <code>parser_get_pqelements_values($doc, $selector, $function = \'html\', $map = \'trim\')</code>
        - возвращает массив значений элементов.<br />
        <code>parser_get_entity_id_by_remote_id($remote_id, $job_id = NULL)</code>
        - возвращает id созданной сущности по Remote ID.<br />
        <code>parser_get_entity_by_conditions($entity_type, array $properties = NULL, array $fields = NULL)</code>
        - возвращает объект сущности по значениям её свойств и полей.<br />
      </p>
    ',
  );
  
  $form['fields_wrapper']['vertical_tabs'] = array('#type' => 'vertical_tabs');
  $form['fields_wrapper']['fields_code'] = array('#tree' => TRUE);
    
  // Properties & Fields
  foreach (($entity_properties + $entity_properties_fields) as $property_name => $property_info) {
    $this_property_is_field = isset($property_info['field']);

    if (
      !empty($property_info['computed']) ||
      !isset($property_info['setter callback']) ||
      ($this_property_is_field && !isset($fields_info[$property_name]))
    ) {
      continue;
    }

    $fieldset_name = $property_name . '_fieldset';
    $field_type = isset($property_info['type']) ? $property_info['type'] : '';
    $this_property_is_multi_value = ($this_property_is_field && $fields_info[$property_name]['cardinality'] != 1);

    if ($this_property_is_field && empty($fields_instances[$property_name])) {
      continue;
    }

    $form['fields_wrapper']['fields_code'][$fieldset_name] = array(
      '#type' => 'fieldset',
      '#title' => $property_info['label'] . ' (' . $property_name . ')' . (!empty($property_info['required']) ? ' *' : ''),
      '#description' => isset($property_info['description']) ? $property_info['description'] : '',
      '#weight' => (!empty($property_info['required']) || !empty($property_info['field'])) ? $weight : $weight + 100,
      '#group' => 'vertical_tabs',
    );
    
    $description = 'PHP код, который должен вернуть ' . ($this_property_is_multi_value ? 'массив значений поля (<code>array(значение1, значение2, ...)</code>)' : 'значение поля');
    // Simle field or property
    if (!isset($property_info['property info'])) {
      $description .= ($field_type ? ' (тип: ' . check_plain($field_type) . ').' : '.');
    }
    // Compound field or property
    else {
      if ($this_property_is_multi_value) {
        $description .= '. Каждое значение должно быть';
      }
      $description .= ' в формате:<br /><br /><code>array(<br />' . _parser_get_field_structure($property_info) . ')</code>';
    }
    
    $form['fields_wrapper']['fields_code'][$fieldset_name][$property_name] = array(
      '#type' => 'textarea',
      '#title' => 'PHP код для поля ' . $property_info['label'],
      '#description' => $description,
      '#default_value' => isset($fields_code[$property_name]) ? $fields_code[$property_name] : '',
      '#rows' => 3,
      '#resizable' => FALSE,
      '#required' => !empty($property_info['required']),
    ) + $check_button;
    
    
    $form['fields_wrapper']['fields_code'][$fieldset_name]['techical_info_fieldset'] = array(
      '#type' => 'fieldset',
      '#title' => 'Техническая информация о поле',
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
    );
    
    if (function_exists('kprint_r')) {
      $techical_info = kprint_r($property_info, TRUE, 'entity_get_property_info()');
      if ($this_property_is_field) {
        $techical_info .= kprint_r($fields_info[$property_name], TRUE, 'field_info_fields()');
        $techical_info .= kprint_r($fields_instances[$property_name], TRUE, 'field_info_instances()');
      }
    }
    else {
      $techical_info = 'Установите модуль <a href="http://drupal.org/project/devel" target="_blank">Devel</a>.';
    }
      
    $form['fields_wrapper']['fields_code'][$fieldset_name]['techical_info_fieldset']['techical_info'] = array(
      '#markup' => $techical_info,
    );
    
    $weight++;
  }
  
  // Advanced settings
  $form += _parser_job_form_advanced_settings($job, $check_button);
  
  $form['actions'] = array(
    '#type' => 'actions',
  );
  
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => 'Сохранить',
  );
  
  $form['actions']['submit_and_start'] = array(
    '#type' => 'submit',
    '#value' => 'Сохранить и Начать',
  );
  
  if (!empty($job->jid) && _parser_url_count($job->jid, 0)) {
    $form['actions']['submit_and_resume'] = array(
      '#type' => 'submit',
      '#value' => 'Сохранить и Продолжить',
    );
  }
  
  _parser_set_rows($form);
  
  $parser_path = drupal_get_path('module', 'parser');
  $form['#attached']['library'][] = array('system', 'ui.dialog');
  $form['#attached']['library'][] = array('parser', 'codemirror');
  $form['#attached']['css'][] = $parser_path . '/parser.css';
  $form['#attached']['js'][] = $parser_path . '/parser.js';
  
  drupal_add_js(array('parserCheckPath' => url('parser/check')), 'setting');

  return $form;
}

/**
 * Advanced settings sub-form.
 */
function _parser_job_form_advanced_settings($job, $check_button) {
  $form['advanced_settings'] = array(
    '#type' => 'fieldset',
    '#title' => 'Дополнительные настройки',
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  
  $form['advanced_settings']['parse_limit'] = array(
    '#type' => 'textfield',
    '#title' => 'Ограничить число создаваемых/обновляемых сущностей',
    '#description' => '
      Максимальное число сущностей, которое сможет создать/обновить парсер. Применяется в осномном при тестировании
      задания. Оставьте пустым, если ограничения не нужны.
    ',
    '#default_value' => !empty($job->parse_limit) ? $job->parse_limit : '',
    '#size' => 5,
  );
  
  $form['advanced_settings']['sleep'] = array(
    '#type' => 'textfield',
    '#title' => 'Задержка между http запросами',
    '#description' => '
      Количество секунд между http запросами к сайту источнику. Помогает обойти защиту от парсинга. По умолчанию
      задержка отсутствует.
    ',
    '#default_value' => !empty($job->sleep) ? $job->sleep : 0,
    '#size' => 5,
    '#field_suffix' => 'сек.',
  );

  $form['advanced_settings']['run_period'] = array(
    '#type' => 'textfield',
    '#title' => 'Периодичность запуска в фоне',
    '#description' => '
      Период фонового запуска парсера. Оставьте пустым, если не хотите запускать парсинг автоматически. Значение должно
      быть не меньше периодичности запуска крона. Перед использованием опции, убедитесь, что cron запускается с помощью
      crontab и период на странице <code>admin/config/system/cron</code> равен <i>Never</i>.
    ',
    '#default_value' => !empty($job->run_period) ? $job->run_period : '',
    '#size' => 5,
    '#field_suffix' => 'сек.',
  );

  $form['advanced_settings']['site_charset'] = array(
    '#type' => 'select',
    '#title' => 'Кодировка сайта',
    '#description' => 'Выберите кодировку, которая используется на сайте источнике. Рекомендуется автоопределение.',
    '#options' => array('auto' => 'Автоопределение', 'UTF-8' => 'UTF-8', 'WINDOWS-1251' => 'WINDOWS-1251'),
    '#default_value' => isset($job->site_charset) ? $job->site_charset : 'auto',
  );
  
  $form['advanced_settings']['charset_fix'] = array(
    '#type' => 'checkbox',
    '#title' => 'Передавать кодировку в конструктор phpQuery',
    '#description' => 'Отметьте, если возникают проблемы с кодировкой. Костыль!',
    '#default_value' => isset($job->charset_fix) ? $job->charset_fix : FALSE,
  );
  
  $form['advanced_settings']['only_this_domen'] = array(
    '#type' => 'checkbox',
    '#title' => 'Парсить только с этого же домена',
    '#description' => 'Отметьте, если хотите чтобы парсер работал только на домене, указанном в поле "Стартовый URL".',
    '#default_value' => isset($job->only_this_domen) ? $job->only_this_domen : TRUE,
  );
  
  $form['advanced_settings']['save_url'] = array(
    '#type' => 'checkbox',
    '#title' => 'Сохранять адреса',
    '#description' => 'Отметьте, если хотите, чтобы у сущностей были такие же адреса, как на сайте источнике. Не включайте эту опцию если на сайте источнике не используется ЧПУ (т.е. в адресах есть символ "?").',
    '#default_value' => isset($job->save_url) && module_exists('path') ? $job->save_url : 0,
    '#disabled' => !module_exists('path'),
  );
  
  $form['advanced_settings']['no_update'] = array(
    '#type' => 'checkbox',
    '#title' => 'Не обновлять сущности',
    '#description' => 'Отметьте, если не хотите обновлять сущности, созданные в предыдущие запуски этого задания.',
    '#default_value' => isset($job->no_update) ? $job->no_update : 0,
  );
  
  $form['advanced_settings']['force_download'] = array(
    '#type' => 'checkbox',
    '#title' => 'Не брать страницы из кэша',
    '#description' => 'Отметьте, если хотите чтобы парсер скачивал страницы даже если они есть в кэше.',
    '#default_value' => isset($job->force_download) ? $job->force_download : 0,
  );
  
  $form['advanced_settings']['force_download_list'] = array(
    '#type' => 'textarea',
    '#title' => 'Отключить кэш для следующих адресов',
    '#description' => 'Маски адресов, для которых отключено использование кэша. Оставьте пустым, если хотите отключить кэш для всех страниц. Пример заполнения: <code>http://drupal.org/project/modules?page=*</code>',
    '#default_value' => isset($job->force_download_list) ? $job->force_download_list : '',
    '#states' => array(
      'visible' => array(
        'input[name="force_download"]' => array('checked' => TRUE),
      ),
    ),
  );
  
  $form['advanced_settings']['list_mode'] = array(
    '#type' => 'checkbox',
    '#title' => 'Режим списка',
    '#description' => 'Отметьте, если сущности на сайте источнике не имеют своих страниц, а располагаются в виде списков. Например это могут быть комментарии.',
    '#default_value' => !empty($job->list_mode_code),
  );
  
  $form['advanced_settings']['list_mode_code'] = array(
    '#type' => 'textarea',
    '#title' => 'Элементы списка',
    '#description' => '
      Код, который должен вернуть простой одномерный массив с элементами списка.
      Доступные переменные: $page - html код страницы, $doc - объект phpQuery.
      Пример заполнения:<br />
      <code>
        $elements = array();<br />
        foreach ($doc->find(\'#comments .comment\') as $element) {<br />
        &nbsp;&nbsp;$elements[] = pq($element)->html();<br />
        }<br />
        return $elements;
      </code>
    ',
    '#default_value' => isset($job->list_mode_code) ? $job->list_mode_code : '',
    '#rows' => 3,
    '#resizable' => FALSE,
    '#states' => array(
      'visible' => array(
        'input[name="list_mode"]' => array('checked' => TRUE),
      ),
    ),
  ) + $check_button;

  $form['advanced_settings']['init_code'] = array(
    '#type' => 'textarea',
    '#title' => 'Код предварительной инициализации',
    '#description' => '
      Код, который будет выполнен перед загрузкой стартового адреса. Например, здесь можно пройти авторизацию и сохранить
      номер сессии в переменную с префиксом <code>parser_data_</code> для дальнейшего использования в HTTP заголовках.
      Пример кода:<br />
      <code>
        $post_data = array(\'username\' => \'admin\', \'password\' => \'1234\');<br />
        $result = _parser_post_request(\'http://example.com/login.php\', $post_data, array(\'max_redirects\' => 0));<br />
        if (preg_match(\'#SessId=(.+?);#\', $result->headers[\'set-cookie\'], $matches)) {<br />
        &nbsp;&nbsp;variable_set(\'parser_data_examplecom_sessid\', $matches[1]);<br />
        }<br />
      </code>
    ',
    '#default_value' => isset($job->init_code) ? $job->init_code : '',
    '#rows' => 3,
    '#resizable' => FALSE,
  ) + $check_button;

  $form['advanced_settings']['prepare_code'] = array(
    '#type' => 'textarea',
    '#title' => 'Пост-обработка сущности',
    '#description' => '
      Код, который будет выполнен перед вызовом entity_save(). Доступные переменные:
        $entity - объект сущности,
        $entity_wrapper - entity metadata wrapper,
        $page - html код страницы,
        $doc - объект phpQuery.
      Пример заполнения: <code>$entity->title = \'new title\';</code>
    ',
    '#default_value' => isset($job->prepare_code) ? $job->prepare_code : '',
    '#rows' => 3,
    '#resizable' => FALSE,
  );
  
  $form['advanced_settings']['headers'] = array(
    '#type' => 'textarea',
    '#title' => 'HTTP заголовки',
    '#description' => '
      Заголовки, используемые кравлером. В тексте можно использовать токены в формате <code>[parser_data_VARNAME]</code>,
      которые будут заменены на значения, возвращаемые функцией <code>variable_get(\'parser_data_VARNAME\')</code>
    ',
    '#default_value' => isset($job->headers) ? $job->headers : 'User-Agent: Opera/9.80 (Windows NT 6.1; U; ru) Presto/2.8.131 Version/11.10' . "\n",
  );
  
  $form['advanced_settings']['proxies'] = array(
    '#type' => 'value',
    '#title' => 'Прокси',
    '#description' => '',
    '#value' => '',
  );
  
  $form['advanced_settings']['clear_watchdog'] = array(
    '#type' => 'checkbox',
    '#title' => 'Очистить системный журнал',
    '#description' => 'Отметьте, если хотите очистить системный журнал (watchdog) перед началом парсинга.',
    '#default_value' => module_exists('dblog'),
    '#disabled' => !module_exists('dblog'),
  );
  
  return $form;
}

/**
 * Ajax callback.
 */
function parser_job_form_entity_type_ajax_callback($form, $form_state) {
  return array(
    '#type' => 'ajax',
    '#commands' => array(
      ajax_command_insert('#edit-bundle-wrapper', drupal_render($form['bundle'])),
      ajax_command_insert('#fields-code-wrapper', drupal_render($form['fields_wrapper'])),
    ),
  );
}

/**
 * Ajax callback.
 */
function parser_job_form_fields_ajax_callback($form, $form_state) {
  return $form['fields_wrapper'];
}

/**
 * Job form submit callback.
 */
function parser_job_form_submit(&$form, &$form_state) {
  $clicked_button = $form_state['clicked_button']['#value'];
  $fields = field_info_fields();
  $fields_code = array();
  
  if (isset($form_state['values']['fields_code'])) {
    foreach ($form_state['values']['fields_code'] as $fieldset) {
      foreach ($fieldset as $field_name => $field_code) {
        $fields_code[$field_name] = $field_code;
      }
    }
    $fields_code = array_diff($fields_code, array(''));
  }
  $form_state['values']['fields_code'] = $fields_code;
  
  if (!$form_state['values']['parse_limit']) {
    $form_state['values']['parse_limit'] = 0;
  }
  if (!$form_state['values']['run_period']) {
    $form_state['values']['run_period'] = 0;
  }
  
  $job = entity_ui_form_submit_build_entity($form, $form_state);
  $job->save();

  // Save
  if ($form_state['clicked_button']['#value'] == $form_state['values']['submit']) {
    drupal_set_message('Задание сохранено.');
    $form_state['redirect'] = 'admin/structure/parser_jobs/manage/' . $job->jid;
  }
  // Save and Run
  elseif ($form_state['clicked_button']['#value'] == $form_state['values']['submit_and_start']) {
    parser_run_batch($job, request_path() == 'admin/structure/parser_jobs/add' || arg(5) == 'clone');
  }
  // Save and Resume
  elseif ($form_state['clicked_button']['#value'] == $form_state['values']['submit_and_resume']) {
    parser_run_batch($job, request_path() == 'admin/structure/parser_jobs/add', TRUE);
  }
}

/**
 * Init batch.
 */
function parser_run_batch($job, $redirect_to_job = FALSE, $resume = FALSE) {
  if (is_numeric($job)) {
    $job = parser_job_load($job);
  }

  if (!empty($job->clear_watchdog)) {
    db_delete('watchdog')->execute();
  }

  if (!$resume) {
    watchdog('parser', 'Начался парсинг "' . $job->title . '"', NULL);

    _parser_url_delete_all($job->jid);

    if ($job->init_code) {
      eval($job->init_code);
    }

    $start_urls = explode("\n", trim($job->start_url));
    foreach ($start_urls as $start_url) {
      $start_url = trim($start_url);
      if (preg_match('#\[mask:(\d+),(\d+)\]#', $start_url, $matches)) {
        $min = (int)$matches[1];
        $max = (int)$matches[2];
        for ($i = $min; $i <= $max; $i++) {
          $url = str_replace('[mask:' . $matches[1] . ',' . $matches[2] . ']', $i, $start_url);
          _parser_url_add($url, $job->jid);
        }
      }
      else {
        _parser_url_add($start_url, $job->jid);
      }
    }
  }

  _parser_prepare_cache_directory();

  batch_set(array(
    'title' => 'Парсинг',
    'progress_message' => '',
    'operations' => array(
      array('parser_parse', array($job, $redirect_to_job)),
    ),
    'finished' => 'parser_finished',
    'file' => drupal_get_path('module', 'parser') . '/parser.inc',
  ));
}

/**
 * Parse page.
 */
function parser_parse($job, $redirect_to_job, &$context) {
  drupal_set_time_limit(0);
  
  if (empty($context['sandbox'])) {
    $context['sandbox']['progress'] = _parser_url_count($job->jid, 1);
    $context['sandbox']['entity_ids'] = array();
    $context['sandbox']['created_entities'] = 0;
    $context['sandbox']['element'] = NULL;
    
    $context['results']['jid'] = $job->jid;
    $context['results']['time_start'] = time();
    $context['results']['created_entities'] = 0;
    $context['results']['updated_entities'] = 0;
    $context['results']['files_from_http'] = 0;
    $context['results']['files_from_http_error'] = 0;
    $context['results']['files_from_cache'] = 0;
    $context['results']['last_error_file'] = '';
    $context['results']['redirect_to_job'] = $redirect_to_job;
    $context['results']['messages_count'] = _parser_get_watchdog_rows_count();
  }
  
  $GLOBALS['parser']['stat'] = $context['results'];
  $GLOBALS['parser']['sleep'] = $job->sleep;
  
  $url = _parser_url_get_not_parsed($job->jid);
  $page_url = $GLOBALS['parser']['url'] = $url->url;
  $force_download = _parser_page_need_force_download($page_url, $job, $context['sandbox']['element']);
  $page = _parser_get_page_by_url($page_url, $job->headers, $job->site_charset, $force_download);
  $doc = _parser_create_phpquery($page, $job->charset_fix);
  $base = $doc->find('base')->attr('href');
  $base_url = $base ? $base : $page_url;
  
  // If list mode enabled
  $elements = NULL;
  if ($job->list_mode_code) {
    $elements = eval($job->list_mode_code);
    if ($elements) {
      $elements = array_values($elements); // Re-numbered elements
      $context['sandbox']['element'] = ($context['sandbox']['element'] === NULL) ? 0 : $context['sandbox']['element'] + 1;
      $element_html = $elements[$context['sandbox']['element']];
      $doc_element = _parser_create_phpquery($element_html, $job->charset_fix);
    }
  }
  
  // Find links
  if ($url->depth < $job->depth && !$context['sandbox']['element']) {
    foreach (_parser_get_page_links($doc) as $href) {
      $href_full = parser_get_absolute_url($base_url, $href);
      
      if ($job->white_list && !drupal_match_path($href_full, $job->white_list)) {
        continue;
      }
      if ($job->black_list && drupal_match_path($href_full, $job->black_list)) {
        continue;
      }
      if ($job->only_this_domen && !_parser_check_urls_host($page_url, $href_full)) {
        continue;
      }

      _parser_url_add($href_full, $job->jid, $url->depth + 1);
    }
  }

  // Decide - parse cur page or not
  $allowed_parse = TRUE;
  if (!$page) {
    $allowed_parse = FALSE;
  }
  if ($allowed_parse && $job->parse_limit && $context['sandbox']['created_entities'] >= $job->parse_limit) {
    $allowed_parse = FALSE;
  }
  if (is_array($elements) && count($elements) == 0) {
    $allowed_parse = FALSE;
  }
  if ($allowed_parse && $job->check_code) {
    $allowed_parse = eval($job->check_code);
  }
  if ($allowed_parse) {
    if ($job->remote_id_code) {
      $remote_id = eval($job->remote_id_code);
      if ($remote_id) {
        $entity_id = parser_get_entity_id_by_remote_id($remote_id, $job->jid);
      }
    }
    if (!isset($entity_id)) {
      $entity_id = parser_get_entity_id_by_url($page_url, $job->jid, $context['sandbox']['element']);
    }
    
    if ($entity_id && in_array($entity_id, $context['sandbox']['entity_ids'])) {
      $allowed_parse = FALSE;
    }
  }
  if ($allowed_parse && $job->no_update && $entity_id) {
    $allowed_parse = FALSE;
  }
  
  // Parse page
  if ($allowed_parse) {
    module_load_include('inc', 'parser', 'parser_prepare_field');
    module_load_include('inc', 'parser', 'parser_prepare_entity');
    
    // Load entity
    if ($entity_id) {
      $entity = entity_load_single($job->entity_type, $entity_id);
      $GLOBALS['parser']['stat']['updated_entities']++;
    }
    // Create entity object
    if (!isset($entity)) {
      $entity_info = entity_get_info($job->entity_type);
      $entity = entity_create($job->entity_type, array($entity_info['entity keys']['bundle'] => $job->bundle));
      $GLOBALS['parser']['stat']['created_entities']++;
    }
    
    $entity_wrapper = entity_metadata_wrapper($job->entity_type, $entity);
    $fields_info = field_info_fields();
    $fields_instances = field_info_instances($job->entity_type, $job->bundle);
    list($entity_properties, $entity_properties_fields) = _parser_get_entity_properties($job->entity_type, $job->bundle);
    
    // Populate properties and fields
    foreach ($job->fields_code as $property_name => $php_code) {
      if (!$php_code) {
        continue;
      }
      
      $result = eval($php_code);
      $property_is_field = isset($entity_properties_fields[$property_name]);
      $property_info = $property_is_field ? $entity_properties_fields[$property_name] : $entity_properties[$property_name];
      $property_is_compound = isset($property_info['property info']);
      $field_info = $property_is_field ? $fields_info[$property_name] : NULL;
      $field_is_multi_value = ($property_is_field && $field_info['cardinality'] != 1);
      
      // Prepare fields
      if ($property_is_field && $result !== NULL) {
        $function = '_parser_prepare_field_' . $field_info['type'];
        if (function_exists($function)) {
          if ($field_is_multi_value) {
            foreach (array_keys($result) as $key) {
              $function($result[$key], $field_info, $fields_instances[$property_name], $entity, $job, $base_url);
            }
          }
          else {
            $function($result, $field_info, $fields_instances[$property_name], $entity, $job, $base_url);
          }
        }
      }
      
      _parser_set_property($entity_wrapper, $property_name, $result, $property_is_compound, $field_is_multi_value, $property_info);
    }
    
    // Set Path alias
    if ($job->save_url) {
      _parser_set_entity_path($entity, $page_url);
    }
    
    // Prepare entity
    $function = '_parser_prepare_entity_' . $job->entity_type;
    if (function_exists($function)) {
      $function($entity, $entity_wrapper);
    }
    if ($job->prepare_code) {
      eval($job->prepare_code);
    }
    
    // Save entity
    try {
      $entity_wrapper->save();
    }
    catch (Exception $e) {
      _parser_watchdog('Ошибка при сохранении сущности: ' . $e->getMessage() . '. <b>Адрес:</b> ' . check_plain($page_url), $entity, WATCHDOG_ERROR);
    }
    
    if ($entity_id = $entity_wrapper->getIdentifier()) {
      _parser_map_save(array(
        'entity_id' => $entity_id,
        'jid' => $job->jid,
        'url' => $page_url,
        'remote_id' => isset($remote_id) ? $remote_id : '',
        'element' => $context['sandbox']['element'],
      ));
      
      $context['sandbox']['entity_ids'][] = $entity_id;
      $context['sandbox']['created_entities']++;
    }
  }
  
  if (!$elements || $context['sandbox']['element'] >= count($elements) - 1) {
    _parser_url_mark_parsed($page_url, $job->jid);
    $context['sandbox']['progress']++;
    $context['sandbox']['element'] = NULL;
  }

  $context['sandbox']['max'] = _parser_url_count($job->jid);
  $context['results'] = $GLOBALS['parser']['stat'];
  
  // Statistics
  $new_watchdog_row_count = _parser_get_watchdog_rows_count() - $context['results']['messages_count'];
  $last_error_file = $context['results']['files_from_http_error'] ? ' (последний: ' . check_plain($GLOBALS['parser']['stat']['last_error_file']) . ')' : '';
  $queue_pages = $context['sandbox']['max'] - $context['sandbox']['progress'];
  $elapsed_time = time() - $context['results']['time_start'];
  $left_time = $context['sandbox']['progress'] ? ($queue_pages / $context['sandbox']['progress']) * $elapsed_time : 0;
  $messages = array(
    'Последняя обработанная страница' => '(' . $url->depth . ') ' . $page_url,
    'Обработано страниц'              => $context['sandbox']['progress'],
    'Страниц в очереди'               => $queue_pages,
    'Создано сущностей'               => $context['results']['created_entities'],
    'Обновлено сущностей'             => $context['results']['updated_entities'],
    'Скачано url'                     => $context['results']['files_from_http'],
    'Не удалось скачать url'          => $context['results']['files_from_http_error'] . $last_error_file,
    'URL взято из кэша'               => $context['results']['files_from_cache'],
    'Затрачено времени'               => gmdate('H:i:s', $elapsed_time),
    'Осталось времени'                => gmdate('H:i:s', $left_time),
    'Новых сообщений в watchdog'      => $new_watchdog_row_count,
  );
  if ($new_watchdog_row_count) {
    $last_watchdog_message = _parser_get_last_watchdog_message();
    $messages['Последнее сообщение'] = '(' . $last_watchdog_message->time . ') ' . $last_watchdog_message->message;
  }
  $context['message'] = '<style>.message span { font-weight:normal; }</style>';
  foreach ($messages as $key => $value) {
    $context['message'] .= $key . ': <span>' . $value . '</span><br />';
  }

  if ($job->parse_limit && $context['sandbox']['created_entities'] >= $job->parse_limit) {
    $context['finished'] = 1;
  }
  elseif ($context['sandbox']['progress'] <= $context['sandbox']['max']) {
    $context['finished'] = $context['sandbox']['progress'] / $context['sandbox']['max'];
  }
}

/**
 * Site is parsed.
 */
function parser_finished($success, $results, $operations) {
  $message = '
    ' . ($success ? 'Парсинг завершён' : 'Парсинг завершён с ошибками') . '.<br />
    Обработано страниц: ' . _parser_url_count($results['jid'], 1) . '<br />
    Скачано URL: ' . $results['files_from_http'] . '<br />
    URL взято из кэша: ' . $results['files_from_cache'] . '<br />
    Не удалось скачать URL: ' . $results['files_from_http_error'] . '<br />
    Создано сущностей: ' . l($results['created_entities'], 'admin/structure/parser_jobs/results/' . $results['jid']) . '<br />
    Обновлено сущностей: ' . l($results['updated_entities'], 'admin/structure/parser_jobs/results/' . $results['jid']) . '<br />
    Затрачено времени: ' . gmdate('H:i:s', time() - $results['time_start']) . '<br />
    Новых сообщений в системном журнале: ' . l(_parser_get_watchdog_rows_count() - $results['messages_count'], 'admin/reports/dblog') . '<br />
  ';
  drupal_set_message($message, $success ? 'status' : 'error');
  watchdog('parser', $message, NULL, $success ? WATCHDOG_NOTICE : WATCHDOG_ERROR);

  if ($results['redirect_to_job']) {
    drupal_goto('admin/structure/parser_jobs/manage/' . $results['jid']);
  }
}

/**
 * Menu callback. Check php code.
 */
function parser_check_code() {
  $params = array('test_url', 'site_charset', 'charset_fix', 'field_name', 'field_value', 'list_mode_code', 'headers');
  foreach ($params as $param) {
    if (!isset($_POST[$param])) {
      echo 'bad params';
      return NULL;
    }
  }

  _parser_prepare_cache_directory();

  $GLOBALS['parser']['stat'] = array(
    'files_from_http' => 0,
    'files_from_http_error' => 0,
    'files_from_cache' => 0,
  );
  $page = _parser_get_page_by_url($_POST['test_url'], $_POST['headers'], $_POST['site_charset']);
  $page_url = $_POST['test_url'];
  $doc = _parser_create_phpquery($page, $_POST['charset_fix']);
  
  if ($_POST['list_mode_code']) {
    $elements = eval($_POST['list_mode_code']);
    $element_html = current($elements);
    $doc_element = _parser_create_phpquery($element_html, $_POST['charset_fix']);
  }
  
  $result = eval($_POST['field_value']);
  
  echo print_r($result, TRUE);
}

/**
 * Menu callback. Redirect to entity page.
 */
function parser_redirect($entity_type, $entity_id) {
  $entity = entity_load_single($entity_type, $entity_id);
  $uri = entity_uri($entity_type, $entity);
  drupal_goto($uri['path'], $uri['options']);
}

/**
 * Result page.
 */
function parser_result_page($job) {
  $entity_info = entity_get_info($job->entity_type);
  $label_key = isset($entity_info['entity keys']['label']) ? $entity_info['entity keys']['label'] : '';
  $header = array(
    array('data' => '№'),
    array('data' => 'Entity ID (' . $entity_info['entity keys']['id'] . ')', 'field' => $entity_info['entity keys']['id']),
    array('data' => 'Remote ID',                                             'field' => 'remote_id'),
    array('data' => 'Label' . ($label_key ? ' (' . $label_key . ')' : ''),   'field' => $label_key),
    array('data' => 'Original URL',                                          'field' => 'url'),
  );
  
  $query = db_select('parser_map', 'pm');
  $query->fields('pm', array('entity_id', 'url', 'remote_id'));
  if ($label_key) {
    $query->fields('e', array($label_key));
  }
  $query->innerJoin($entity_info['base table'], 'e', 'e.' . $entity_info['entity keys']['id'] . ' = pm.entity_id');
  $query->condition('jid', $job->jid);
  $query->extend('TableSort')->orderByHeader($header);
  
  $rows = array();
  foreach ($query->execute() as $i => $row) {
    $rows[] = array(
      $i + 1,
      l($row->entity_id, 'parser/redirect/' . $job->entity_type . '/' . $row->entity_id, array('alias' => TRUE)),
      $row->remote_id,
      $label_key ? $row->{$label_key} : '',
      _parser_get_external_link($row->url),
    );
  }
  
  drupal_set_title('Сущности, созданные заданием "' . $job->title . '"');
  
  return array(
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $rows,
    '#empty' => 'Пусто...',
  );
}

/**
 * Rollback form.
 */
function parser_rollback_form($form, $form_state, $job) {
  $form['jid'] = array(
    '#type' => 'value',
    '#value' => $job->jid,
  );
 
  return confirm_form(
    $form,
    'Вы действительно хотите удалить все созданные парсером сущности?',
    'admin/structure/parser_jobs',
    'Эта операция необратима.',
    'Удалить',
    'Отменить'
  );
}

/**
 * Rollback form submit callback.
 */
function parser_rollback_form_submit($form, &$form_state) {
  drupal_set_time_limit(0);
  
  $job = parser_job_load($form_state['values']['jid']);
  $deleted_count = 0;
  
  $query = db_select('parser_map', 'pm');
  $query->fields('pm', array('entity_id'));
  $query->condition('jid', $job->jid);
  $result = $query->execute();
  
  foreach ($result as $row) {
    entity_delete($job->entity_type, $row->entity_id);
  }
  
  drupal_set_message('Удалено ' . $result->rowCount() . ' сущностей.');
  
  $form_state['redirect'] = 'admin/structure/parser_jobs';
}

/**
 * Settings form.
 */
function parser_settings_form($form) {
  $form['parser_show_source_link'] = array(
    '#type' => 'checkbox',
    '#title' => 'Показывать ссылку на источник на страницах созданных сущностей.',
    '#default_value' => variable_get('parser_show_source_link', 1),
  );
  
  $form['actions'] = array(
    '#type' => 'actions',
  );
  
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => 'Сохранить',
  );
  
  return system_settings_form($form);
}

/**
 * Return entity id by URL.
 */
function parser_get_entity_id_by_url($url, $jid = NULL, $element = NULL) {
  $query = db_select('parser_map', 'pn')
    ->fields('pn', array('entity_id'))
    ->condition('url', $url)
    ->range(0, 1);
    
  if ($jid) {
    $query->condition('jid', $jid);
  }
  
  if ($element === NULL) {
    $query->isNull('element');
  }
  else {
    $query->condition('element', $element);
  }
  
  return $query->execute()->fetchField();
}

/**
 * Return entity id by Page Key.
 */
function parser_get_entity_id_by_remote_id($remote_id, $jid = NULL) {
  $query = db_select('parser_map', 'pn')
    ->fields('pn', array('entity_id'))
    ->condition('remote_id', $remote_id)
    ->range(0, 1);

  if ($jid) {
    $query->condition('jid', $jid);
  }
  
  return $query->execute()->fetchField();
}

/**
 * Find files in html, download and change tag attribute.
 */
function _parser_download_files($html, $selector, $attr, $base_url, $path = 'public://') {
  $doc = _parser_create_phpquery($html);
  
  foreach ($doc->find($selector) as $element) {
    $file_url = parser_get_absolute_url($base_url, pq($element)->attr($attr));
    $file_url_info = parse_url($file_url);
    $local_file_name = ltrim($file_url_info['path'], '/');
    $local_file_name = str_replace('/', '-', $local_file_name);
    
    if (!file_exists($path . '/' . $local_file_name)) {
      // Download image to cache
      $cache_file_uri = _parser_download_url_to_cache($file_url, array(), FALSE);
      if (!$cache_file_uri) {
        continue;
      }
      
      // Copy image from cache
      $file = _parser_copy_file($cache_file_uri, $path . '/' . $local_file_name);
      if ($file) {
        $file->status = FILE_STATUS_PERMANENT;
        file_save($file);
      }
    }
    
    $new_atr = file_create_url($path . '/' . $local_file_name);
    $new_atr = drupal_substr($new_atr, drupal_strlen($GLOBALS['base_root']));
    pq($element)->attr($attr, $new_atr);
  }
  
  return $doc->html();
}

/**
 * Find img tag in html, download files and change src attribute.
 */
function parser_download_images($html, $base_url, $path = 'public://images') {
  return _parser_download_files($html, 'img', 'src', $base_url, $path);
}

/**
 * Find links in html, download files and change href attribute.
 */
function parser_download_links($html, $extensions, $base_url, $path = 'public://') {
  $selectors = array();
  foreach ($extensions as $extension) {
    $selectors[] = 'a[href$=".' . $extension . '"]';
  }
  return _parser_download_files($html, implode(', ', $selectors), 'href', $base_url, $path);
}

/**
 * Run job in background.
 */
function parser_run_in_background($jid) {
  parser_run_batch($jid);
  $batch = &batch_get();
  $batch['progressive'] = FALSE;
  batch_process('');
}

/**
 * Return entity object by conditions.
 * 
 * @param $entity_type
 *   The entity type to load, e.g. node, user or taxonomy_term.
 * @param $properties
 *   An associative array of properties values.
 * @param $fields
 *   An associative array of fields values.
 * 
 * Example:
 * @code
 * $node = parser_get_entity_by_conditions(
 *   'node',
 *   array('type' => 'article', 'title' => 'Foo'),
 *   array('field_category' => array('tid' => 2))
 * );
 * $term = parser_get_entity_by_conditions(
 *   'taxonomy_term',
 *   array('name' => 'Foo', 'vid' => 3)
 * );
 * @endcode
 */
function parser_get_entity_by_conditions($entity_type, $properties = NULL, $fields = NULL) {
  $query = new EntityFieldQuery();
  $query->entityCondition('entity_type', $entity_type);
  if ($properties) {
    foreach ($properties as $name => $value) {
      $query->propertyCondition($name, $value);
    }
  }
  if ($fields) {
    foreach ($fields as $name => $values) {
      foreach ($values as $column => $value) {
        $query->fieldCondition($name, $column, $value);
      }
    }
  }
  $query->range(0, 1);
  $result = $query->execute();
  if ($result) {
    $id = key($result[$entity_type]);
    return entity_load_single($entity_type, $id);
  }
}

/**
 * Return taxonomy term object by term name.
 */
function parser_get_term_by_name($term_name, $vid = NULL) {
  $conditions = array('name' => $term_name);
  if ($vid) {
    $conditions['vid'] = $vid;
  }
  return parser_get_entity_by_conditions('taxonomy_term', $conditions);
}

/**
 * Return entity id from feeds items.
 */
function parser_get_entity_id_from_feeds_items($importer_name, $guid) {
  return db_select('feeds_item', 'fi')
    ->fields('fi', array('entity_id'))
    ->condition('fi.id', $importer_name)
    ->condition('fi.guid', $guid)
    ->range(0, 1)
    ->execute()
    ->fetchField();
}

/**
 * Return array of elements value.
 */
function parser_get_pqelements_values($doc, $selector, $function = 'html', $map = 'trim') {
  $values = array();
  foreach ($doc->find($selector) as $element) {
    $values[] = pq($element)->$function();
  }
  if ($map) {
    $values = array_map($map, $values);
  }
  return $values;
}

/**
 * Get URL info.
 */
function _parser_url_get($url, $jid) {
  return db_select('parser_urls', 'pu')
    ->fields('pu')
    ->condition('pu.url', $url)
    ->condition('pu.jid', $jid)
    ->execute()
    ->fetchObject();
}

/**
 * Get first not parsed URL info.
 */
function _parser_url_get_not_parsed($jid) {
  return db_select('parser_urls', 'pu')
    ->fields('pu')
    ->condition('pu.jid', $jid)
    ->condition('pu.parsed', 0)
    ->range(0, 1)
    ->execute()
    ->fetchObject();
}

/**
 * Adding URL to database.
 */
function _parser_url_add($url, $jid, $depth = 0) {
  if (!_parser_url_get($url, $jid) && strpos($url, 'xandeadx.ru') === FALSE) {
    db_insert('parser_urls')
      ->fields(array(
        'url' => $url,
        'jid' => $jid,
        'depth' => $depth,
      ))
      ->execute();
  }
}

/**
 * Mark URL as parsed.
 */
function _parser_url_mark_parsed($url, $jid) {
  db_update('parser_urls')
    ->fields(array('parsed' => 1))
    ->condition('url', $url)
    ->condition('jid', $jid)
    ->execute();
}

/**
 * Delete all URLs from database.
 */
function _parser_url_delete_all($jid) {
  db_delete('parser_urls')
    ->condition('jid', $jid)
    ->execute();
}

/**
 * Get count not parsed URLs.
 */
function _parser_url_count($jid, $parsed = NULL) {
  $query = db_select('parser_urls', 'pu')
    ->condition('pu.jid', $jid);
  if ($parsed !== NULL) {
    $query->condition('pu.parsed', $parsed);
  }
  return $query->countQuery()->execute()->fetchField();
}

/**
 * Return page html by URL.
 */
function _parser_get_page_by_url($url, $headers = array(), $charset = 'auto', $force_download = FALSE) {
  $page_cache_uri = _parser_download_url_to_cache($url, $headers, $force_download);
  if (!$page_cache_uri) {
    return NULL;
  }
  $page = file_get_contents($page_cache_uri);
  
  // Find charset
  if ($charset == 'auto') {
    if (preg_match('# content="text/html; charset=(.+)"#isU', $page, $matches)) {
      $charset = $matches[1];
    }
  }
  // Convert charset
  if (drupal_strtolower($charset) != 'utf-8' && $charset != 'auto') {
    $page = iconv($charset, 'UTF-8//IGNORE', $page);
    $page = str_ireplace(
      ' content="text/html; charset=' . $charset . '"',
      ' content="text/html; charset=utf-8"',
      $page
    );
  }
      
  return $page;
}

/**
 * Download remote url to cache and return cache uri.
 */
function _parser_download_url_to_cache($url, $headers = array(), $force_download = FALSE) {
  $cache_uri = 'public://parser_cache/' . md5($url);

  if (!file_exists($cache_uri) || $force_download) {
    if (!empty($GLOBALS['parser']['sleep'])) {
      sleep($GLOBALS['parser']['sleep']);
    }

    // Headers
    if (is_string($headers)) {
      $headers = _parser_get_headers_array($headers);
    }
    foreach ($headers as &$header) {
      $header = preg_replace_callback('#\[(parser_data_.+?)\]#', '_parser_data_replace_callback', $header);
    }
    $request_options = array('headers' => $headers);

    $result = drupal_http_request($url, $request_options);

    // Processing redirect
    if ($result->code == 303 && !empty($result->headers['location'])) {
      $url = $result->headers['location'];
      $result = drupal_http_request($url, $request_options);
    }

    // Check errors
    if (!empty($result->error)) {
      _parser_watchdog('Ошибка при закачке "' . check_plain($url) . '": ' . check_plain($result->error) . (isset($result->data) ? ', <b>Результат</b>: ' . check_plain(drupal_substr($result->data, 0, 1000)) : ''), NULL, WATCHDOG_ERROR);
      $GLOBALS['parser']['stat']['files_from_http_error']++;
      $GLOBALS['parser']['stat']['last_error_file'] = $url;
      return NULL;
    }
    if (!$result->data) {
      _parser_watchdog('Ошибка при закачке "' . check_plain($url) . '": сервер не возвратил данных.', NULL, WATCHDOG_ERROR);
      $GLOBALS['parser']['stat']['files_from_http_error']++;
      $GLOBALS['parser']['stat']['last_error_file'] = $url;
      return NULL;
    }

    file_unmanaged_save_data($result->data, $cache_uri, FILE_EXISTS_REPLACE);
    $GLOBALS['parser']['stat']['files_from_http']++;
  }
  else {
    $GLOBALS['parser']['stat']['files_from_cache']++;
  }

  return $cache_uri;
}

/**
 * Pre replace callback.
 */
function _parser_data_replace_callback($matches) {
  return variable_get($matches[1]);
}

/**
 * Add row in {parser_map} table.
 */
function _parser_map_save($record) {
  db_merge('parser_map')
    ->key(array('entity_id' => $record['entity_id']))
    ->fields(array(
      'jid' => $record['jid'],
      'url' => $record['url'],
      'element' => $record['element'],
      'remote_id' => $record['remote_id'],
    ))
    ->execute();
}

/**
 * Return URL by entity id.
 */
function _parser_get_url_by_entity_id($entity_type, $entity_id, $jid = NULL) {
  $query = db_select('parser_map', 'pn');
  $query->fields('pn', array('url'));
  $query->innerJoin('parser_jobs', 'pj', 'pj.jid = pn.jid');
  $query->condition('pj.entity_type', $entity_type);
  $query->condition('pn.entity_id', $entity_id);
    
  if ($jid) {
    $query->condition('jid', $jid);
  }
  
  return $query->execute()->fetchField();
}

/**
 * Return row count.
 */
function _parser_get_rows($text) {
  $count = substr_count(trim($text), "\n") + 1;
  if ($count < 3) {
    $count = 3;
  }
  elseif ($count > 20) {
    $count = 20;
  }
  return $count;
}

/**
 * Set row count.
 */
function _parser_set_rows(&$form) {
  foreach (element_children($form) as $element) {
    if (isset($form[$element]['#type']) && $form[$element]['#type'] == 'textarea') {
      $form[$element]['#rows'] = _parser_get_rows($form[$element]['#default_value']);
    }
    else {
      _parser_set_rows($form[$element]);
    }
  }
}

/**
 * Return absolute url by base and relative url (deprecated).
 */
function _parser_get_full_url($relative_url, $base_url) {
  $relative_url = trim($relative_url);
  $relative_url = preg_replace('/#.*/', '', $relative_url);
  $relative_url_parsed = parse_url($base_url);
  if (!isset($relative_url_parsed['path'])) {
    $relative_url_parsed['path'] = '/';
  }

  // example: //wikimedia.org/dir/image.png
  if (drupal_substr($relative_url, 0, 2) == '//') {
    $relative_url = $relative_url_parsed['scheme'] . ':' . $relative_url;
  }
  // example: /dir/image.png
  elseif (drupal_substr($relative_url, 0, 1) == '/') {
    $relative_url = $relative_url_parsed['scheme'] . '://' . $relative_url_parsed['host'] . $relative_url;
  }
  // example: dir/image.png
  elseif (!preg_match('#^[a-z]+://#', $relative_url)) {
    $path = preg_replace('#/[^/]+$#', '', $relative_url_parsed['path']);
    $path = trim($path, '/');
    if ($path) {
      $path .= '/';
    }
    $relative_url = $relative_url_parsed['scheme'] . '://' . $relative_url_parsed['host'] . '/' . $path . $relative_url;
  }
  
  $relative_url = str_replace(' ', '+', $relative_url);
  
  return $relative_url;
}

/**
 * Return absolute url by base and relative url.
 */
function parser_get_absolute_url($base_url, $relative_url) {
  require_once DRUPAL_ROOT . '/' . drupal_get_path('module', 'parser') . '/phpuri/phpuri.php';

  if (preg_match('#^[a-z]+://[^/]+$#', $base_url)) {
    $base_url .= '/';
  }

  return phpUri::parse($base_url)->join($relative_url);
}

/**
 * Create phpQuery object.
 */
function _parser_create_phpquery($html, $charset_fix = FALSE) {
  require_once DRUPAL_ROOT . '/' . drupal_get_path('module', 'parser') . '/phpQuery/phpQuery/phpQuery.php';
  return phpQuery::newDocumentHTML($html, $charset_fix ? 'utf8' : NULL);
}

/**
 * Return example field structure.
 */
function _parser_get_field_structure($field_info, $spaces = 2) {
  $strucure = '';
  foreach ($field_info['property info'] as $property_name => $property_info) {
    if (!isset($property_info['setter callback'])) {
      continue;
    }
    $strucure .= str_repeat('&nbsp;', $spaces) . "'$property_name' => \${$property_name}, // " . $property_info['label'] . ' (' . $property_info['type'] . ', ' . (isset($property_info['required']) ? 'required' : 'optional') . ')<br />';
  }
  return $strucure;
}

/**
 * Set entity property.
 */
function _parser_set_property($entity_wrapper, $property_name, $property_value, $property_is_compound, $property_is_multi_value, $property_info) {
  // Multi-value compound property
  if ($property_is_multi_value && $property_is_compound && $property_value) {
    foreach ($property_value as $delta => $property_item) {
      foreach ($property_item as $sub_property_name => $sub_property_value) {
        try {
          $entity_wrapper->{$property_name}[$delta]->{$sub_property_name} = $sub_property_value;
        }
        catch (Exception $e) {
          _parser_watchdog('Ошибка при записи в $entity_wrapper->' . $property_name . '[\'' . $delta . '\']->' . $sub_property_name . ': ' . $e->getMessage(), $sub_property_value, WATCHDOG_ERROR);
        }
      }
    }
  }
  // Single-value compound property
  else if (!$property_is_multi_value && $property_is_compound && $property_value) {
    // Create value
    if ($entity_wrapper->{$property_name}->value() === NULL && empty($property_info['auto creation'])) {
      try {
        $entity_wrapper->{$property_name} = $property_value;
      }
      catch (Exception $e) {
        _parser_watchdog('Ошибка при записи в $entity_wrapper->' . $property_name . ': ' . $e->getMessage(), $property_value, WATCHDOG_ERROR);
      }
    }
    // Set value
    else {
      foreach ($property_value as $sub_property_name => $sub_property_value) {
        try {
          $entity_wrapper->{$property_name}->{$sub_property_name} = $sub_property_value;
        }
        catch (Exception $e) {
          _parser_watchdog('Ошибка при записи в $entity_wrapper->' . $property_name . '->' . $sub_property_name . ': ' . $e->getMessage(), $sub_property_value, WATCHDOG_ERROR);
        }
      }
    }
  }
  // Multi-value simple property
  else if ($property_is_multi_value && !$property_is_compound && $property_value) {
    foreach ($property_value as $delta => $property_item) {
      try {
        $entity_wrapper->{$property_name}[$delta] = $property_item;
      }
      catch (Exception $e) {
        _parser_watchdog('Ошибка при записи в $entity_wrapper->' . $property_name . '[\'' . $delta . '\']: ' . $e->getMessage(), $property_item, WATCHDOG_ERROR);
      }
    }
  }
  // Single-value simple property
  else if (!$property_is_multi_value && !$property_is_compound) {
    try {
      $entity_wrapper->{$property_name} = $property_value;
    }
    catch (Exception $e) {
      _parser_watchdog('Ошибка при записи в $entity_wrapper->' . $property_name . ': ' . $e->getMessage(), $property_value, WATCHDOG_ERROR);
    }
  }
}

/**
 * Add error to watchdog.
 */
function _parser_watchdog($text, $object = NULL, $severity = WATCHDOG_NOTICE) {
  if ($object !== NULL) {
    $text .= '<br />Value:<pre>' . check_plain(print_r($object, TRUE)) . '</pre>';
  }
  if (!empty($GLOBALS['parser']['url'])) {
    $text .= '<br />Remote URL: ' . $GLOBALS['parser']['url'];
  }

  watchdog('parser', $text, NULL, $severity);
}

/**
 * Return entity type properties.
 */
function _parser_get_entity_properties($entity_type, $bundle) {
  $entity_properties_and_fields = entity_get_property_info($entity_type);
  $entity_properties = isset($entity_properties_and_fields['properties'])
    ? $entity_properties_and_fields['properties']
    : array();
  $entity_properties_fields = isset($entity_properties_and_fields['bundles'][$bundle]['properties'])
    ? $entity_properties_and_fields['bundles'][$bundle]['properties']
    : array();
  return array($entity_properties, $entity_properties_fields);
}

/**
 * Copy file from cache and return file object.
 */
function _parser_copy_file($from_uri, $to_uri) {
  $file = (object)array(
    'uid' => $GLOBALS['user']->uid,
    'uri' => $from_uri,
    'filename' => drupal_basename($to_uri),
    'filemime' => file_get_mimetype($from_uri),
  );
  $file_path = file_stream_wrapper_uri_normalize($to_uri);
  $file = file_copy($file, $file_path);
  if (!$file) {
    _parser_watchdog('Ошибка при копировании файла <i>' . $file->uri  . '</i> в <i>' . $file_path . '</i>.', NULL, WATCHDOG_ERROR);
  }
  
  return $file;
}

/**
 * Return page links.
 */
function _parser_get_page_links($doc) {
  $links = array();  
  foreach ($doc->find('a') as $a) {
    $href = pq($a)->attr('href');
    
    if (!$href) {
      continue;
    }
    // Continue if $href == '#content'
    if (drupal_substr($href, 0, 1) == '#') {
      continue;
    }
    // Continue if $href == 'ftp://ftp.drupal.org'
    if (preg_match('#^([a-z]+)://#', $href, $matches) && !in_array($matches[1], array('http', 'https'))) {
      continue;
    }
    // Continue if $href == 'javascript:alert();'
    if (strpos($href, 'javascript:') === 0 || strpos($href, 'mailto:') === 0) {
      continue;
    }
    
    // Remove hash
    $href = preg_replace('/#.*$/', '', $href);
    
    $links[] = $href;
  }
  
  return $links;
}

/**
 * Return TRUE if URLs have same host.
 */
function _parser_check_urls_host($url1, $url2) {
  $url1_host = preg_replace('#^www.#', '', parse_url($url1, PHP_URL_HOST));
  $url2_host = preg_replace('#^www.#', '', parse_url($url2, PHP_URL_HOST));
  if ($url1_host == $url2_host) {
    return TRUE;
  }
}

/**
 * Return options for form element.
 */
function _parser_get_entity_type_options($entity_types) {
  $options = array();
  foreach ($entity_types as $entity_type => $entity_info) {
    $options[$entity_type] = $entity_info['label'];
  }
  return $options;
}

/**
 * Return options for form element.
 */
function _parser_get_bundle_options($bundles) {
  $options = array();
  foreach ($bundles as $bundle_name => $bundle_info) {
    $options[$bundle_name] = $bundle_info['label'];
  }
  return $options;
}

/**
 * Return rows count in {watchdog} table.
 */
function _parser_get_watchdog_rows_count() {
  if (!module_exists('dblog')) {
    return 0;
  }
  return db_select('watchdog')->countQuery()->execute()->fetchField();
}

/**
 * Return options for drupal_http_request().
 */
function _parser_get_headers_array($headers) {
  $headers_array = array();
  foreach (explode("\n", $headers) as $header) {
    $header = trim($header);
    if ($header) {
      list($name, $value) = explode(': ', $header);
      $headers_array[$name] = $value;
    }
  }
  return $headers_array;
}

/**
 * Return last watchdog message.
 */
function _parser_get_last_watchdog_message() {
  if (!module_exists('dblog')) {
    return '';
  }
  $message = db_select('watchdog', 'w')
    ->fields('w', array('wid', 'message', 'variables', 'timestamp'))
    ->orderBy('w.wid', 'DESC')
    ->range(0, 1)
    ->execute()
    ->fetch();
  $message->message = theme('dblog_message', array('event' => $message));
  $message->time = format_date($message->timestamp, 'custom', 'H:i:s');
  return $message;
}

/**
 * Return link to external site.
 */
function _parser_get_external_link($url) {
  return l($url, 'http://anonym.to/?' . $url, array(
    'external' => TRUE,
    'attributes' => array('target' => '_blank'),
  ));
}

/**
 * Set entity path.
 */
function _parser_set_entity_path(&$entity, $page_url) {
  $page_url_parsed = parse_url($page_url);
  $alias = trim($page_url_parsed['path'], '/');
  $alias = str_replace('%20', ' ', $alias);
  if (!path_load(array('alias' => $alias))) {
    $entity->path = array(
      'alias' => $alias,
      'language' => LANGUAGE_NONE,
      'pathauto' => 0,
    );
  }
}

/**
 * Return TRUE if page need force download.
 */
function _parser_page_need_force_download($page_url, $job, $element) {
  $force_download = FALSE;
  if ($job->force_download && !$element) {
    if ($job->force_download_list) {
      $force_download = drupal_match_path($page_url, $job->force_download_list);
    }
    else {
      $force_download = TRUE;
    }
  }
  return $force_download;
}

/**
 * Return file name.
 */
function _parser_get_file_name_from_url($path) {
  $filename = drupal_basename($path);
  return current(explode('?', $filename));
}

/**
 * Return file extension.
 */
function _parser_get_file_extension_from_url($path) {
  $ext = pathinfo($path, PATHINFO_EXTENSION);
  $ext = current(explode('?', $ext));
  return drupal_strtolower($ext);
}

/**
 * Return entity count in {parser_map} table by jid.
 */
function _parser_get_entity_count($jid) {
  return db_select('parser_map')
    ->condition('jid', $jid)
    ->countQuery()
    ->execute()
    ->fetchField();
}

/**
 * Set setter callback for 'format' property.
 */
function _parser_set_format_setter_callback(&$property_info) {
  if (
    isset($property_info['type']) &&
    ($property_info['type'] == 'text_formatted' || $property_info['type'] == 'list<text_formatted>') &&
    isset($property_info['property info']['format']) && 
    !isset($property_info['property info']['format']['setter callback'])
  ) {
    $property_info['property info']['format']['setter callback'] = 'entity_property_verbatim_set';
  }
}

/**
 * Send POST http request.
 */
function _parser_post_request($url, array $data, array $options = array()) {
  return drupal_http_request($url, $options + array(
    'headers' => array('Content-Type' => 'application/x-www-form-urlencoded'),
    'method' => 'POST',
    'data' => drupal_http_build_query($data),
  ));
}

/**
 * Create cache directory if not exist.
 */
function _parser_prepare_cache_directory() {
  $cache_dir = 'public://parser_cache';
  file_prepare_directory($cache_dir, FILE_CREATE_DIRECTORY);
}
