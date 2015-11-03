#!/bin/bash
#Made by tocororo-macho

CONTENT="${1}"; PADDING="${2}"; LENGTH="${3}"; TRG_EDGE="${4}";
case "${TRG_EDGE}" in
		left)
				echo ${CONTENT} | sed -e :a -e 's/^.\{1,'${LENGTH}'\}$/&\'${PADDING}'/;ta'
				;;
		right)
				echo ${CONTENT} | sed -e :a -e 's/^.\{1,'${LENGTH}'\}$/\'${PADDING}'&/;ta'
				;;
		center)
				echo ${CONTENT} | sed -e :a -e 's/^.\{1,'${LENGTH}'\}$/'${PADDING}'&'${PADDING}'/;ta'
				;;
esac
