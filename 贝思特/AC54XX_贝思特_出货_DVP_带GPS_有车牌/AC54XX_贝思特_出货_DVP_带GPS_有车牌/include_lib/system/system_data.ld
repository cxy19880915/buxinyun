
	_key_driver_begin = .;
	PROVIDE(key_driver_begin = .);
	   *(.key_driver)
	_key_driver_end = .;
	PROVIDE(key_driver_end = .);

	_touch_driver_begin = .;
	PROVIDE(touch_driver_begin = .);
	   *(.touch_driver)
	_touch_driver_end = .;
	PROVIDE(touch_driver_end = .);

	_static_hi_timer_begin = .;
	PROVIDE(static_hi_timer_begin = .);
	   *(.hi_timer)
	_static_hi_timer_end = .;
	PROVIDE(static_hi_timer_end = .);

	_sys_cpu_timer_begin = .;
	PROVIDE(sys_cpu_timer_begin = .);
	   *(.sys_cpu_timer)
	_sys_cpu_timer_end = .;
	PROVIDE(sys_cpu_timer_end = .);

    _sys_config_begin = .;
    PROVIDE(sys_config_begin = .);
        *(.sys_cfg)
    _sys_config_end = .;
    PROVIDE(sys_config_end = .);

