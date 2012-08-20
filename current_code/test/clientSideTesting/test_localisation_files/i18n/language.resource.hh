# No translations required in:
#   dbaccess.c
#   db.c
#   debug.c
#   imageProcessing.c
#   localisation.c
#   main.c
#   odf_plug.c
#   saneDispatcher.c
#   scanner.c
#   simpleLinkedList.c
#   utils.c
#   validation.c
#

#########################################

#
# doc_editor.c
#

# getDocDetail() - see docFilter

#########################################

#
# import_doc.c
#

# uploadfile()
LOCAL_ocr_default_text|--#### ##### ### ## #########--

#########################################

#
# pageRender.c
#

# docFilter() - shared with getDocDetail
LOCAL_file_type_odf|######## ODF ###
LOCAL_file_type_pdf|######## PDF ###
LOCAL_file_type_image|######## #####
LOCAL_file_type_scanned|####### ###
LOCAL_default_title|### (########) ########.

#########################################

#
# utils.c
#

LOCAL_no_date_set| - ## #### ### -

#########################################

#
# scan.c
#

# ocrImage()
LOCAL_page_delimiter|---------------- #### %d ----------------\n%s\n
LOCAL_resolution_outside_range_to_attempt_ocr|########## ### ####### #### ##### ## ####### OCR.\n 

# internalDoScanningOperation()
LOCAL_opendias_server|opendias ######

#########################################

#
# web_handler.c
#
LOCAL_server_busy|#### ###### ## ####, ###### ### ##### #####.
LOCAL_server_error|## ######## ###### ##### ### #######.
LOCAL_request_error|#### ####### ### ### ### ### #### 'http://&lt;host&gt;(:&lt;port&gt;)/opendias/(&ltrequest&gt;)'.
LOCAL_access_denied|###### ######
LOCAL_no_access|### ## ### #### ########### ## ######## ### #######
LOCAL_processing_error|#### ####### ##### ### ## #########
LOCAL_missing_support|####### ### #### ####### ### ### #### ######## ##
