/*
 * utils.c
 * Copyright (C) Clearscene Ltd 2008 <wbooth@essentialcollections.co.uk>
 * 
 * utils.c is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * utils.c is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <memory.h>
#include <stdlib.h>
#include <stdio.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <unistd.h> // for getpid & readlink
#include "main.h"
#include "debug.h"
#include "utils.h"

static char *ItoaDigits = 
"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";


// ritoa - recursive itoa
long int ritoa(long int val, long int topval, char *s, int base) {

    long int n = val / base;
    if (n > 0)
        topval = ritoa(n, topval, s+1, base);
    else
        *(s+1) = '\0';
    *s = ItoaDigits[ topval % base ];
    return(topval / base);

}


/* itoa - int to char
 *- mallocs a string of the right length
 * - calls ritoa to fill in the string for a given base
 */
extern char * itoa(long int val, int base) {

    int len;
    char *s,*buf;
    long int t = val;
    for (len=2; t; t /= base) len++ ; // quickie log_base

    if((buf = (char *) malloc(len)) == NULL)
        {
        debug_message("out of memory in itoa\n", ERROR);
        return "";
        }
    s = buf;
    if (val < 0)
        {
        *s++ = '-'; 
        val = -val;
        };
    len = (int) ritoa(val, val, s, base);
    return(buf);

}


/* load a file into a buffer */
extern int load_file_to_memory(const char *p_filename, unsigned char **result) {

    int size = 0;
    FILE *p_f = fopen(p_filename, "r");

    if (p_f == NULL) 
        {
        *result = NULL;
        fprintf(stderr,"Count not open file '%s'.\n", p_filename);
        return -1; // -1 means file opening fail
        }

    fseek(p_f, 0, SEEK_END);
    size = ftell(p_f);
    fseek(p_f, 0, SEEK_SET);

    if((*result = (unsigned char *)malloc(size+1)) == NULL)
        {
        debug_message("out of memory while reading file information\n", ERROR);
        return 0;
        }

    if (size != fread(*result, sizeof(char), size, p_f))
        {
        free(*result);
        return -2; // -2 means file reading fail
        }

    fclose(p_f);
    (*result)[size] = 0;
    return size;

}


extern void createDir_ifRequired(char *dir) {

    if (!g_file_test(dir, G_FILE_TEST_EXISTS))
        {
        // Create the directory
        debug_message("Created directory.", INFORMATION);
        g_mkdir(dir, 5570);
        if (!g_file_test(dir, G_FILE_TEST_EXISTS))
            {
            // Major error - do something!
            debug_message("Could not get to new directory.", ERROR);
            exit(1);
            }
        }

}



/* return the path where the executable is located
 * Gets the path name where this exe is installed
 * buffer = Where to format the path. Must be at least
 * PATH_MAX in size.
 */
/*
extern void get_exe_name(char *buffer) {

    char linkname[64];
    register pid_t pid;
    register unsigned long offset;

    pid = getpid();
    snprintf(&linkname[0], sizeof(linkname), "/proc/%i/exe", pid);
    if (readlink(&linkname[0], buffer, PATH_MAX) == -1)
        offset = 0;
    else
        {
        offset = strlen(buffer);
        while (offset && buffer[offset - 1] != '/') 
            --offset;
        if (offset && buffer[offset - 1] == '/') 
            --offset;
        }
    buffer[offset] = 0;
}
*/



/* fcopy - copy a file contents to new location */
extern void fcopy(char *fnsource, char *fntarget) {
/*
    FILE *fpin = fopen(fnsource, "rb");

    if(fpin != NULL)
        {
        FILE *fpout = fopen(fntarget, "wb");
        if(fpout != NULL)
            {
            int ch = 0;
            while((ch = getc(fpin)) != EOF)
                {
                putc(ch, fpout);
                }
            fclose(fpin);
            }
        fclose(fpout);
        }
*/
}

extern gboolean std_scrollEvent (GtkWidget *widget, GdkEventScroll *event, GtkRange *range ) {

    gboolean retval = FALSE;
/*
    GtkAdjustment *adj;

    adj = gtk_range_get_adjustment(range);
    switch (event->direction) 
        {
        case GDK_SCROLL_UP:
            gtk_range_set_value(range, gtk_range_get_value(range)-adj->step_increment);
            break;
        case GDK_SCROLL_DOWN:
            gtk_range_set_value(range, gtk_range_get_value(range)+adj->step_increment);
            break;
        default:
            break;
        }
*/
    return retval;
}

/*
extern void std_setFontSize (GtkWidget *widget, relativeSize size) {

    PangoContext *pangoContext;
    PangoFontDescription *fontDesc;

    pangoContext = gtk_widget_get_pango_context(GTK_WIDGET(filterTags));
    fontDesc = pango_context_get_font_description(pangoContext);
    pango_font_description_set_size(fontDesc, pango_font_description_get_size(fontDesc)*0.66);
    gtk_widget_modify_font (GTK_WIDGET(filterTags), fontDesc);
}*/

extern int max(int a, int b) {

    if(a >= b)
        return a;
    else
        return b;

}

extern int min(int a, int b) {

    if(a <= b)
        return a;
    else
        return b;

}

extern char *dateHuman(char *d, char *m, char *y) {

    // This will need to be converted, to use current machines LOCALE

    if(!g_str_equal(d, "NULL") && !g_str_equal(m, "NULL") && !g_str_equal(y, "NULL"))
        {
        conCat(&d, "/");
        conCat(&d, m);
        conCat(&d, "/");
        conCat(&d, y);
        free(m);
        free(y);
        return d;
        }
    else
        {
        free(d);
        free(m);
        free(y);
        return g_strdup("No date set");
        }
}

extern void conCat(char **mainStr, const char *addStr) {

    char *tmp, *tmp2;

    if(addStr && !g_str_equal(addStr, ""))
        {
        tmp2 = *mainStr;
        tmp = g_strconcat(tmp2, addStr, NULL);
        free(tmp2);
        *mainStr = tmp;
        }

}
