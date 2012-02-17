BEGIN TRANSACTION;
INSERT INTO docs (docid, doneocr, ocrtext, depth, lines, ppl, resolution, docdatey, docdatem, docdated, entrydate, filetype, title, pages, actionrequired) VALUES (1,1,'This is the OCR text.',8,3509,2519,300,2010,12,31,'2011-01-02T21:13:04.393946Z',2,'Test Title',1,0);
COMMIT;
