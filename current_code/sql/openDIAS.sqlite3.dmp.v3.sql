BEGIN TRANSACTION;

CREATE TABLE "scan_params" (
    "client_id" TEXT NOT NULL DEFAULT (0),
    "param_option" INTEGER NOT NULL DEFAULT (0),
    "param_value" TEXT
);

CREATE UNIQUE INDEX "primary" on scan_params (client_id ASC, param_option ASC);

CREATE TABLE scan_progress (
    "status" INTEGER NOT NULL DEFAULT (0),
    "value" INTEGER,
    "client_id" CHAR(36) NOT NULL DEFAULT (' ')
);

CREATE UNIQUE INDEX "client_id" on "scan_progress" (client_id ASC);

CREATE TABLE "config" (
    "config_option" TEXT NOT NULL,
    "config_value" TEXT NOT NULL
);

CREATE UNIQUE INDEX "option" on config (config_option ASC);

UPDATE version
SET version = 3,
for_app_version = "0.5";

COMMIT;

