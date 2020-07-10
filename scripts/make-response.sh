#!/bin/sh
output="${1}"
shift
process_objs()
{
	for obj in "${@}"; do
		if [ ! -f "${obj}" ]; then
			echo "${obj} not found" >&2
			exit 1
		fi
		if [[ "${obj}" == *.a ]]; then
			process_objs $(cat "${obj}")
		else
			echo "${obj}"
		fi
	done
}
process_objs "${@}" > "${output}"
