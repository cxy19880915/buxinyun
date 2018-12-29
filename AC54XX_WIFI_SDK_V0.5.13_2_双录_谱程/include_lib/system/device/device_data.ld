
	_video_subdev_begin = .;
	PROVIDE(video_subdev_begin = .);
	 	*(.video_subdev.0)
	 	*(.video_subdev.1)
	 	*(.video_subdev.2)
	 	*(.video_subdev.3)
	 	*(.video_subdev.4)
	 	*(.video_subdev.5)
	_video_subdev_end = .;
	PROVIDE(video_subdev_end = .);

	_audio_subdev_begin = .;
	PROVIDE(audio_subdev_begin = .);
	 	*(.audio_subdev.0)
	 	*(.audio_subdev.1)
	 	*(.audio_subdev.2)
	 	*(.audio_subdev.3)
	_audio_subdev_end = .;
	PROVIDE(audio_subdev_end = .);

	_iic_device_begin = .;
	PROVIDE(iic_device_begin = .);
	 	*(.iic)
	_iic_device_end = .;
	PROVIDE(iic_device_end = .);

	_uart_device_begin = .;
	PROVIDE(uart_device_begin = .);
	 	*(.uart)
	_uart_device_end = .;
	PROVIDE(uart_device_end = .);
	_gps_device_begin = .;
	PROVIDE(gps_device_begin = .);
	    *(.gps)
	_gps_device_end = .;
	PROVIDE(gps_device_end = .);

	_pwm_device_begin = .;
	PROVIDE(pwm_device_begin = .);
	    *(.pwm)
	_pwm_device_end = .;
	PROVIDE(pwm_device_end = .);

		_eth_phy_device_begin = .;
		PROVIDE(eth_phy_device_begin = .);
			*(.eth_phy_device)
		_eth_phy_device_end = .;
		PROVIDE(eth_phy_device_end = .);

	_rf_phy_device_begin = .;
		PROVIDE(rf_phy_device_begin = .);
			*(.rf_phy_device)
		_rf_phy_device_end = .;
		PROVIDE(rf_phy_device_end = .);

	_ctp_mapping_tab_begin = .;
	PROVIDE(ctp_mapping_tab_begin = .);

	_ctp_mapping_tab_system_cmd_begin = .;
	PROVIDE(ctp_mapping_tab_system_cmd_begin = .);
	 	*(.ctp_system_cmd)
	_ctp_mapping_tab_system_cmd_end = .;
	PROVIDE(ctp_mapping_tab_system_cmd_end = .);

		_ctp_mapping_tab_video_cmd_begin = .;
	PROVIDE(ctp_mapping_tab_video_cmd_begin = .);
	 	*(.ctp_video_cmd)
	_ctp_mapping_tab_video_cmd_end = .;
	PROVIDE(ctp_mapping_tab_video_cmd_end = .);


	_ctp_mapping_tab_photo_cmd_begin = .;
	PROVIDE(ctp_mapping_tab_photo_cmd_begin = .);
	 	*(.ctp_photo_cmd)
	_ctp_mapping_tab_photo_cmd_end = .;
	PROVIDE(ctp_mapping_tab_photo_cmd_end = .);



	_ctp_mapping_tab_end = .;
	PROVIDE(ctp_mapping_tab_end = .);

	_sig_mapping_tab_cmd_begin = .;
	PROVIDE(sig_mapping_tab_cmd_begin = .);
	 	*(.sig_cmd)
	_sig_mapping_tab_cmd_end = .;
	PROVIDE(sig_mapping_tab_cmd_end = .);

	_net_video_stream_sub_begin = .;
	PROVIDE(net_video_stream_sub_begin = .);
	 	*(.net_video_stream)
	_net_video_stream_sub_end = .;
	PROVIDE(net_video_stream_sub_end = .);



    _avin_spi_device_begin = .;
	PROVIDE(avin_spi_device_begin = .);
		*(.sw_spi)
	_avin_spi_device_end = .;
	PROVIDE(avin_spi_device_end = .);
