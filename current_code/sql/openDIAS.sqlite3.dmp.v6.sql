BEGIN TRANSACTION;

CREATE TEMPORARY TABLE t1_backup(a);
INSERT INTO t1_backup SELECT version FROM version;
DROP TABLE version;
CREATE TABLE version (version INTEGER PRIMARY KEY);
INSERT INTO version SELECT a FROM t1_backup;
DROP TABLE t1_backup;

UPDATE version SET version = 6;

COMMIT;

