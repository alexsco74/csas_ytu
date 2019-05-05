<?php

/**
 * csas_ytu theme function
 */

/**
 * hook template_preprocess
 *
 * @param $variables
 */
function template_preprocess_csas_ytu_theme_rss(&$variables) {
  module_load_include('lib.inc', 'csas_ytu');
  if ($query = csas_ytu_lib_get_query($variables['page_num'], $variables['start_index'], $variables['count_rec'])) {
    $variables['feeds'] = $query->execute()
      ->fetchAllAssoc('nid', PDO::FETCH_ASSOC);
  }
}

function theme_csas_ytu_theme_rss($variables) {
  $cdata_pattern = '/^\s*\/\/<!\[CDATA\[([\s\S]*)\/\/\]\]>\s*\z/';
  module_load_include('lib.inc', 'csas_ytu');
  $page_uri = csas_ytu_lib_get_rss_uri($variables['page_num'], $variables['channel']);
  $rss_link = url($page_uri, ['absolute' => TRUE]);
  $feeds = $variables['feeds'];
  $items = '';
  $site_name = variable_get('site_name', '');
  $site_slogan = variable_get('site_slogan', 'Site description');
  $channel_title = variable_get('csas_ytu_rss_title', $site_name);
  $channel_description = variable_get('csas_ytu_rss_description', $site_slogan);
  $channel_analytics = variable_get('csas_ytu_rss_analytics_type', 'Yandex');
  $channel_analytics_id = variable_get('csas_ytu_rss_analytics_id', '');
  $style_name = variable_get('csas_ytu_image_style', 0);
  if (!empty($feeds)) {
    $used = 0;
    foreach ($feeds as $nid => &$node) {
      $item_turbo_content = '';
      //img
      //@todo
      if (!empty($node['img_uri']) && !empty($style_name)) {
        $node['img_uri'] = image_style_url($style_name, $node['img_uri']);
      }
      if (!empty($node['img_uri'])) {
        $item_turbo_content .= '<img src="' . $node['img_uri'] . '"/>';
      }
      //body
      $allow_tags = variable_get('csas_ytu_rss_enable_tag', '');
      $allow_tags = explode("\n", $allow_tags);
      $node['body'] = preg_replace($cdata_pattern, '$1', $node['body']);
      $node['body'] = filter_xss($node['body'], $allow_tags);
      if (drupal_strlen($node['body']) > 350) {
        $node['body'] = preg_replace('~(?:\r?\n){2,}~', "\n", $node['body']);
        $node['body'] = explode("\n", $node['body']);
        foreach ($node['body'] as $line_key => $line_val) {
          if (!preg_match("/(\<p|\<br|\<ol|\<ul|\<li)/is", $line_val)) {
            $node['body'][$line_key] .= '<br>';
          }
        }
        $node['body'] = implode("\n", $node['body']);
      }
      if (!empty($node['body'])) {
        $item_turbo_content .= $node['body'];
      }
      // Skip empty nodes
      if (!$item_turbo_content) {
        continue;
      }
      $node_uri = drupal_get_path_alias('node/' . $nid);
      $title_clear = filter_xss($node['title'], $allow_tags);
      $title_clear = check_plain($title_clear);
      $clear_search = ['&amp;', '&quot;', '&#039;', '&apos;', '&lt;', '&gt;'];
      $title_clear = str_replace($clear_search, '', $title_clear);
      $data = [
        [
          'key' => 'title',
          'value' => $title_clear,
        ],
        [
          'key' => 'link',
          'value' => url($node_uri, ['absolute' => TRUE]),
        ],

        [
          'key' => 'pubDate',
          'value' => date(DateTime::RFC822, $node['created']),
        ],
        [
          'key' => 'turbo:content',
          'value' => '<![CDATA[' . PHP_EOL . $item_turbo_content . PHP_EOL . ']]>',
          'encoded' => TRUE,
        ],
      ];
      $used++;
      $items .= format_xml_elements([
        [
          'key' => 'item',
          'value' => PHP_EOL . format_xml_elements($data),
          'encoded' => TRUE,
          'attributes' => [
            'turbo' => 'true',
          ],
        ],
      ]);
    }
  }
  if ($variables['return_mode'] == 'print') {
    drupal_add_http_header('Content-Type', 'application/rss+xml; charset=utf-8');
  }
  if (!empty($channel_analytics_id)) {
    $is_liveinternet = ('LiveInternet' == $channel_analytics);
    $param = $is_liveinternet ? 'param' : 'id';
    $analytics_attributes = [
      'type' => $channel_analytics,
      $param => $channel_analytics_id,
    ];
    $analytics = format_xml_elements([
      [
        'key' => 'yandex:analytics',
        'value' => '',
        'attributes' => $analytics_attributes,
      ],
    ]);
    $items = $analytics . $items;
  }
  $rss = [
    'key' => 'rss',
    'value' => PHP_EOL . format_rss_channel($channel_title, $rss_link, $channel_description, $items),
    'encoded' => TRUE,
    'attributes' => [
      'xmlns:yandex' => 'http://news.yandex.ru',
      'xmlns:media' => 'http://search.yahoo.com/mrss/',
      'xmlns:turbo' => 'http://search.yahoo.com/mrss/',
      'version' => '2.0',
    ],
  ];
  $rss_str = "<?xml version=\"1.0\" encoding=\"utf-8\"?>" . PHP_EOL . format_xml_elements([$rss]);
  if ($variables['return_mode'] == 'print') {
    print $rss_str;
  }
  elseif ($variables['return_mode'] == 'ret') {
    return $rss_str;
  }
}