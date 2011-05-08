BEGIN TRANSACTION;

CREATE TABLE version (
	version INTEGER PRIMARY KEY,
	for_app_version NUMERIC,
	version_name TEXT);

CREATE UNIQUE INDEX version_idx 
ON version(version ASC);

DELETE FROM version;

INSERT INTO version VALUES(1,0.1,'Draft');

CREATE TABLE docs (
	docid INTEGER PRIMARY KEY, 
	doneocr NUMERIC, 
	ocrtext TEXT, 
	depth NUMERIC, 
	lines NUMERIC, 
	ppl NUMERIC, 
	resolution NUMERIC, 
	docdatey NUMERIC, 
	docdatem NUMERIC, 
	docdated NUMERIC, 
	entrydate TEXT, 
	filetype NUMERIC, 
	title TEXT);

CREATE TABLE tags (
	tagid INTEGER PRIMARY KEY, 
	tagname TEXT);

CREATE UNIQUE INDEX tagname_idx 
ON tags(tagname ASC);

INSERT INTO tags VALUES(1,'Bank');
INSERT INTO tags VALUES(2,'Utility');
INSERT INTO tags VALUES(3,'Tax');
INSERT INTO tags VALUES(4,'Household');
INSERT INTO tags VALUES(5,'Account');
INSERT INTO tags VALUES(6,'Credit Card');
INSERT INTO tags VALUES(7,'Gas');
INSERT INTO tags VALUES(8,'Electricity');
INSERT INTO tags VALUES(9,'Water');
INSERT INTO tags VALUES(10,'Government');
INSERT INTO tags VALUES(11,'Local');
INSERT INTO tags VALUES(12,'Personal');
INSERT INTO tags VALUES(13,'Morgadge');
INSERT INTO tags VALUES(14,'Insurance');
INSERT INTO tags VALUES(15,'Car');
INSERT INTO tags VALUES(16,'Land-Line');
INSERT INTO tags VALUES(17,'Mobile');
INSERT INTO tags VALUES(18,'Internet');
INSERT INTO tags VALUES(19,'TV');
INSERT INTO tags VALUES(20,'Letter');
INSERT INTO tags VALUES(21,'Statement');
INSERT INTO tags VALUES(22,'Bill');
INSERT INTO tags VALUES(23,'Recieved');
INSERT INTO tags VALUES(24,'Sent');

CREATE TABLE doc_tags (
	doctagid INTEGER PRIMARY KEY,
	docid NUMERIC,
	tagid NUMERIC);

CREATE INDEX doc_tags_tagid_idx
ON doc_tags(tagid ASC);

COMMIT;

