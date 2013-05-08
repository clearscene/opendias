#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sane/sane.h>
#include <sane/saneopts.h>

SANE_Device *dev[2];
SANE_Option_Descriptor *sod;

SANE_Int option_count = 4;
SANE_Int depth = 8;
SANE_Fixed resolution = SANE_FIX( 300 );

static SANE_Range resolution_range = {
  SANE_FIX (1.0),
  SANE_FIX (800.0),
  SANE_FIX (1.0)
};

static SANE_Int depth_list[] = {
  3, 1, 8, 16
};

static SANE_String_Const mode_list[] = {
  SANE_VALUE_SCAN_MODE_GRAY,
  SANE_VALUE_SCAN_MODE_COLOR,
  0
};

static size_t max_string_size (const SANE_String_Const strings[]) {
  size_t size, max_size = 0;
  SANE_Int i;

  for (i = 0; strings[i]; ++i) {
    size = strlen (strings[i]) + 1;
    if (size > max_size)
      max_size = size;
  }
  return max_size;
}

SANE_Status sane_init(SANE_Int *version, SANE_Auth_Callback authorize ) {
  dev[0] = malloc( sizeof( SANE_Device ) );
  dev[0]->name = "test:0";
  dev[0]->vendor = "opendias-overridden-sane-libs";
  dev[0]->model = "frontend-tester";
  dev[0]->type = "virtual device";

  dev[1] = malloc( sizeof( SANE_Device ) );
  dev[1]->name = "test:1";
  dev[1]->vendor = "opendias-overridden-sane-libs";
  dev[1]->model = "timeout frontend tester";
  dev[1]->type = "virtual device";

  dev[2] = NULL;

  return SANE_STATUS_GOOD;
}

SANE_Status sane_get_devices (const SANE_Device ***device_list, SANE_Bool local_only) {

  *device_list = (const SANE_Device **)dev;
  return SANE_STATUS_GOOD;
}

SANE_Status sane_open (SANE_String_Const devicename, SANE_Handle *handle) {
  if ( 0 == strcmp(devicename, "test:1") ) {
    return SANE_STATUS_IO_ERROR;
  }
  else {
    return SANE_STATUS_GOOD;
  }
}

SANE_Status sane_start (SANE_Handle handle) {
  return SANE_STATUS_GOOD;
}

const SANE_Option_Descriptor *sane_get_option_descriptor (SANE_Handle handle, SANE_Int option) {
  if( sod ) {
    free( sod );
  }
  sod = malloc( sizeof( SANE_Option_Descriptor ) );
  switch ( option ) {
    case 0: {
      sod->name = "";
      sod->title = SANE_TITLE_NUM_OPTIONS;
      sod->desc = SANE_DESC_NUM_OPTIONS;
      sod->type = SANE_TYPE_INT;
      sod->unit = SANE_UNIT_NONE;
      sod->size = sizeof (SANE_Word);
      sod->cap = SANE_CAP_SOFT_DETECT;
      sod->constraint_type = SANE_CONSTRAINT_NONE;
      sod->constraint.range = 0;
      break;
    }
    case 1: {
      sod->name = "";
      sod->title = SANE_I18N ("Scan Mode");
      sod->desc = "";
      sod->type = SANE_TYPE_GROUP;
      sod->unit = SANE_UNIT_NONE;
      sod->size = 0;
      sod->cap = 0;
      sod->constraint_type = SANE_CONSTRAINT_NONE;
      sod->constraint.range = 0;
      break;
    }
    case 2: {
      sod->name = SANE_NAME_SCAN_MODE;
      sod->title = SANE_TITLE_SCAN_MODE;
      sod->desc = SANE_DESC_SCAN_MODE;
      sod->type = SANE_TYPE_STRING;
      sod->unit = SANE_UNIT_NONE;
      sod->size = max_string_size (mode_list);
      sod->cap = SANE_CAP_SOFT_DETECT | SANE_CAP_SOFT_SELECT;
      sod->constraint_type = SANE_CONSTRAINT_STRING_LIST;
      sod->constraint.string_list = mode_list;
      break;
    }
    case 3: {
      sod->name = SANE_NAME_BIT_DEPTH;
      sod->title = SANE_TITLE_BIT_DEPTH;
      sod->desc = SANE_DESC_BIT_DEPTH;
      sod->type = SANE_TYPE_INT;
      sod->unit = SANE_UNIT_NONE;
      sod->size = sizeof (SANE_Word);
      sod->cap = SANE_CAP_SOFT_DETECT | SANE_CAP_SOFT_SELECT;
      sod->constraint_type = SANE_CONSTRAINT_WORD_LIST;
      sod->constraint.word_list = depth_list;
      break;
    }
    case 4: {
      sod->name = SANE_NAME_SCAN_RESOLUTION;
      sod->title = SANE_TITLE_SCAN_RESOLUTION;
      sod->desc = SANE_DESC_SCAN_RESOLUTION;
      sod->type = SANE_TYPE_FIXED;
      sod->unit = SANE_UNIT_DPI;
      sod->size = sizeof (SANE_Word);
      sod->cap = SANE_CAP_SOFT_DETECT | SANE_CAP_SOFT_SELECT;
      sod->constraint_type = SANE_CONSTRAINT_RANGE;
      sod->constraint.range = &resolution_range;
      break;
    }
    default:
      return NULL;
  }
  return sod;
}

SANE_Status sane_control_option( SANE_Handle handle, SANE_Int option, SANE_Action action, void *value, SANE_Int *info) {
  if( action == SANE_ACTION_GET_VALUE ) {
    switch ( option ) {
      case 0: {
        // option count
        *(SANE_Word *) value = option_count;
        break;
      }
      case 1: {
        value = NULL;
        break;
      }
      case 2: {
        // mode
        strcpy(value, SANE_VALUE_SCAN_MODE_GRAY);
        break;
      }
      case 3: {
        // depth
        *(SANE_Int *)value = depth;
        break;
      }
      case 4: {
        // resolution
        *(SANE_Fixed *)value = resolution;
        break;
      }
      default: {
        break;
      }
    }
  }
  return SANE_STATUS_GOOD;
}

SANE_Status sane_get_parameters( SANE_Handle handle, SANE_Parameters *pars) {
  pars->last_frame = 1;
  pars->bytes_per_line = 787;
  pars->pixels_per_line = 787;
  pars->lines = 787;
  pars->depth = 8;
  pars->format = SANE_FRAME_GRAY;
  return SANE_STATUS_GOOD;
}

SANE_Status sane_read(SANE_Handle handle, SANE_Byte *data, SANE_Int max_length, SANE_Int *length) {
  int i;
  for(i=0;i<161;i++) {
    *(data+(1*i))='0';
    *(data+(2*i))='3';
    *(data+(3*i))='F';
  }
  *(data+376) = 'F';
  *length = 787;

  // Tests can block this, to check what happens when the app tries to access sane while it's busy
  if ( 0 == access( "/tmp/pause.sane.override", F_OK ) ) {
    // Let the test system know were blocked
    FILE *f = fopen( "/tmp/sane.override.is.paused", "w+" );
    fprintf( f, "blocked" );
    fclose( f );
    // wait for the green light
    while ( 0 == access( "/tmp/pause.sane.override", F_OK ) ) {
      usleep( 5000 );
    }
    // cleanup
    unlink( "/tmp/sane.override.is.paused" );
  }

  usleep( 5000 );

  return SANE_STATUS_GOOD;
}
SANE_String_Const sane_strstatus(SANE_Status st) {
  if( st == SANE_STATUS_GOOD ) {
    return "SUCCESS";
  }
  else {
    return "SANE ERROR MESSAGE";
  }
}

void sane_cancel(SANE_Handle handle) {
}

void sane_close(SANE_Handle handle) {
  free( sod );
  sod = NULL;
}

void sane_exit( void ) {
  free( dev[0] );
  free( dev[1] );
  dev[0] = NULL;
  dev[1] = NULL;
}

