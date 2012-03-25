BEGIN TRANSACTION;

CREATE TEMPORARY TABLE t1_backup(a);
INSERT INTO t1_backup SELECT version FROM version;
DROP TABLE version;
CREATE TABLE version (version INTEGER PRIMARY KEY);
INSERT INTO version SELECT a FROM t1_backup;
DROP TABLE t1_backup;

UPDATE version SET version = 6;

CREATE TABLE doc_links (
        doclinkid INTEGER PRIMARY KEY,
        docid NUMERIC,
        linkeddocid NUMERIC);

CREATE INDEX doc_links_linkid_idx
ON doc_links(docid ASC);


CREATE TEMPORARY TABLE t1_backup("status" INTEGER, "value" INTEGER, "client_id" CHAR(36));
INSERT INTO t1_backup SELECT status, value, client_id FROM scan_progress;
DROP TABLE scan_progress;
CREATE TABLE scan_progress (
    "status" INTEGER NOT NULL DEFAULT (0),
    "value" INTEGER,
    "client_id" CHAR(36) NOT NULL DEFAULT (' ')
);
INSERT INTO scan_progress SELECT status, value, client_id FROM t1_backup;
CREATE UNIQUE INDEX "client_id" on "scan_progress" (client_id ASC);
DROP TABLE t1_backup;


COMMIT;

