
#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>

/*
 * Implement a skellington Tesseract Base API
 * For use in testing, so we can test the app, 
 * but not us tesseract in the process.
 */
namespace tesseract {

  const char *TessBaseAPI::Version( ) { return "Overridden Tesseract Lib 0.1"; }

  int TessBaseAPI::Init( const char* datapath, const char* language, OcrEngineMode mode,
           char **configs, int configs_size,
           const GenericVector<STRING> *vars_vec,
           const GenericVector<STRING> *vars_values,
           bool set_only_non_debug_params ) {
    return 0;
  }

  const char *TessBaseAPI::GetInitLanguagesAsString() const { return "ZZZ"; }

  void TessBaseAPI::SetImage( const Pix* pix ) { }

  void TessBaseAPI::SetSourceResolution( int ppi ) { };  

  char *TessBaseAPI::GetUTF8Text( ) { char *txt = new char[20]; strcpy( txt, "Example OCR output." ); return txt; }

  void TessBaseAPI::Clear( ) { };

  void TessBaseAPI::End( ) { };

}
