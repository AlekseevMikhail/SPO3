#!/usr/bin/env sh

wine RemoteTasks/Portable.RemoteTasks.Manager.exe \
    -ul u284726 -up "$(cat passwd.txt)" \
    -s Assemble -w \
    definitionFile spo.target.pdsl \
    archName TestArch  \
    asmListing test.out

printf "Task ID: "
read task_id

wine RemoteTasks/Portable.RemoteTasks.Manager.exe \
    -ul u284726 -up "$(cat passwd.txt)" \
    -g "$task_id" -r out.ptptb -o out.ptptb
