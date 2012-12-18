#
# Build override libraries - stub implementations of the real thing.
#

# Tesseract OCR
cd libtesseract
echo Building libtesseract
rm -f libtesseract.so* libtesseract.o
g++ -fPIC -g -c -Wall libtesseract.cc
g++ -shared -Wl,-soname,libtesseract.so.3 -o libtesseract.so.3.0.2 libtesseract.o -lc
ln -s libtesseract.so.3.0.2 libtesseract.so.3
ln -s libtesseract.so.3 libtesseract.so
cd ../


# Sane
cd libsane
echo Building libsane
rm -f libsane.so* libsane.o
gcc -fPIC -g -c -Wall libsane.c
g++ -shared -Wl,-soname,libsane.so.1 -o libsane.so.1.0.22 libsane.o -lc
ln -s libsane.so.1.0.22 libsane.so.1
ln -s libsane.so.1 libsane.so
cd ../


# Leptonica
cd liblept
echo Building liblept
rm -f liblept.so* liblept.o
gcc -fPIC -g -c -Wall liblept.c
g++ -shared -Wl,-soname,liblept.so.3 -o liblept.so.3.0.0 liblept.o -lc
ln -s liblept.so.3.0.0 liblept.so.3
ln -s liblept.so.3 liblept.so
cd ../


