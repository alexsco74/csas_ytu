<?php

/**
 * csas_ytu library
 */

function csas_ytu_lib_get_rss_urls() {
  $items = [];
  $channel_params = csas_ytu_lib_get_channel_params();
  if ($channel_params[1]) {
    foreach (range(1, $channel_params[1]) as $page_num) {
      $rss_uri = csas_ytu_lib_get_rss_uri($page_num);
      $rss_href = url($rss_uri, ['absolute' => TRUE]);
      $items[$rss_uri] = l($rss_href, $rss_href);
    }
  }
  return $items;
}

function csas_ytu_lib_get_channel_params($channel_item = 500) {
  module_load_include('lib.inc', 'csas_ytu');
  if ($query = csas_ytu_lib_get_query(0)) {
    $count_rec = $query->execute()
      ->fetchAllAssoc('nid', PDO::FETCH_ASSOC);
    $count_rec = count($count_rec);
    $count_page = intval($count_rec / $channel_item) + 1;
  }
  else {
    $count_rec = 0;
    $count_page = 0;
  }
  return [$count_rec, $count_page];
}

function csas_ytu_lib_get_rss_uri($page_num = 0, $channel = 'rss') {
  return CSAS_YTU_RSS_URI . '-' . ($channel == 'rss' ? '' : $channel . '-') . $page_num;
}

function csas_ytu_lib_get_query($page_num = 0, $start_index = 0, $count_rec = 500) {
  $query = '';
  $settings = variable_get('csas_ytu_node_types');
  $node_types = !empty($settings) ? array_keys(array_filter($settings)) : [];
  if (empty($node_types)) {
    return $query;
  }
  $orderBy = variable_get('csas_ytu_order_by', 'created');
  $order = variable_get('csas_ytu_order', 'DESC');
  $source = variable_get('csas_ytu_body', 'value');
  $query = db_select('node', 'n')
    ->fields('n', ['nid', 'type', 'title', 'created'])
    ->condition('n.status', NODE_PUBLISHED)
    ->condition('n.type', $node_types, 'IN');
  $query->leftJoin('field_data_body', 'd', 'd.entity_id = n.nid AND d.entity_type = :node', [
    ':node' => 'node',
  ]);
  $query->addField('d', 'body_' . $source, 'body');
  //join image field
  $image_fields = db_select('variable', 'v')
    ->fields('v')->condition('v.name', 'csas_ytu_image_field_bundle_%', 'LIKE')->execute()->fetchAllAssoc('name');
  foreach($image_fields as $image_field_name => $image_field_obj){
    $value = unserialize($image_field_obj->value);

    if(empty($value)){
      continue;
    }
    $img_field = $value;
    $img_table = 'field_data_' . $img_field;
    $img_field_value_name = $img_field . '_fid';
    if (db_table_exists($img_table)) {
      $query->leftJoin($img_table, $img_table,
        "{$img_table}.entity_type = :et AND {$img_table}.entity_id = n.nid AND {$img_table}.delta = 0",
        [':et' => 'node']
      );
      $query->addField($img_table, $img_field_value_name, $image_field_name . '_fid');
      $query->leftJoin('file_managed', $image_field_name, "{$image_field_name}.fid = {$img_table}.{$img_field_value_name}");
      $query->addField($image_field_name, 'uri', $image_field_name . '_uri');
    }
  }
  if ('Yes' == variable_get('csas_ytu_sticky', 'No')) {
    $query->orderBy('n.sticky', 'DESC');
  }
  if ('Yes' == variable_get('csas_ytu_promoted', 'No')) {
    $query->orderBy('n.promote', 'DESC');
  }
  if ($page_num) {
    $range_start = $start_index + ($page_num - 1) * $count_rec;
    $query->orderBy('n.' . $orderBy, $order)->range($range_start, $count_rec);
  }
  else {
    $query->orderBy('n.' . $orderBy, $order);
  }
  return $query;
}