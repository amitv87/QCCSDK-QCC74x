#!/usr/bin/bash

paste <(grep 'APP_NAME' tools/CI/config/build.yml) <(grep -r 'APP_PATH' tools/CI/config/build.yml) <(grep 'BUILD_CMD' tools/CI/config/build.yml) >cmd
cat cmd  |while read p q u x y m n; do true $((num++)); jq '.demoPath = "'$x'"' tools/qccide/template.json |jq '.buildArgs = "'"$n"'"' | sed "s/\"1\":/\"$num\":/" | sed "s/hello_world/${q}/"; echo ',';done >config.file
sed -i '$d' config.file
sed -i '1i\[' config.file
sed -i '$a\]' config.file
jq . config.file >tools/qccide/qccIdeProjConfig.json
rm config.file
rm cmd
