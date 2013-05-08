/*
 * pdf_plug.c
 * Copyright (C) Clearscene Ltd 2008 <wbooth@essentialcollections.co.uk>
 * 
 * pdf_plug.c is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * pdf_plug.c is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"

#ifdef CAN_PDF
#include <utils.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <poppler/cpp/poppler-document.h>
#include <poppler/cpp/poppler-image.h>
#include <poppler/cpp/poppler-page.h>
#include <poppler/cpp/poppler-page-renderer.h>
#include <cstdlib>
#include <iostream>
#include <memory>

#include "debug.h"

#include "pdf_plug.h"

#ifndef NULL
#define NULL 0L
#endif /* NULL */

extern "C" char *parse_pdf( const char *pdf_filename, const char *out_filename ) {

  if (!poppler::page_renderer::can_render()) {
    o_log(ERROR, "renderer compiled without Splash support");
  }

  const std::string file_name(pdf_filename);
  std::auto_ptr<poppler::document> doc( poppler::document::load_from_file(file_name) );

  if ( ! doc.get() ) {
    o_log(ERROR, "PDF could not be loaded");
  }

  if (doc->is_locked()) {
    o_log(ERROR, "encrypted document");
  }

  int doc_page = 0; 
  std::auto_ptr<poppler::page> p( doc->create_page(doc_page) );

  if ( ! p.get() ) {
    o_log(ERROR, "Could not get the PDF page.");
  }
  else {
    o_log(ERROR, "PDF page successfuly parsed");
  }

  //
  // Extract image
  //
  poppler::page_renderer pr;
  pr.set_render_hint(poppler::page_renderer::antialiasing, true);
  pr.set_render_hint(poppler::page_renderer::text_antialiasing, true);
 
  poppler::image img = pr.render_page(p.get(), 170, 170); // 170 resolution
  if ( ! img.is_valid() ) {
    o_log(ERROR, "rendering failed");
  }
  else {
    o_log(ERROR, "Image generated from the PDF");
  }
 
  if ( ! img.save(out_filename, "jpg") ) {
    o_log(ERROR, "saving to file failed");
  }
  else {
    o_log(DEBUGM, "Thumbnail saved");
  }

  //
  // extract text
  //
  poppler::byte_array ocr_text_ba = p->text( p->page_rect(), poppler::page::physical_layout ).to_utf8(); 
  ocr_text_ba.push_back( 0 ); // Add a NULL terminator for the C char *
  std::string ocr_text( ocr_text_ba.begin(), ocr_text_ba.end() );
  o_log(ERROR, "Text Extracted from the PDF");

  return strdup( ocr_text.c_str() );
}

#endif /* CAN_PDF */
