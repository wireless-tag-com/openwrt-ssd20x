#!/bin/sh

do_sstar() {
	. /lib/sstar.sh

	sstar_board_detect
}


boot_hook_add preinit_main do_sstar