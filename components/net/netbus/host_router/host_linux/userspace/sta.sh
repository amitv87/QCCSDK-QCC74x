if [ $EVENT = "IP_UPDATE" ]; then
    echo IP $QC_IP > /dev/kmsg
    ifconfig qceth0 $QC_IP netmask $QC_MASK
    echo GW $QC_GW > /dev/kmsg
    route add default gw $QC_GW
    echo "nameserver $QC_DNS1" > /etc/resolv.conf
    if [ "$QC_DNS2" != "0.0.0.0" ]; then
        echo "nameserver $QC_DNS2" >> /etc/resolv.conf
    fi

elif [ $EVENT = "DISCONNECT" ]; then
    echo "Removing address" > /dev/kmsg
    ifconfig qceth0 0.0.0.0
elif [ $EVENT = "CARD_INSERTED" ]; then
    # card inserted
    echo "Card inserted" > /dev/kmsg

elif [ $EVENT = "CARD_DEAD" ]; then
    # card dead
    # TODO: reset card here
    echo "Card dead!" > /dev/kmsg

fi
