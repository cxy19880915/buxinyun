
		_camera_dev_begin = .;
		PROVIDE(camera_dev_begin = .);
		*(.camera_dev)
		_camera_dev_end = .;
		PROVIDE(camera_dev_end = .);

		_adc_scan_begin = .;
		PROVIDE(adc_scan_begin = .);
		*(.adc_scan)
		_adc_scan_end = .;
		PROVIDE(adc_scan_end = .);

		_spi_device_begin = .;
		PROVIDE(spi_device_begin = .);
		 *(.spi_device)
		_spi_device_end = .;
		PROVIDE(spi_device_end = .);
