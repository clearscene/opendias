BEGIN TRANSACTION;

ALTER TABLE scan_progress 
ADD COLUMN lockexpires;

ALTER TABLE docs
ADD COLUMN actionrequired NUMERIC;

UPDATE version
SET version = 5,
for_app_version = "0.6.4";

COMMIT;

