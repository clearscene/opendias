
#include <poppler/cpp/poppler-document.h>
#include <poppler/cpp/poppler-image.h>
#include <poppler/cpp/poppler-page.h>
#include <poppler/cpp/poppler-page-renderer.h>
#include <stdio.h>

/*
 * Implement a skellington Poppler CPP API
 * For use in testing, so we can test the app, 
 * but not us poppler in the process.
 */
namespace poppler {

  byte_array ustring::to_utf8() const {
    const char *d = "placeholder text";
    std::vector<char> dd( d, d+17 );
    return dd;
  }


  ustring page::text(const rectf &rect, text_layout_enum layout_mode) const {
    ustring us;
    return us;
  }


  image page_renderer::render_page(const page *p,
                      double xres, double yres,
                      int x, int y, int w, int h,
                      rotation_enum rotate) const {
      image img;
      return img;
  }

  bool image::is_valid() const {
    return true;
  }

  bool image::save(const std::string &file_name, const std::string &out_format, int dpi) const {
    FILE *fp;

    if( (fp = fopen(file_name.c_str(), "a")) == NULL ) {
      return false;
    }

    char buf[BUFSIZ];
    size_t size;

    FILE* source = fopen("/tmp/poppler_override_output", "rb");
    while (size = fread(buf, 1, BUFSIZ, source)) {
        fwrite(buf, 1, size, fp);
    }

    fclose(source);
//    fprintf(fp,"test image data\n");
    fclose(fp);

    return true;
  }

}

