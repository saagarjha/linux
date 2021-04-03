# Darwin doesn't do linker scripts. This file is the closest you get.

# Sections appear in the order they show up an object files. Here we have empty
# sections in the desired order, so if this object is linked first they will
# appear in that order.
.macro sect S,s
	.section __\S,__\s
	_\@:
.endm

# Define variables pointing to the boundaries of sections by defining aliases
# to magic symbols.
.macro alias sym, old
	.globl \sym
	.set \sym, \old
.endm
.macro s_sect S,s, sym
	alias _\sym, section$start$__\S\()$__\s
.endm
.macro e_sect S,s, sym
	alias _\sym, section$end$__\S\()$__\s
.endm
.macro s_seg S, sym
	alias _\sym, segment$start$__\S
.endm
.macro e_seg S, sym
	alias _\sym, segment$end$__\S
.endm

.macro INIT_CALLS_LEVEL level
	s_sect	DATA,initcall\level,	__initcall\level\()_start
	sect	DATA,initcall\level
	sect	DATA,initcall\level\()s
.endm

.macro INIT_DATA_SECTION
	s_sect	DATA,init,		__init_begin

	# INIT_DATA
	sect	DATA,init
	s_sect	DATA,ftrace_events,	__start_ftrace_events
	sect	DATA,ftrace_events
	e_sect	DATA,ftrace_events,	__stop_ftrace_events
	s_sect	DATA,ftrce_eval_map,	__start_ftrace_eval_maps
	sect	DATA,ftrce_eval_map
	e_sect	DATA,ftrce_eval_map,	__stop_ftrace_eval_maps

	# INIT_SETUP
	s_sect	DATA,init_setup,	__setup_start
	sect	DATA,init_setup
	e_sect	DATA,init_setup,	__setup_end

	# INIT_CALLS
	s_sect	DATA,initcallearly,	__initcall_start
	sect	DATA,initcallearly
	INIT_CALLS_LEVEL 0
	INIT_CALLS_LEVEL 1
	INIT_CALLS_LEVEL 2
	INIT_CALLS_LEVEL 3
	INIT_CALLS_LEVEL 4
	INIT_CALLS_LEVEL 5
	s_sect	DATA,initcallrootfs,	__initcallrootfs_start
	sect	DATA,initcallrootfs
	INIT_CALLS_LEVEL 6
	INIT_CALLS_LEVEL 7
	e_sect	DATA,initcall7s,	__initcall_end

	# CON_INITCALL
	s_sect	DATA,con_initcall,	__con_initcall_start
	sect	DATA,con_initcall
	e_sect	DATA,con_initcall,	__con_initcall_end

	# INIT_RAM_FS
	s_sect	DATA,init_ramfs,	__initramfs_start
	sect	DATA,init_ramfs

	# PERCPU_SECTION
	s_sect	DATA,percpu_first,	__per_cpu_load
	s_sect	DATA,percpu_first,	__per_cpu_start
	sect	DATA,percpu_first
	sect	DATA,percpu
	sect	DATA,percpu_shr_aln
	e_sect	DATA,percpu_shr_aln,	__per_cpu_end

	e_sect	DATA,percpu_shr_aln,	__init_end
.endm

.macro TEXT_SECTION
	s_seg	TEXT,		text
	s_seg	TEXT,		_stext

	sect	TEXT,text
	s_sect	TEXT,init,	_sinittext
	sect	TEXT,init
	e_sect	TEXT,init,	_einittext

	s_sect	TEXT,sched,	__sched_text_start
	sect	TEXT,sched
	e_sect	TEXT,sched,	__sched_text_end
	s_sect	TEXT,cpuidle,	__cpuidle_text_start
	sect	TEXT,cpuidle
	e_sect	TEXT,cpuidle,	__cpuidle_text_end
	s_sect	TEXT,spinlock,	__lock_text_start
	sect	TEXT,spinlock
	e_sect	TEXT,spinlock,	__lock_text_end
	s_sect	TEXT,softirqentry, __softirqentry_text_start
	sect	TEXT,softirqentry
	e_sect	TEXT,softirqentry, __softirqentry_text_end

	e_sect	TEXT,softirqentry, _etext
.endm

.macro RO_DATA_SECTION
	s_sect	DATA,const,	__start_rodata
	sect	DATA,const
	sect	DATA,const_init

	s_sect	DATA,tracepts_ptrs, __start___tracepoints_ptrs
	sect	DATA,tracepts_ptrs
	e_sect	DATA,tracepts_ptrs, __stop___tracepoints_ptrs
	s_sect	DATA,const_param, __start___param
	sect	DATA,const_param
	e_sect	DATA,const_param, __stop___param
	s_sect	DATA,const_modver,	__start___modver
	sect	DATA,const_modver
	e_sect	DATA,const_modver,	__stop___modver
	s_sect	DATA,const_ex_table,	__start___ex_table
	sect	DATA,const_ex_table
	e_sect	DATA,const_ex_table,	__stop___ex_table
	s_sect	DATA,const_notes,	__start_notes
	sect	DATA,const_notes
	e_sect	DATA,const_notes,	__stop_notes

	e_sect	DATA,const_notes,	__end_rodata
.endm

.macro RW_DATA_SECTION
	s_seg	DATA,		_sdata
	sect	DATA,data

	s_sect	DATA,once, __start_once
	sect	DATA,once
	e_sect	DATA,once, __end_once

	s_sect	DATA,trace_bprintk, __start___trace_bprintk_fmt
	sect	DATA,trace_bprintk
	e_sect	DATA,trace_bprintk, __stop___trace_bprintk_fmt
	s_sect	DATA,tracepoint_str, __start___tracepoint_str
	sect	DATA,tracepoint_str
	e_sect	DATA,tracepoint_str, __stop___tracepoint_str

	# bss always sorts to the end, no need to write it here
	s_sect	DATA,bss,	_edata
	s_sect	DATA,bss,	__bss_start
	e_sect	DATA,bss,	__bss_stop
	e_seg	DATA,		_end
.endm

TEXT_SECTION
RO_DATA_SECTION
RW_DATA_SECTION
INIT_DATA_SECTION

# For some reason the last section always gets moved to be first, so add a
# dummy section to avoid breaking anything
sect DATA,dummy

