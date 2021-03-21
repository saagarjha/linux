#include <asm-generic/vmlinux.lds.h>
#include <asm/page.h>
#include <asm/cache.h>
#include <asm/thread_info.h>

jiffies = jiffies_64;

SECTIONS
{
	. = SIZEOF_HEADERS;

	_text = .;
	__init_begin = .;
	HEAD_TEXT_SECTION
	INIT_TEXT_SECTION(PAGE_SIZE)
	INIT_DATA_SECTION(16)
	PERCPU_SECTION(L1_CACHE_BYTES)
	__init_end = .;

	.rel.plt : {
		*(.rel.plt)
		PROVIDE_HIDDEN(__rel_iplt_start = .);
		*(.rel.iplt)
		PROVIDE_HIDDEN(__rel_iplt_end = .);
	}
	.rela.plt : {
		*(.rela.plt)
		PROVIDE_HIDDEN(__rela_iplt_start = .);
		*(.rela.iplt)
		PROVIDE_HIDDEN(__rela_iplt_end = .);
	}
	_stext = .;
	.text : {
		TEXT_TEXT
		SCHED_TEXT
		CPUIDLE_TEXT
		LOCK_TEXT
	}
	_etext = .;

	_sdata = .;
	RO_DATA(PAGE_SIZE)
	RW_DATA(16, PAGE_SIZE, THREAD_SIZE)
	_edata = .;

	EXCEPTION_TABLE(16)

	.init_array : {
		__init_array_start = .;
		*(.init_array)
		__init_array_end = .;
	}

	BSS_SECTION(0, 0, 0)
	_end = .;

	STABS_DEBUG
	DWARF_DEBUG

	DISCARDS
}
