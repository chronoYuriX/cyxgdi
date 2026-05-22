##START GAME.py
import chessboard
import chars
import setting

settings = setting.readsettings('config')
chessboard_width = settings['width' ]
chessboard_lenth = settings['length']
chessboard_river = settings['river' ]
chessboard_void  = settings['void'  ]
check_stumbling_horse  = settings['check_stumbling_horse' ]
check_stumbling_vizier = settings['check_stumbling_vizier']
players = settings['players']
gamemode = settings['mode']
player_name = (chars.XiangYu,chars.LiuBang)
admirals = chars.admirals

print(chars.helping)
c = chessboard.chessboard(chessboard_width,\
                          chessboard_lenth,\
                          chessboard_river,\
                          chessboard_void,\
                          check_stumbling_horse,\
                          check_stumbling_vizier,\
                          players,\
                          admirals,\
                          player_name)
if gamemode == 'pvp':
    c.summon_newchessboard()
c.draw()
c.renderer.GUI.canvas.mainloop()
