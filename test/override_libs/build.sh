#
# Build override libraries - stub implementations of the real thing.
#

# Tesseract OCR
rm -f libtesseract.so* TessBaseAPI.o
g++ -fPIC -g -c -Wall TessBaseAPI.cc
g++ -shared -Wl,-soname,libtesseract.so.3 -o libtesseract.so.3.0.2 TessBaseAPI.o -lc
ln -s libtesseract.so.3.0.2 libtesseract.so.3
ln -s libtesseract.so.3 libtesseract.so


# Sane
rm -f libsane.so* ScannerList.o
gcc -fPIC -g -c -Wall ScannerList.c
g++ -shared -Wl,-soname,libsane.so.1 -o libsane.so.1.0.22 ScannerList.o -lc
ln -s libsane.so.1.0.22 libsane.so.1
ln -s libsane.so.1 libsane.so


