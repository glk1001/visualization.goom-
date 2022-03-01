#!/bin/bash

set -u

declare -r LOG_DIR=${HOME}/kodi-log
declare -r KODI_DOCKER_LOG=${LOG_DIR}/kodi-docker.log
declare -r IREXEC_CAPTURES=${HOME}/temp/irexec.out
# Set the following to redirect irexec logs
declare -r IREXEC_LOG_DIR=${LOG_DIR}

log()
{
  local -r date_str=$(date +"%m-%d-%y %T")
  { >&2 echo "${date_str} $1 " ; } |& tee -a "${KODI_DOCKER_LOG}"
}

get_kodi_pid()
{
  pidof "kodi.bin"
}

stop_kodi()
{
  if [[ "$(get_kodi_pid)" == "" ]]; then
    log "Kodi is not running. Exiting." ; exit 0
  fi

  log "Stopping Kodi..."
  kodi-send --action="Quit"

  local -r timeout=10
  local timer=0

  while [[ ${timer} -lt ${timeout} && "$(get_kodi_pid)" != "" ]]; do
    log "waiting for Kodi to terminate: ($((timeout - timer)) seconds to go)"
    timer=$((timer+1))
    sleep 1
  done

  if [[ "$(get_kodi_pid)" == "" ]]; then
    log 'Kodi terminated successfully.' ; exit 0
  fi

  log "WARNING: Could not stop Kodi after $timeout seconds."
}


mkdir -p "${LOG_DIR}"
mkdir -p "${HOME}/temp"

trap stop_kodi EXIT

# Run irexec to echo any 'KEY_SLEEP" presses to a signal file.
log "Starting irexec..."
rm -f "${IREXEC_CAPTURES}"
XDG_CACHE_HOME=${IREXEC_LOG_DIR} irexec /etc/lirc/irexec.lircrc > "${IREXEC_CAPTURES}" &

log "Starting Kodi standalone..."
kodi-standalone
declare RET_CODE=$?

log "Kodi exited with return code ${RET_CODE}."
