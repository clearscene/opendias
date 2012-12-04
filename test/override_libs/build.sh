#
# Build override libraries - stub implementations of the real thing.
#

# Tesseract OCR
rm -f libtesseract.so* TessBaseAPI.o
g++ -fPIC -g -c -Wall TessBaseAPI.cc
g++ -shared -Wl,-soname,libtesseract.so.3 -o libtesseract.so.3.0.2 TessBaseAPI.o -lc
ln -s libtesseract.so.3.0.2 libtesseract.so.3
ln -s libtesseract.so.3 libtesseract.so


