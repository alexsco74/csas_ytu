<?php

/**
 * csas_ytu pages
 */
function csas_ytu_pages_rss($rss_uri) {
  $page_num = str_replace([CSAS_YTU_RSS_URI, '-'], '', $rss_uri);
  $vars = [
    'page_num' => $page_num,
    'start_index' => 0,
    'count_rec' => variable_get('csas_ytu_count_item_channel', 1000),
    'return_mode' => 'print',
    'channel' => 'RSS',
  ];
  return theme('csas_ytu_theme_rss', $vars);
}