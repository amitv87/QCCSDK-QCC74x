#!/usr/bin/env bash
# 生成compile_commands.json
# bear make

# 生成rwnx_config.list
# grep -o 'CFG_[_a-zA-Z0-9]*' rwnx_config.h |sort|uniq >rwnx_config.list


if [ $# -lt 1 ]; then
  echo "Usage: $0 <compile_commands.json Path>"
  exit 1
fi

# 获取CFG开头的宏, 过滤出在rwnx_config.list中的项
cat $1 | jq '.[].arguments[]' |grep -- -DCFG  |sort |uniq |while read x; do eval x=$x; echo $x; done \
|while read x; do xx=${x#-D}; if grep ^${xx%=*}$ rwnx_config.list >/dev/null; then echo $x; fi done \
|./check_cfg.perl
