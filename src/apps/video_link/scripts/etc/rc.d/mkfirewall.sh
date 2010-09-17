
if [[ -d ~/bin ]]; then false; else mkdir ~/bin; fi

MYIP="$(~/bin/getgwip)"
TOIP="$(ipof $1)"
DATACORE="$(ipof $DATABASE_HOST)"
DATACORE_GW="$(~/bin/ipon eth1)"

echo "my:$MYIP to:$TOIP"

case $2 in 
   $(hostname))
       ROUTETO="" #"iptables -t nat -A POSTROUTING -s $DATACORE/32 -p tcp --dport 3306 -j SNAT --to-source $MYIP"
       ROUTETO2="" #"iptables -t nat -A POSTROUTING -p tcp --dport 3306 -j SNAT --to-source $DATACORE_GW"
       ROUTEFROM="" #"iptables -t nat -A PREROUTING -p tcp --dport 3306 -j DNAT --to-destination $DATACORE"
       ;;
   *)
       ROUTETO="iptables -t nat -A POSTROUTING -p tcp --dport 3306 -j SNAT --to-source $MYIP"
       ROUTETO2=""
       ROUTEFROM="iptables -t nat -A PREROUTING -p tcp --dport 3306 -j DNAT --to-destination $TOIP"
       ;;
esac 

cd ~/bin

echo "
modprobe iptable_filter
modprobe iptable_nat
modprobe xt_tcpudp
echo 1 >/proc/sys/net/ipv4/ip_forward
#ifconfig eth0 add 192.168.1.1
iptables -t nat --flush
# -p port
# 

$ROUTETO
$ROUTETO2
$ROUTEFROM
#iptables -t nat -A POSTROUTING -p tcp --dport 3306 -j SNAT --to-source $MYIP
#iptables -t nat -A PREROUTING -p tcp --dport 3306 -j DNAT --to-destination $(ipof $1)

#iptables -t nat -A PREROUTING -p tcp --dport 8000 -i eth1 -j DNAT --to-destination $1
#iptables -t nat -A PREROUTING -p tcp --dport 8001 -i eth1 -j DNAT --to-destination $1
#iptables -t nat -A PREROUTING -p tcp --dport 8767 -i eth1 -j DNAT --to-destination $1
" > firewall.sh
chmod 755 ./firewall.sh
./firewall.sh
