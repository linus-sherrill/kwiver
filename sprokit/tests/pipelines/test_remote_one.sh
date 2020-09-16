#!/bin/bash

#
# Allows simple testing of ZeroMQ pipelines.  Designed to test 1 Reciever and
# two senders.  Usage:
#
#     ./test_remote_one.sh  <environment_setup_script>
#
# Where environment_setup_script is a scripe (possibly
# setup_kiwver.sh) that sets the environent up properly so that
# "kwiver runner" will be found in PATH and will execute properly
#
# TMUX session will create a pane with the remote pipeline and another
# with the main pipeline.
#

SESSION="kwiver_sprokit_zeromq"
START_SCRIPT=$1

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd ${SCRIPT_DIR}

echo "$(date) Creating session ${SESSION}"
tmux new-session -d -s $SESSION

sleep 1
echo "$(date) Starting Remote Pipeline..."
tmux select-window -t $SESSION:0
tmux rename-window -t $SESSION:0 'RemotePipe'
tmux send-keys -t $SESSION:0 "cd ${SCRIPT_DIR}" C-m
tmux send-keys -t $SESSION:0 "source ${START_SCRIPT}" C-m
tmux send-keys -t $SESSION:0 "kwiver runner ${SCRIPT_DIR}/remote_one.pipe" C-m

sleep 1
tmux split-window -t $SESSION:0
tmux select-pane -t 0
# tmux split-window -t $SESSION:0

sleep 1
echo "$(date) Second Main Pipe..."
tmux select-pane -t 1
tmux send-keys -t 1 "cd ${SCRIPT_DIR}" C-m
tmux send-keys -t 1 "source ${START_SCRIPT}" C-m
tmux send-keys -t 1 "kwiver runner ${SCRIPT_DIR}/remote_one_main.pipe " C-m

echo "$(date) Test prepared!"
