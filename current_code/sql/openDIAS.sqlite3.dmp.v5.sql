BEGIN TRANSACTION;

UPDATE scan_progress 
ADD COLUMN lockexpires

UPDATE version
SET version = 5,
for_app_version = "0.6.4";

COMMIT;

