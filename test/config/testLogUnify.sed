s/\w\{8\}-\w\{4\}-\w\{4\}-\w\{4\}-\w\{12\}/[ZZZZZZZZ-UUID-ZZZZ-ZZZZ-ZZZZZZZZZZZZ]/g
s/Admin User','[0-9]\{4\}-[0-9]\{2\}-[0-9]\{2\} [0-9]\{2\}:[0-9]\{2\}:[0-9]\{2\}','automatically/Admin User','[YYYY-MM-DD HH:MM:SS]','automatically/g
s/[0-9]\{4\}-[0-9]\{2\}-[0-9]\{2\} [0-9]\{2\}:[0-9]\{2\}:[0-9]\{2\}/[YYYY-MM-DD HH:MM:SS]/g
