BEGIN TRANSACTION;
update config set config_value=4 where config_option='log_verbosity';
COMMIT;
