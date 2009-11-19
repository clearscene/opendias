s/==[0-9]*== /==XXXXX== /g
s/My PID = [0-9]*, parent PID = [0-9]*./My PID = XXXXX, parent PID = XXXXX./
s/\([0-9]\),\([0-9]\)/\1\2/g
s/[0-9]*[0-9] bytes/XXX bytes/g
s/[0-9]*[0-9] blocks/XXX blocks/g
s/[0-9]*[0-9] frees/XXX frees/g
s/[0-9]*[0-9] allocs/XXX allocs/g
s/pointers to [0-9]* not-freed/pointers to XXXXX not-freed/g
