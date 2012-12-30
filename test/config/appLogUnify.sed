s/[0-9]*:[A-F0-9][A-F0-9][A-F0-9][A-F0-9][A-F0-9][A-F0-9][A-F0-9]:/-date-:-thread-:/g
s/[0-9]*:[A-F0-9][A-F0-9][A-F0-9][A-F0-9][A-F0-9][A-F0-9][A-F0-9][A-F0-9]:/-date-:-thread-:/g
s/:[a-zA-Z0-9_]*.c:[0-9]* /:-file-:-line- /g
s/Child process created [0-9]*/Child process created [CONFIG_FILE]/g
s/Using config file: .*/Using config file: [CONFIG_FILE]/g
s/t open database: .*/t open database: [ERROR]/g
s/Run Query .[0-9A-Fa-f]*./Run Query [RECORDSET]/g
s/Free recordset .[0-9A-Fa-f]*./Free recordset [RECORDSET]/g
s/file: .*\/test\/config/file: [PATH]\/test\/config/g
s/\w\{8\}-\w\{4\}-\w\{4\}-\w\{4\}-\w\{12\}/[ZZZZZZZZ-UUID-ZZZZ-ZZZZ-ZZZZZZZZZZZZ]/g
s/[0-9][0-9][0-9][0-9]-[0-9][0-9]-[0-9][0-9] [+-][0-9][0-9][0-9][0-9] [0-9][0-9]:[0-9][0-9]:[0-9][0-9]/YYYY-MM-DD +-UTC HH:MM:SS/g

