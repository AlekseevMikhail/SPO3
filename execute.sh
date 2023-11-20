#!/usr/bin/env sh

wine RemoteTasks/Portable.RemoteTasks.Manager.exe \
    -ul u284726 -up "$(cat passwd.txt)" \
    -s ExecuteBinaryWithInput -w \
    definitionFile spo.target.pdsl \
    archName TestArch \
    binaryFileToRun out.ptptb \
    codeRamBankName ram \
    ipRegStorageName ip \
    stdinRegStName stdin_storage \
    stdoutRegStName stdout_storage \
    inputFile test.stdin \
    finishMnemonicName hlt

printf "Task ID: "
read task_id

wine RemoteTasks/Portable.RemoteTasks.Manager.exe \
    -ul u284726 -up "$(cat passwd.txt)" \
    -g "$task_id" -r trace.txt -o trace.txt
