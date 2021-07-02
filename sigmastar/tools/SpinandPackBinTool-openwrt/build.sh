#!/bin/sh

case "$1" in
"FS35ND04G")
	cp -f wt/logo/$2 images/logo
	cp -f wt/partinfo/PARTINFO-4G-wt.pni images/boot/PARTINFO.pni
	./SstarMakeBin -n SPINAND_FS35ND04G.ini
	;;
"FS35ND02G")
	cp -f wt/logo/$2 images/logo
	cp -f wt/partinfo/PARTINFO-2G-wt.pni images/boot/PARTINFO.pni
	./SstarMakeBin -n SPINAND_FS35ND02G.ini
	;;
"FS35ND01G")
	cp -f wt/logo/$2 images/logo
	cp -f wt/partinfo/PARTINFO-wt.pni images/boot/PARTINFO.pni
	./SstarMakeBin -n SPINAND_FS35ND01G.ini
	;;
"DOSILOCON")
	cp -f wt/logo/$2 images/logo
	cp -f wt/partinfo/PARTINFO-wt.pni images/boot/PARTINFO.pni
	./SstarMakeBin -n SPINAND_DOSILOCON.ini
	;;
"WINBOND")
	cp -f wt/logo/$2 images/logo
	cp -f wt/partinfo/PARTINFO-wt.pni images/boot/PARTINFO.pni
	./SstarMakeBin -n SPINAND_WINBOND.ini
	;;
esac

