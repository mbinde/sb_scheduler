<?php

/*****************************************************************************
 *****************************************************************************
 * 
 * Calendar functionality
 *
 *****************************************************************************
 *****************************************************************************/

function sb_my_calendar_display($target_user = "") {
  global $user;

  if ($target_user) {
    $target_user = user_load(array('name' => $target_user));
    if ($target_user->uid != $user->uid) {
      if (!user_access('view others schedule')) {
	return drupal_not_found();
      }
    }
    $base_path = 'users/' . $user->name . '/calendar';
  } else {
    $target_user = $user;
    $base_path = 'user/calendar';
  }

  drupal_set_title("Calendar for " . $user->name);

  if (user_access('create_schedule')) {
    $output .= "<p>" . l("+Schedule a block of time", $base_path . '/add') . "</p>";
  }

  $output .= "Calendar goes here";

  return $output;
}

/*****************************************************************************
 *****************************************************************************
 * 
 * Calendar functionality
 *
 *****************************************************************************
 *****************************************************************************/

function user_schedule_display($name = "") {
  jquery_ui_add('ui.datepicker');
  drupal_add_js(drupal_get_path('module', 'sb_scheduler') . '/js/choose-date.js');

  if ($name) {
    $user = user_load(array('name' => $name));
  }
  if (!$user) {
    return drupal_not_found();
  }

  $output = "";

  $output .= '<div id="choose-date"></div><div id="date-details"></div>';

  return $output;
}

/*
 * callback for displaying a user's schedule
 */

function callback_user_schedule_fetch () {
  $output = "";

  $target_date = $_POST['date']; // MM/DD/YYYY
  $target_user = $_POST['user'];

  // rewrite our target date
  $d = split('/', $target_date);
  $db_date_start = $d[2] . "-" . $d[0] . "-" . $d[1] . " 00:00:00";
  $db_date_start = date_make_date($db_date_start, user_tz());
  date_timezone_set($db_date_start, timezone_open('UTC'));
  $db_date_end = $d[2] . "-" . $d[0] . "-" . $d[1] . " 23:59:59";
  $db_date_end = date_make_date($db_date_end, user_tz());
  date_timezone_set($db_date_end, timezone_open('UTC'));

  $target_date = $d[2] . "-" . $d[0] . "-" . $d[1] . " 00:00:00";
  $target_date = date_make_date($target_date, user_tz());
  $tz_string = date_format($target_date, "T");

  $return = db_query("SELECT * FROM sb_schedule_block_event 
                      WHERE start_datetime_utc BETWEEN '%s' AND '%s'
                      ORDER BY start_datetime_utc ASC",
		     date_format($db_date_start, DATE_FORMAT_DATETIME), 
		     date_format($db_date_end, DATE_FORMAT_DATETIME));

  print "<p>";
  while ($obj = db_fetch_object($return)) {
    $events[] = $obj;
    print_r2($obj);
  }

  $output .= "<h2>Available times ($tz_string) on " . date_format($target_date, "M jS, Y") . ":</h2>";
  if ($events) {

    foreach ($events as $event) {
      $start = date_make_date($event->start_datetime_utc, "UTC");
      date_timezone_set($start, timezone_open($tz_string));
      $end = date_make_date($event->end_datetime_utc, "UTC");
      date_timezone_set($end, timezone_open($tz_string));

      // later we'll check what things are booked.  For now, just make URLs
      $interval = new DateInterval("PT15M");
      $continue = 1;
      $count = 0;
      while ($continue == 1) {
	$arguments = $event->parent_schedule_block_id . '/' . $event->schedule_block_event_id . '/' . $count;
	$url = 'users/' . $target_user . '/schedule/appointment/' . sb_encode($arguments);
	$output .= l(date_format($start, 'g:ia'), $url) . "<br>";
	$start = date_add($start, $interval);
	if ($start > $end) {
	  $continue = 0;
	}
	$count ++;
      }
    }


  } else {
    $output .= "<em>No available times found, please try another day.</em>";
  }

  print $output;
}



/*****************************************************************************
 *****************************************************************************
 * 
 * Calendar functionality
 *
 *****************************************************************************
 *****************************************************************************/

function user_schedule_appointment_display ($name, $arg) {

  if ($name) {
    $calendar_user = user_load(array('name' => $name));
  }
  if (!$calendar_user) {
    return drupal_not_found();
  }

  $app = _schedule_decode_time_args($arg);
  $title = "Schedule session with " . $name . " on " . date_format($app['target_date_user'], 'M jS, Y') . " at " . date_format($app['target_date_user'], 'g:ia');
  drupal_set_title($title);

  $output .= "<h1>" . $title . "</h1>";

  global $user;
  $type = "sb_appointment";
  $node = array('uid' => $user->uid, 'name' => (isset($user->name) ? $user->name : ''), 'type' => $type, 'language' => '');
  $node['tutor_uid'] = $calendar_user->uid;
  $node['start_datetime_utc'] = date_format($app['target_date_utc'], DATE_FORMAT_DATETIME);
  
  $output .= drupal_get_form('sb_appointment_form', (object) $node);
  
  return $output;
}

function user_schedule_appointment_edit($id) {
  $app = sb_appointment_load($id);
  if (! $app) {
    return drupal_not_found();
  }

  $date = date_make_date($app->start_datetime_utc, 'UTC');
  date_timezone_set($date, timezone_open(user_tz()));
  
  // else
  $title = "Edit Session with " . _sb_format_user($app->tutor_uid) . " on " . date_format($date, 'M jS, Y') . " at " . date_format($date, 'g:ia');
  drupal_set_title($title);

  $output .= "<h1>$title</h1>";
  
  
  $output .= drupal_get_form('sb_appointment_form', $app);

  return $output;
}