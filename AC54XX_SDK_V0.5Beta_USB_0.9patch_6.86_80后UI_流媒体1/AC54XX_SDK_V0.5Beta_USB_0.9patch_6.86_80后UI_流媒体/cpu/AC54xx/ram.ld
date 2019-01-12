SDRAM_SIZE = 32M;
MEMORY
{
    xrom(rx)          : ORIGIN =  0x2000000, LENGTH = 4M
	sdram_mem(rwx)    : ORIGIN =  0x4000000, LENGTH = SDRAM_SIZE
	ram0(rwx)         : ORIGIN =  0x3f00000, LENGTH = 0x3f0fc00-0x3f00000
	/* ram2(rwx)         : ORIGIN =  0x3f08000, LENGTH = 0x3f0fc00-0x3f08000 */
	vram(rwx)         : ORIGIN =  0x3fa0000, LENGTH = 0x3fc8800-0x3fa0000
}

SECTIONS
{
/********************************************/

    . = ORIGIN(sdram_mem);

    .rom_code ALIGN(4):
    {
        _text_rodata_begin = .;
        PROVIDE(text_rodata_begin = .);

        *startup.o(.text)

        *sfc_boot.o(.text*)
        *sfc_boot.o(.rodata*)
        *boot_main.o(.text*)
        *boot_main.o(.rodata*)

        *(.text*)
        *(.rodata*)

        _text_rodata_end = .;
        PROVIDE(text_rodata_end = .);

        . = ALIGN(32);
        *(.data)

		. = ALIGN(4);
		vfs_ops_begin = .;
			*(.vfs_operations)
		vfs_ops_end = .;

		_lcd_device_begin = .;
		PROVIDE(lcd_device_begin = .);
			*(.lcd_device)
		_lcd_device_end = .;
		PROVIDE(lcd_device_end = .);

		_lcd_device_drive_begin = .;
		PROVIDE(lcd_device_drive_begin = .);
			*(.lcd_device_drive)
		_lcd_device_drive_end = .;
		PROVIDE(lcd_device_drive_end = .);

        _gsensor_dev_begin = .;
		PROVIDE(gsensor_dev_begin = .);
			*(.gsensor_dev)
		_gsensor_dev_end = .;
		PROVIDE(gsensor_dev_end = .);

        _touch_panel_dev_begin = .;
        PROVIDE(touch_panel_dev_begin = .);
        *(.touch_panel_dev)
            _touch_panel_dev_end = .;
        PROVIDE(touch_panel_dev_end = .);

		_isp_scr_begin = .;
		PROVIDE(isp_scr_begin = .);
			*(.isp_scr_work)
		_isp_scr_end = .;
		PROVIDE(isp_scr_end = .);

		_power_dev_begin = .;
		PROVIDE(power_dev_begin = .);
			*(.power_dev)
		_power_dev_end = .;
		PROVIDE(power_dev_end = .);

		_eth_phy_device_begin = .;
		PROVIDE(eth_phy_device_begin = .);
			*(.eth_phy_device)
		_eth_phy_device_end = .;
		PROVIDE(eth_phy_device_end = .);

		INCLUDE include_lib/ui/ui/ui.ld
		INCLUDE include_lib/system/system.ld


        . = ALIGN(32);
		INCLUDE include_lib/system/cpu/AC54xx/device.ld
		. = ALIGN(32);
		INCLUDE include_lib/system/device/device.ld
		INCLUDE include_lib/system/device/device_data.ld
		. = ALIGN(32);
		INCLUDE include_lib/system/system_data.ld

		INCLUDE include_lib/server/server.ld


		. = (( . + 31) / 32 * 32);
		_layer_buf_begin = .;
		PROVIDE(layer_buf_begin = .);
		*(.layer_buf_t)
		_layer_buf_end = .;
		PROVIDE(layer_buf_end = .);

		. = (( . + 31) / 32 * 32);
		_font_info_begin = .;
		PROVIDE(font_info_begin = .);
		*(.font_info_t)
		_font_info_end = .;
		PROVIDE(font_info_end = .);

    } >sdram_mem

	 .data ALIGN(4):
	  {
	  } > sdram_mem

    .bss ALIGN(32) :
    {
		*(.stack)
		_system_data_begin = .;
        *(.bss)
        *(COMMON)
		*(.usb_slave_var)
        *(.mem_heap)
		. = (( . + 31) / 32 * 32);
		_lcd_dev_buf_addr = .;
		PROVIDE(lcd_dev_buf_addr = .);
		*(.lcd_dev_buf)
		_system_data_end = .;

    } > sdram_mem
    _HEAP_BEGIN = ( . +31 )/ 32 *32 ;
    PROVIDE(HEAP_BEGIN = (. + 31) / 32 *32);
   _HEAP_END = 0x4000000 + SDRAM_SIZE - 32;
   PROVIDE(HEAP_END = 0x4000000 + SDRAM_SIZE - 32);
   _MALLOC_SIZE = _HEAP_END - _HEAP_BEGIN;
   PROVIDE(MALLOC_SIZE = _HEAP_END - _HEAP_BEGIN);

	. = 0x3f08000 ;
	.sram ALIGN(4):
	{
        *(.boot)
	}


/********************************************/
    . =ORIGIN(ram0);
    .ram0_data ALIGN(4):
    {
        _VM_CODE_START = . ;
        *(.vm)
        _VM_CODE_END = . ;
        *(.flushinv_icache)
    } > ram0

    .ram0_bss ALIGN(4):
    {
        /*
         * sd卡全局变量要放sram，否则会读写出错
         */
        *(.sd_var)
		*(.sram)
        *(.usb0_fifo)
    } > ram0

/********************************************/
    /* . =ORIGIN(ram2); */
    /* .ram2_data ALIGN(4): */
    /* { */
    /* } > ram2 */
    /* .ram2_bss ALIGN(4): */
    /* { */
    /* } > ram2 */
/********************************************/
    . =ORIGIN(xrom);
    .xrom  0x2000000 :
    {
    } >xrom
/********************************************/



    text_begin = ADDR(.rom_code ) ;
    text_size = SIZEOF(.rom_code ) ;
    bss_begin = ADDR(.bss ) ;
    bss_size  = SIZEOF(.bss);
    bss_size1 = _system_data_end - _system_data_begin;

    bss_begin1 = _system_data_begin;

    data_begin = LOADADDR(.data)  ;
    data_addr  = ADDR(.data) ;
    data_size =  SIZEOF(.data) ;


/********************************************/
    _ram0_bss_vma = ADDR(.ram0_bss);
    _ram0_bss_size  =  SIZEOF(.ram0_bss);
    _ram0_data_vma  = ADDR(.ram0_data);
    _ram0_data_lma =  text_begin + SIZEOF(.rom_code);
    _ram0_data_size =  SIZEOF(.ram0_data);

    /* _ram2_bss_vma = ADDR(.ram2_bss); */
    /* _ram2_bss_size  =  SIZEOF(.ram2_bss); */
    /* _ram2_data_vma  = ADDR(.ram2_data); */
    /* _ram2_data_lma =  text_begin + SIZEOF(.rom_code)+SIZEOF(.ram0_data); */
    /* _ram2_data_size =  SIZEOF(.ram2_data); */

/********************************************/
}

