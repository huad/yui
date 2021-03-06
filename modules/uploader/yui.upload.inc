<?php

/**
 * @file
 *
 * Handles the upload callback for the file.
 */

/**
 * Handles the uploading of files.
 */
function yui_uploader_upload_file() {
  yui_uploader_restore_session();
  $form_build_id = $_POST['form_build_id'];
  foreach ($_FILES as $data) {
    if($data['error'] == 0) {
      $file = yui_uploader_save_uploaded_file($form_build_id, $data);
      if($file !== FALSE) { 
        yui_uploader_store_uploaded_file($form_build_id, $file);
      }
    } // @TODO log/display error to the user.
  }
  echo ' ';
  exit(); //Not Sure about this? Debug by Dong
}

/**
 * Saves the given file to the yui/uploader folder.
 */
function yui_uploader_save_uploaded_file($form_build_id, array $data) {
  global $user;
  $directory = file_directory_path() . '/yui/uploader';
  $basename = $form_build_id . '_' . $data['name'];
  $dest = file_create_filename($basename, $directory);
  if(move_uploaded_file($data['tmp_name'], $dest) !== FALSE) {
    $file = new stdClass();
    $file->uid = $user->uid;
    $file->filename = $data['name'];
    $file->filepath = $dest;
    $file->filemime = file_get_mimetype($data['name']);
    $file->filesize = $data['size'];
    $file->status = FILE_STATUS_TEMPORARY;
    $file->timestamp = time();
    drupal_write_record('files', $file);
    return $file;
  }
  return FALSE;
}


/**
 * Stores the uploaded file in the form_state.
 */
function yui_uploader_store_uploaded_file($form_build_id, stdClass $file) {
  $form_state = array('storage' => NULL, 'submitted' => FALSE, 'post' => $_POST);
  while(!lock_acquire('yui_upload_file')) {sleep(1);}; 
    $form = form_get_cache($form_build_id, $form_state);
    if (isset($form)) {
      $form_state['storage']['yui_uploader'][] = $file;
      form_set_cache($form_build_id, $form, $form_state);
   }
    lock_release('yui_upload_file');
}


/**
 * Because of limitiations on flash we have to restore the session from a POST
 */
function yui_uploader_restore_session() {
  if(user_is_anonymous()) {
    $name = $_POST['SNAME'];
    $id = $_POST['SID'];
    $_COOKIE[$name] = $id; // Used by Drupal session handlers.
    session_destroy(); // Forget the session of the anonymous user incorrectly created because of the missing cookies.
    session_start(); // Restart the session with the correct information.
  }
}
