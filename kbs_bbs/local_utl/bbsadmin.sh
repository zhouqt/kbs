#!/bin/sh
#
#       define the standout() and standend() escape sequences...
#
#SO="^[[7m"
#SE="^[[0m"
#stty pass8

#biff n
cd /home/bbs
while true
do
clear
echo "---------------------------------------------"
echo "         用于BBS系统管理	"
echo "---------------------------------------------"
echo "
   [1] 转接BBS(请避免在重起之后用本菜单马上连接BBS)
   [2] 注册断线后恢复注册单
   [3] 停止登陆系统
   [4] 重新生成热门话题 
   [Q] 退出

"
echo  "请选择 [Q]: "

        if read CHOICE
           then
            clear
            case "${CHOICE}"
              in
                '')
                  break
                  ;;
                'Q')
                  break
                  ;;
                'q')
                  break
                  ;;
                1)
		  telnet -E localhost 8002
		  break
                  ;;
                2)
                  echo " 将new_register.tmp加入new_register"
		  cat new_register.tmp >> new_register
                  echo "删除文件new_register.tmp"
		  rm -f new_register.tmp
                  ;;
		3)
		  echo "输入停止登陆提示，按Ctrl+D结束"
		  cat > /home/bbs/NOLOGIN
		  ;;
		4)
		  /home/bbs/bin/poststat /home/bbs
		  /home/bbs/bin/htmltop10 /home/bbs
		  ;;
                *)
                  echo ""
                  echo "错误的选项"

                  ;;
            esac
          else
            exit
        fi
done

clear

