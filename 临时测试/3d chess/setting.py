##setting.py

#Dependencies
from typing import Final as type_const
from typing import Dict  as type_dict
from typing import List  as type_list
from typing import Any   as type_any

#Tags
CONST:type_const[str] = 'CONST'
FUNK :type_const[str] = 'FUNC'
EXPR :type_const[str] = 'EXPR'
EXEC :type_const[str] = 'EXEC'
FLOAT:type_const[str] = 'FLOAT'
INT  :type_const[str] = 'INT'
STR  :type_const[str] = 'STR'
SPACE:type_const[str] = ' '
VOID :type_const[str] = '--'

#Functions
def readsettings(name:str = 'default') -> type_dict[str,type_any]:
    values:type_dict[str,type_any] = {}
    with open(r'%s.setting.txt' % name,'r') as file:
        line:str = file.readline()
        while line:
            info:type_list[str] = line[:-1:].split(SPACE)
            if len(info) > 0:
                if info[0][:2:] == VOID:
                    pass
                elif CONST == info[0]:
                    if FLOAT == info[1]:
                        values[info[2]] = float(info[4])
                    elif INT == info[1]:
                        values[info[2]] = int(info[4])
                    elif STR == info[1]:
                        values[info[2]] = str(info[4])
                elif FUNK == info[0]:
                    values[info[1]] = str(info[2])
                elif EXPR == info[0]:
                    values[info[1]] = eval(info[2])
                elif EXEC == info[0]:
                    exec(info[1])
            line:str = file.readline()
    return values

if __name__ == '__main__':
    readsettings()
