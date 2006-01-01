 *   NOTES
 *   only deal with posts and attachments
 *   no bbcode conversion except [quote]
 *   no user conversion
 *   overwrite all exists
 *   tested on vbb 3.0.0
 *   
 *   STEPS
 *   BACKUP all
 *   edit convert.h first
 *   type make
 *   STOP daemons and CLEAN shms (suggested)
 *   ./convert
 *   $BBSHOME/bin/gen_title (suggested)
 *
 *   WARNING
 *   very crappy code!
 *   ur own risk!
