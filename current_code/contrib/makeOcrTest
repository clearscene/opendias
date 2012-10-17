rm ocrTest ocrTest.o 
gcc -I. -I.. -g -c -o ocrTest.o ocrTest.c
g++ -g -o ocrTest ocrTest.o ocr_plug.o -lsqlite3 -lmicrohttpd -luuid -lglib-2.0  -lfreeimage  -lsane  -ltesseract_full -ltiff  -lespeak  -larchive -lxml2 -lzzip
