/*
 * scanner.c
 * Copyright (C) Clearscene Ltd 2008 <wbooth@essentialcollections.co.uk>
 * 
 * scan.c is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * scan.c is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/* Collection of sane related helper functions */

#include "config.h"

#ifdef CAN_SCAN
#include <stdlib.h>
#include <string.h>     // compares
#include <sane/sane.h>  // Scanner Interface
#include <sane/saneopts.h>  // Scanner Interface

#include "utils.h"
#include "debug.h"

#include "scanner.h"


void handleSaneErrors(char *defaultMessage, SANE_Status st, int retCode) {

  o_log(ERROR, "%s: sane error = %d (%s), return code = %d", defaultMessage, st, sane_strstatus(st), retCode);

}

/* 
 * Pritty much from here down is lifted from 'simple-scan'. 
 */

const char *get_status_string (SANE_Status status) {
    struct {
        SANE_Status status;
        const char *name;
    } status_names[] = {
        { SANE_STATUS_GOOD,          "SANE_STATUS_GOOD"},
        { SANE_STATUS_UNSUPPORTED,   "SANE_STATUS_UNSUPPORTED"},
        { SANE_STATUS_CANCELLED,     "SANE_STATUS_CANCELLED"},
        { SANE_STATUS_DEVICE_BUSY,   "SANE_STATUS_DEVICE_BUSY"},
        { SANE_STATUS_INVAL,         "SANE_STATUS_INVAL"},
        { SANE_STATUS_EOF,           "SANE_STATUS_EOF"},
        { SANE_STATUS_JAMMED,        "SANE_STATUS_JAMMED"},
        { SANE_STATUS_NO_DOCS,       "SANE_STATUS_NO_DOCS"},
        { SANE_STATUS_COVER_OPEN,    "SANE_STATUS_COVER_OPEN"},
        { SANE_STATUS_IO_ERROR,      "SANE_STATUS_IO_ERROR"},
        { SANE_STATUS_NO_MEM,        "SANE_STATUS_NO_MEM"},
        { SANE_STATUS_ACCESS_DENIED, "SANE_STATUS_ACCESS_DENIED"},
        { -1,                        NULL}
    };
    static char *unknown_string = NULL;
    int i;

    for (i = 0; status_names[i].name != NULL && status_names[i].status != status; i++);

    if (status_names[i].name == NULL) {
        free (unknown_string);
        unknown_string = o_strdup("");
        o_concatf (&unknown_string, "SANE_ACTION(%d)", status);
        return unknown_string; /* Note result is undefined on second call to this function */
    }

    return status_names[i].name;
}


const char *get_action_string (SANE_Action action) {
    struct {
        SANE_Action action;
        const char *name;
    } action_names[] = {
        { SANE_ACTION_GET_VALUE, "SANE_ACTION_GET_VALUE" },
        { SANE_ACTION_SET_VALUE, "SANE_ACTION_SET_VALUE" },
        { SANE_ACTION_SET_AUTO,  "SANE_ACTION_SET_AUTO" },
        { -1,                    NULL}
    };
    static char *unknown_string = NULL;
    int i;

    for (i = 0; action_names[i].name != NULL && action_names[i].action != action; i++);

    if (action_names[i].name == NULL) {
        free (unknown_string);
        unknown_string = o_strdup("");
        o_concatf (&unknown_string, "SANE_ACTION(%d)", action);
        return unknown_string; /* Note result is undefined on second call to this function */
    }

    return action_names[i].name;
}

void log_option (SANE_Int index, const SANE_Option_Descriptor *option) {
    char *string;
    SANE_Word i;
    SANE_Int cap;
    
    string = o_strdup("");
    o_concatf (&string, "Option %d:", index);
    
    if (option->name)    
        o_concatf (&string, " name='%s'", option->name);
    
    if (option->title)
        o_concatf (&string, " title='%s'", option->title);

    switch (option->type) {
    case SANE_TYPE_BOOL:
        conCat(&string, " type=bool");
        break;
    case SANE_TYPE_INT:
        conCat(&string, " type=int");
        break;
    case SANE_TYPE_FIXED:
        conCat(&string, " type=fixed");        
        break;
    case SANE_TYPE_STRING:
        conCat(&string, " type=string");        
        break;
    case SANE_TYPE_BUTTON:
        conCat(&string, " type=button");        
        break;
    case SANE_TYPE_GROUP:
        conCat(&string, " type=group");
        break;
    default:
        o_concatf (&string, " type=%d", option->type);
        break;
    }
    
    o_concatf (&string, " size=%d", option->size);

    switch (option->unit) {
    case SANE_UNIT_NONE:
        break;
    case SANE_UNIT_PIXEL:
        conCat(&string, " unit=pixels");
        break;
    case SANE_UNIT_BIT:
        conCat(&string, " unit=bits");
        break;
    case SANE_UNIT_MM:
        conCat(&string, " unit=mm");
        break;
    case SANE_UNIT_DPI:
        conCat(&string, " unit=dpi");
        break;
    case SANE_UNIT_PERCENT:
        conCat(&string, " unit=percent");
        break;
    case SANE_UNIT_MICROSECOND:
        conCat(&string, " unit=microseconds");
        break;
    default:
        o_concatf (&string, " unit=%d", option->unit);
        break;
    }

    switch (option->constraint_type) {
    case SANE_CONSTRAINT_RANGE:
        if (option->type == SANE_TYPE_FIXED)
            o_concatf (&string, " min=%f, max=%f, quant=%d",
                                    SANE_UNFIX (option->constraint.range->min), SANE_UNFIX (option->constraint.range->max),
                                    option->constraint.range->quant);
        else
            o_concatf (&string, " min=%d, max=%d, quant=%d",
                                    option->constraint.range->min, option->constraint.range->max,
                                    option->constraint.range->quant);
        break;
    case SANE_CONSTRAINT_WORD_LIST:
        conCat(&string, " values=[");
        for (i = 0; i < option->constraint.word_list[0]; i++) {
            if (i != 0)
                conCat(&string, ", ");
            if (option->type == SANE_TYPE_INT)
                o_concatf (&string, "%d", option->constraint.word_list[i+1]);
            else
                o_concatf (&string, "%f", SANE_UNFIX (option->constraint.word_list[i+1]));
        }
        conCat(&string, "]");
        break;
    case SANE_CONSTRAINT_STRING_LIST:
        conCat(&string, " values=[");
        for (i = 0; option->constraint.string_list[i]; i++) {
            if (i != 0)
                conCat(&string, ", ");
            o_concatf (&string, "\"%s\"", option->constraint.string_list[i]);
        }
        conCat(&string, "]");
        break;
    default:
        break;
    }
    
    cap = option->cap;
    if (cap) {
        struct {
            SANE_Int cap;
            const char *name;
        } caps[] = {
            { SANE_CAP_SOFT_SELECT,     "soft-select"},
            { SANE_CAP_HARD_SELECT,     "hard-select"},
            { SANE_CAP_SOFT_DETECT,     "soft-detect"},
            { SANE_CAP_EMULATED,        "emulated"},
            { SANE_CAP_AUTOMATIC,       "automatic"},
            { SANE_CAP_INACTIVE,        "inactive"},
            { SANE_CAP_ADVANCED,        "advanced"},
            { 0,                        NULL}
        };
        int i, n = 0;
        
        conCat(&string, " cap=");
        for (i = 0; caps[i].cap > 0; i++) {
            if (cap & caps[i].cap) {
                cap &= ~caps[i].cap;
                if (n != 0)
                    conCat(&string, ",");
                conCat(&string, caps[i].name);
                n++;
            }
        }
        /* Unknown capabilities */
        if (cap) {
            if (n != 0)
                conCat(&string, ",");
            o_concatf (&string, "%x", cap);
        }
    }

    o_log(DEBUGM, "%s", string);
    free(string);

//    if (option->desc)
//      o_log(DEBUGM, "  Description: %s", option->desc);
}

SANE_Status control_option (SANE_Handle handle, const SANE_Option_Descriptor *option, SANE_Int index, SANE_Action action, void *value, int *ret) {
    SANE_Status status;
    char *proposed_value;
    char *scanners_value;

    // See what the device has set for this option. Compare it to what we want to set
    switch (option->type) {
      case SANE_TYPE_BOOL:
        proposed_value = o_printf (*((SANE_Bool *) value) ? "SANE_TRUE" : "SANE_FALSE");
        SANE_Bool v_b;
        status = sane_control_option (handle, index, SANE_ACTION_GET_VALUE, &v_b, NULL);
        scanners_value = o_printf ((SANE_Bool) v_b ? "SANE_TRUE" : "SANE_FALSE");
        break;

      case SANE_TYPE_INT:
        proposed_value = o_printf ("%d", *((SANE_Int *) value));
        int v_i;
        status = sane_control_option (handle, index, SANE_ACTION_GET_VALUE, &v_i, NULL);
        scanners_value = o_printf ("%d", v_i);
        break;

      case SANE_TYPE_FIXED:
        proposed_value = o_printf ("%f", SANE_UNFIX (*((SANE_Fixed *) value)));
        SANE_Fixed v_f = SANE_FIX( 9999999999 );
        status = sane_control_option (handle, index, SANE_ACTION_GET_VALUE, &v_f, NULL);
        scanners_value = o_printf ("%f", SANE_UNFIX( v_f ) );
        break;

      case SANE_TYPE_STRING:
        proposed_value = o_printf ("\"%s\"", (char *) value);
        char *v_c = malloc( sizeof( char ) * option->size );
        status = sane_control_option (handle, index, SANE_ACTION_GET_VALUE, (void *)v_c, NULL);
        scanners_value = o_printf ("\"%s\"", (char *) v_c);
        free(v_c);
        break;

      default:
        proposed_value = o_strdup ("--unknwon-request-type--");
        scanners_value = o_strdup ("--unknown-scanner-type--");
        break;
    }

    if ( 0 == strcmp(proposed_value, scanners_value) ) {
      // The SANE spec says we were only supposed to get SANE_INFO_RELOAD_OPTIONS when we 
      // UPDATED something? Never mind...
      // Were not setting the device, since some options will request we reload all options.
      // if that happenes and we set the value again the next time around - we end up in an
      // infinate loop
      o_log( DEBUGM, "The scanner reports having \"%s\" already set to %s. Not attempting to update.", option->name, scanners_value );
      free (proposed_value);
      free (scanners_value);
      return SANE_STATUS_GOOD;
    }

    // So, we need to actualy set the value
    o_log( INFORMATION, "Setting scanner param %s to %s", option->name, proposed_value);
    status = sane_control_option (handle, index, action, value, ret);

    switch (option->type) {
      case SANE_TYPE_BOOL:
        o_log(DEBUGM, "sane_control_option (%d, %s, %s) -> (%s, was:%s, now:%s)",
                 index, get_action_string (action),
                 *((SANE_Bool *) value) ? "SANE_TRUE" : "SANE_FALSE",
                 get_status_string (status),
                 scanners_value, proposed_value);
        break;

      case SANE_TYPE_INT:
        o_log(DEBUGM, "sane_control_option (%d, %s, %d) -> (%s, was:%s, now:%s)",
                 index, get_action_string (action),
                 *((SANE_Int *) value),
                 get_status_string (status),
                 scanners_value, proposed_value);
        break;

      case SANE_TYPE_FIXED:
        o_log(DEBUGM, "sane_control_option (%d, %s, %f) -> (%s, was:%s, now:%s)",
                 index, get_action_string (action),
                 SANE_UNFIX (*((SANE_Fixed *) value)),
                 get_status_string (status),
                 scanners_value, proposed_value);
        break;

      case SANE_TYPE_STRING:
        o_log(DEBUGM, "sane_control_option (%d, %s, %s) -> (%s, was:%s, now:%s)",
                 index, get_action_string (action),
                 (char *) value,
                 get_status_string (status),
                 scanners_value, proposed_value);
        break;

      default:
        break;
    }

    free (proposed_value);
    free (scanners_value);

    if ( *ret & SANE_INFO_RELOAD_OPTIONS ) 
      o_log( DEBUGM, "This setting update has requested that we reload all previous settings." );

    if ( status != SANE_STATUS_GOOD )
      o_log( WARNING, "Error setting option %s: %s", option->name, sane_strstatus(status));

    return status;
}

int setDefaultScannerOption(SANE_Handle *devHandle, const SANE_Option_Descriptor *sod, int option, int *ret ) {
  if ( sod->cap & SANE_CAP_AUTOMATIC ) {
    int status = control_option (devHandle, sod, option, SANE_ACTION_SET_AUTO, NULL, ret);
    if(status == SANE_STATUS_GOOD) {
      handleSaneErrors("Cannot set automatically", status, *ret);
      //updateScanProgress(uuid, SCAN_ERRO_FROM_SCANNER, status);
      return 1;
    }
  }
  return 0;
}

#endif // CAN_SCAN //
