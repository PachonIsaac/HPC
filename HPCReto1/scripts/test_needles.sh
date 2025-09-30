
#!/bin/bash
# Prueba de desempeño para algoritmos Needles

echo "Algoritmo,Tiempo(s),Speedup"


get_time() {
       local exe=$1
       if [[ ! -x $exe ]]; then
	       echo "N/A"
	       return
       fi
       local t
       t=$($exe | grep "Tiempo de ejecución" | awk -F ': ' '{print $2}' | awk '{print $1}')
       if [[ -z "$t" ]]; then
	       echo "N/A"
       else
	       echo "$t"
       fi
}


speedup() {
	local base=$1
	local comp=$2
	if [[ "$base" == "N/A" || "$comp" == "N/A" || "$comp" == "0.00" || "$comp" == "0" ]]; then
		echo "N/A"
	else
		echo "$(echo "$base/$comp" | bc -l 2>/dev/null)"
	fi
}

SERIAL_TIME=$(get_time "../bin/pi_needles_serial")
SERIAL_OPT_TIME=$(get_time "../bin/pi_needles_serial_opt")
THREAD_TIME=$(get_time "../bin/pi_needles_thread")
FORK_TIME=$(get_time "../bin/pi_needles_fork")

echo "Serial,$SERIAL_TIME,1"
echo "Serial-Opt,$SERIAL_OPT_TIME,$(speedup $SERIAL_TIME $SERIAL_OPT_TIME)"
echo "Threads,$THREAD_TIME,$(speedup $SERIAL_TIME $THREAD_TIME)"
echo "Fork,$FORK_TIME,$(speedup $SERIAL_TIME $FORK_TIME)"
