sudo hcitool cmd 0x3F 0x01C 0x01 0x02 0x00 0x01 0x01

pacmd set-card-profile bluez_card.00_28_36_11_13_12 headset_head_unit
pacmd set-default-sink bluez_sink.00_28_36_11_13_12.headset_head_unit
pacmd set-default-source bluez_source.00_28_36_11_13_12.headset_head_unit
