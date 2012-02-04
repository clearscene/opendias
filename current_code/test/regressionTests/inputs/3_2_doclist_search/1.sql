BEGIN TRANSACTION;
INSERT INTO docs (docid, doneocr, ocrtext, depth, lines, ppl, resolution, docdatey, docdatem, docdated, entrydate, filetype, title, pages, actionrequired) VALUES (1,1,'This is the OCR text.',8,3509,2519,300,2010,12,31,'2011-01-02T21:13:04.393946Z',2,'Test Titletext',1,0);
INSERT INTO docs (docid, doneocr, ocrtext, depth, lines, ppl, resolution, docdatey, docdatem, docdated, entrydate, filetype, title, pages, actionrequired) VALUES (4,1,'This is OCR text 4.',8,3509,2519,300,2010,12,30,'2011-01-03T21:13:04.393946Z',4,'Test 1 TitleText 2',1,0);
INSERT INTO docs (docid, doneocr, ocrtext, depth, lines, ppl, resolution, docdatey, docdatem, docdated, entrydate, filetype, title, pages, actionrequired) VALUES (3,1,'This is OCR text 2.',8,3509,2519,300,2011,01,01,'2011-01-04T21:13:04.393946Z',2,'Test 2 Title text',1,1);
INSERT INTO docs (docid, doneocr, ocrtext, depth, lines, ppl, resolution, docdatey, docdatem, docdated, entrydate, filetype, title, pages, actionrequired) VALUES (2,1,'This is final OCR text 3.',8,3509,2519,300,2012,12,31,'2011-01-04T21:13:05.393946Z',3,'Test 3 Title',1,1);
COMMIT;
