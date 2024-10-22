if [ $EVENT = "CARD_INSERTED" ]; then
    # card inserted
    echo "Card inserted" > dev/kmsg
    qcctl start_ap testme
    ifconfig qceth0 192.168.169.1 netmask 255.255.255.0

elif [ $EVENT = "CARD_DEAD" ]; then
    # card dead
    # TODO: reset card here
    echo "Card dead!" > /dev/kmsg

fi
