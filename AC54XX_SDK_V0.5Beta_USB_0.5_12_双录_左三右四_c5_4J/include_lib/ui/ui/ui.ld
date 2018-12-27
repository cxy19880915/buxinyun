
		_ui_style_begin = .;
		PROVIDE(ui_style_begin = .);
		   *(.ui_style)
		_ui_style_end = .;
		PROVIDE(ui_style_end = .);

        _elm_event_handler_begin_UPGRADE = .;
		PROVIDE(elm_event_handler_begin_UPGRADE = .);
           *(.elm_event_handler_UPGRADE)
		_elm_event_handler_end_UPGRADE = .;
		PROVIDE(elm_event_handler_end_UPGRADE = .);

                _elm_event_handler_begin_JL02 = .;
		PROVIDE(elm_event_handler_begin_JL02 = .);
           *(.elm_event_handler_JL02)
		_elm_event_handler_end_JL02 = .;
		PROVIDE(elm_event_handler_end_JL02 = .);

		_elm_event_handler_begin_ANI = .;
		PROVIDE(elm_event_handler_begin_ANI = .);
           *(.elm_event_handler_ANI)
		_elm_event_handler_end_ANI = .;
		PROVIDE(elm_event_handler_end_ANI = .);

		_elm_event_handler_begin_LY = .;
		PROVIDE(elm_event_handler_begin_LY = .);
           *(.elm_event_handler_LY)
		_elm_event_handler_end_LY = .;
		PROVIDE(elm_event_handler_end_LY = .);

		_control_event_handler_begin = .;
		PROVIDE(control_event_handler_begin = .);
           *(.control_event_handler)
		_control_event_handler_end = .;
		PROVIDE(control_event_handler_end = .);

		_control_ops_begin = .;
		PROVIDE(control_ops_begin = .);
           *(.control_ops)
		_control_ops_end = .;
		PROVIDE(control_ops_end = .);

		_vg_event_handler_begin = .;
		PROVIDE(vg_event_handler_begin = .);
           *(.vg_event_handler)
        _vg_event_handler_end = .;
		PROVIDE(vg_event_handler_end = .);

        _on_show_map_begin = .;
		PROVIDE(on_show_map_begin = .);
           *(.on_show_map)
        _on_show_map_end = .;
		PROVIDE(on_show_map_end = .);

        _on_click_map_begin = .;
		PROVIDE(on_click_map_begin = .);
           *(.on_click_map)
        _on_click_map_end = .;
		PROVIDE(on_click_map_end = .);

        _on_touch_map_begin = .;
		PROVIDE(on_touch_map_begin = .);
           *(.on_touch_map)
        _on_touch_map_end = .;
		PROVIDE(on_touch_map_end = .);

        _on_change_map_begin = .;
		PROVIDE(on_change_map_begin = .);
           *(.on_change_map)
        _on_change_map_end = .;
		PROVIDE(on_change_map_end = .);

        _on_key_map_begin = .;
		PROVIDE(on_key_map_begin = .);
           *(.on_key_map)
        _on_key_map_end = .;
		PROVIDE(on_key_map_end = .);

		_img_loader_begin = .;
		PROVIDE(img_loader_begin = .);
		   *(.img_loader)
		_img_loader_end = .;
		PROVIDE(img_loader_end = .);

		_battery_notify_begin = .;
		PROVIDE(battery_notify_begin = .);
           *(.battery_notify)
        _battery_notify_end = .;
		PROVIDE(battery_notify_end = .);




