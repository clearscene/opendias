#
# Build override libraries - stub implementations of the real thing.
#

# Tesseract OCR
cd libtesseract
echo Building libtesseract
rm -f libtesseract.so* libtesseract.o
g++ -fPIC -O0 -g -c -Wall libtesseract.cc
g++ -shared -Wl,-soname,libtesseract.so.3 -o libtesseract.so.3.0.2 libtesseract.o -lc
ln -s libtesseract.so.3.0.2 libtesseract.so.3
ln -s libtesseract.so.3 libtesseract.so
cd ../


# Sane
cd libsane
echo Building libsane
rm -f libsane.so* libsane.o
gcc -fPIC -O0 -g -c -Wall libsane.c
g++ -shared -Wl,-soname,libsane.so.1 -o libsane.so.1.0.22 libsane.o -lc
ln -s libsane.so.1.0.22 libsane.so.1
ln -s libsane.so.1 libsane.so
cd ../


# Leptonica
cd liblept
echo Building liblept
rm -f liblept.so* liblept.o
gcc -fPIC -O0 -g -c -Wall liblept.c
g++ -shared -Wl,-soname,liblept.so.3 -o liblept.so.3.0.0 liblept.o -lc
ln -s liblept.so.3.0.0 liblept.so.3
ln -s liblept.so.3 liblept.so
cd ../


# pHash
cd libpHash
echo Building libpHash
rm -f libpHash.so* libpHash.o
g++ -fPIC -O0 -g -c -Wall libpHash.cc > libpHash_override_build.log
g++ -shared -Wl,-soname,libpHash.so.0 -o libpHash.so.0.0.0 libpHash.o -lc
ln -s libpHash.so.0.0.0 libpHash.so.0
ln -s libpHash.so.0 libpHash.so
cd ../


# Poppler
#
# Poppler is a wee bit more complex than other libs. Other libs we can just implement the methods
# we know we need, poppler has a deep structure of interdependency, so that would mean we'de 
# have to stub out dozens on methods, just to override the one we're interested in.
# Therefore, we just compile the bits were interested in and then link in the full static lib
# into our skellinton shared lib, we're setting ignore duplicate defs which will override the
# method provided by the static lib (with ours), but make all the others available unchanged.
#
# Also, the image created when "a PDF is written as an image" is a copy of a file found
# at "/tmp/poppler_override_output"
#
cd libpoppler
echo Building libpoppler
cp poppler_override_output /tmp/poppler_override_output
rm -f libpoppler-cpp.so* libpoppler.o
g++ -fPIC -O0 -g -c -Wall libpoppler.cc
g++ -shared -Wl,-soname,libpoppler-cpp.so.0 -o libpoppler-cpp.so.0.2.0 -Wl,-zmuldefs libpoppler.o -Wl,-whole-archive /usr/lib/i386-linux-gnu/libpoppler-cpp.a -Wl,-no-whole-archive -lc -lpoppler 2> /dev/null
ln -s libpoppler-cpp.so.0.2.0 libpoppler-cpp.so.0
ln -s libpoppler-cpp.so.0 libpoppler-cpp.so
cd ../


