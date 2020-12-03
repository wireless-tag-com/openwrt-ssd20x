SECTIONS
{
  .FCIE.module.text : { *(.text) }
  .FCIE.module.rodata : {
    *(.rodata) *(.rodata.*)
    *(__vermagic) *(__tracepoints_ptrs) *(__markers_strings) *(__tracepoints_strings)
    *(.rodata1) *(.rio_switch_ops)
    *(SORT(___ksymtab+*)) *(SORT(___ksymtab_gpl+*)) *(SORT(___ksymtab_unused+*)) *(SORT(___ksymtab_unused_gpl+*)) *(SORT(___ksymtab_gpl_future+*))
    *(SORT(___kcrctab+*)) *(SORT(___kcrctab_gpl+*)) *(SORT(___kcrctab_unused+*)) *(SORT(___kcrctab_unused_gpl+*)) *(SORT(___kcrctab_gpl_future+*))
    *(__ksymtab_strings)
    *(.ref.rodata)
    *(.devinit.rodata) *(.devexit.rodata)
    *(.cpuinit.rodata) *(.cpuexit.rodata)
    *(__param)
    *(__modver)
  }
}
