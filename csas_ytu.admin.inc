<?php

/**
 * Yandex Turbo settings form
 *
 * @return array
 */
function csas_ytu_admin_config($form, &$form_state) {
  $form = [];
  $args = [
    '!url' => 'https://yandex.ru/support/webmaster/turbo/connection.html',
    '@title' => 'Yandex.Turbo RSS',
  ];
  $rss_href = url(CSAS_YTU_RSS_URI, ['absolute' => TRUE]);
  module_load_include('lib.inc', 'csas_ytu');
  list($count_rec, $count_page) = csas_ytu_lib_get_channel_params();
  $form['count'] = [
    '#type' => 'item',
    '#title' => t('Count items:@count, count page:@count_page', [
      '@count' => $count_rec,
      '@count_page' => $count_page,
    ]),
  ];
  $url_items = csas_ytu_lib_get_rss_urls();
  if ($url_items) {
    $form['rss_urls'] = [
      '#title' => t('Rss url'),
      '#theme' => 'item_list',
      '#items' => $url_items,
    ];
  }
  $form['about'] = [
    '#type' => 'item',
    '#title' => t('Yandex Turbo settings'),
    '#markup' => t('See more about <a href="!url">@title</a>.', $args) . ' ' . t('RSS feed:') . ' ' . l("{$rss_href}-1", "{$rss_href}-1"),
  ];
  //exclude front page variable_get('site_frontpage')
  $form['csas_ytu_exclude_front_page'] = [
    '#title' => t('Exclude front page'),
    '#type' => 'checkbox',
    '#default_value' => variable_get('csas_ytu_exclude_front_page', TRUE),
  ];
  $form['csas_ytu_node_types'] = [
    '#title' => t('Available node types'),
    '#type' => 'checkboxes',
    '#description' => t('Select node types for Yandex Turbo RSS export'),
    '#options' => node_type_get_names(),
    '#default_value' => variable_get('csas_ytu_node_types', []),
    '#ajax' => [
      'callback' => 'ajax_example_autotextfields_callback',
      'wrapper' => 'textfields',
      'effect' => 'fade',
    ],
  ];

  $form['image_fields'] = [
    '#prefix' => '<div id="textfields">',
    '#suffix' => '</div>',
  ];

  if (isset($form_state['values']['csas_ytu_node_types'])) {
    $node_types = array_filter($form_state['values']['csas_ytu_node_types']);
  }
  else {
    $node_types = variable_get('csas_ytu_node_types', []);
  }
  foreach ($node_types as $csas_ytu_node_type) {
    $image_fields = field_read_fields([
      'type' => 'image',
      'entity_type' => 'node',
      'bundle' => $csas_ytu_node_type,
    ]);
    $options = array_keys($image_fields);
    $options = array_combine($options, $options);
    $key = 'csas_ytu_image_field_bundle_' . $csas_ytu_node_type;
    if (empty($options)) {
      continue;
    }
    $form['image_fields'][$key] = [
      '#type' => 'select',
      '#title' => t('Image field for bundle "!bundle"', ['!bundle' => $csas_ytu_node_type]),
      '#options' => [t('- Select -')] + $options,
      '#default_value' => variable_get($key, 0),
    ];
  }
  if ($image_styles = image_styles()) {
    $options = array_keys($image_styles);
    $options = array_combine($options, $options);
    $form['csas_ytu_image_style'] = [
      '#title' => t('Image style'),
      '#type' => 'select',
      '#description' => t('Select image style for image field'),
      '#options' => [t('- Select -')] + $options,
      '#default_value' => variable_get('csas_ytu_image_style', 0),
    ];
  }
  $form['csas_ytu_promoted'] = [
    '#title' => t('Promoted'),
    '#description' => 'Order by promoted to front page option',
    '#type' => 'select',
    '#options' => drupal_map_assoc([t('Yes'), t('No')]),
    '#default_value' => variable_get('csas_ytu_promoted', 'No'),
  ];
  $form['csas_ytu_sticky'] = [
    '#title' => t('Sticky'),
    '#description' => 'Order by sticky at top of lists',
    '#type' => 'select',
    '#options' => drupal_map_assoc([t('Yes'), t('No')]),
    '#default_value' => variable_get('csas_ytu_sticky', 'No'),
  ];
  $form['csas_ytu_order_by'] = [
    '#title' => t('Order by'),
    '#type' => 'select',
    '#options' => [
      'created' => t('Date created'),
      'changed' => t('Date changed'),
      'title' => t('Title'),
    ],
    '#default_value' => variable_get('csas_ytu_order_by', 'created'),
  ];
  $form['csas_ytu_order'] = [
    '#title' => t('Order'),
    '#type' => 'select',
    '#options' => drupal_map_assoc([t('ASC'), t('DESC')]),
    '#default_value' => variable_get('csas_ytu_order', 'DESC'),
  ];
  $form['csas_ytu_body'] = [
    '#title' => t('Display'),
    '#description' => t('Choose turbo:content data source field'),
    '#type' => 'select',
    '#options' => [
      'value' => t('Body'),
      'summary' => t('Summary'),
    ],
    '#default_value' => variable_get('csas_ytu_body', 'value'),
  ];
  $form['csas_ytu_rss_title'] = [
    '#title' => t('RSS Title'),
    '#type' => 'textfield',
    '#required' => TRUE,
    '#default_value' => variable_get('csas_ytu_rss_title', variable_get('site_name', t('Drupal'))),
  ];
  $form['csas_ytu_rss_description'] = [
    '#title' => t('RSS description'),
    '#type' => 'textarea',
    '#required' => TRUE,
    '#default_value' => variable_get('csas_ytu_rss_description', variable_get('site_slogan', t('Drupal'))),
  ];
  $enable_tag = [
    'h1',
    'h2',
    'h3',
    'h4',
    'h5',
    'h6',
    'p',
    'a',
    'br',
    'hr',
    'ul',
    'ol',
    'li',
    'b',
    'strong',
    'i',
    'em',
    'sup',
    'sub',
    'ins',
    'del',
    'small',
    'big',
    'pre',
    'abr',
    'u',
    'table',
    'div',
    'code',
  ];
  $form['csas_ytu_rss_enable_tag'] = [
    '#title' => t('RSS description'),
    '#type' => 'textarea',
    '#required' => TRUE,
    '#default_value' => variable_get('csas_ytu_rss_enable_tag', implode("\n", $enable_tag)),
  ];
  $form['csas_ytu_rss_analytics_type'] = [
    '#title' => t('Analytics type'),
    '#type' => 'select',
    '#options' => [
      'Yandex' => t('Yandex'),
      'LiveInternet' => t('LiveInternet'),
      'Google' => t('Google'),
      'MailRu' => t('MailRu'),
      'Rambler' => t('Rambler'),
      'Mediascope' => t('Mediascope'),
    ],
    '#default_value' => variable_get('csas_ytu_rss_analytics_type', 'Yandex'),
  ];
  $form['csas_ytu_rss_analytics_id'] = [
    '#title' => t('Analytics id'),
    '#type' => 'textfield',
    '#default_value' => variable_get('csas_ytu_rss_analytics_id', ''),
  ];

  $form['#submit'][] = 'csas_ytu_admin_config_submit';
  return system_settings_form($form);
}

function csas_ytu_admin_config_submit($form, &$form_state) {
  drupal_flush_all_caches();
}

function ajax_example_autotextfields_callback($form, $form_state) {
  return $form['image_fields'];
}
