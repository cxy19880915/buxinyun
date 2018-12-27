   _device_node_begin = .;
   PROVIDE(device_node_begin = .);
        *(.device)
   _device_node_end = .;
   PROVIDE(device_node_end = .);
