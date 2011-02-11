s/==[0-9]*==/==XXXXX== /g
s/My PID = [0-9]*, parent PID = [0-9]*./My PID = XXXXX, parent PID = XXXXX./
s/Parent PID: [0-9]*./Parent PID: XXXXX./
s/\([0-9]\),\([0-9]\)/\1\2/g
s/[0-9]*[0-9] bytes/XXX bytes/g
s/[0-9]*[0-9] blocks/XXX blocks/g
s/[0-9]*[0-9] allocs/XXXX allocs/g
s/[0-9]*[0-9] frees/XXXX frees/g
s/suppressed: [0-9]* from [0-9]*/suppressed: XXX from XXX/g
